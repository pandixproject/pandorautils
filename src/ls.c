#define _POSIX_C_SOURCE 200809L
#include <dirent.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

typedef struct {
  int all, long_format, recursive;
} Options;

static void usage(const char *p) {
  printf("Usage: %s [-alR] [FILE]...\n"
         "List directory contents.\n"
         "  -a  include entries starting with .\n"
         "  -l  use a long listing format\n"
         "  -R  list subdirectories recursively\n",
         p);
}

static int compare(const void *a, const void *b) {
  return strcmp(*(const char *const *)a, *(const char *const *)b);
}

static void mode_text(mode_t m, char out[11]) {
  out[0] = S_ISDIR(m)    ? 'd'
           : S_ISLNK(m)  ? 'l'
           : S_ISCHR(m)  ? 'c'
           : S_ISBLK(m)  ? 'b'
           : S_ISFIFO(m) ? 'p'
           : S_ISSOCK(m) ? 's'
                         : '-';
  const mode_t bits[] = {S_IRUSR, S_IWUSR, S_IXUSR, S_IRGRP, S_IWGRP,
                         S_IXGRP, S_IROTH, S_IWOTH, S_IXOTH};
  for (int i = 0; i < 9; i++)
    out[i + 1] = m & bits[i] ? "rwxrwxrwx"[i] : '-';
  out[10] = '\0';
}

/* Terminal width: honor a real tty via TIOCGWINSZ, else $COLUMNS, else 80.
   When stdout isn't a tty, GNU ls falls back to one name per line, which
   we mirror via is_output_tty(). */
static int get_terminal_width(void) {
  struct winsize ws;
  if (isatty(STDOUT_FILENO) && ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 &&
      ws.ws_col > 0)
    return ws.ws_col;
  const char *cols = getenv("COLUMNS");
  if (cols) {
    int c = atoi(cols);
    if (c > 0)
      return c;
  }
  return 80;
}

static int is_output_tty(void) { return isatty(STDOUT_FILENO); }

static void show_long(const char *path, const char *name) {
  struct stat st;
  if (lstat(path, &st)) {
    perror(path);
    return;
  }
  char mode[11], date[32];
  mode_text(st.st_mode, mode);
  struct passwd *pw = getpwuid(st.st_uid);
  struct tm tm;
  localtime_r(&st.st_mtime, &tm);
  strftime(date, sizeof(date), "%Y-%m-%d %H:%M", &tm);
  printf("%s %3lu %-8s %8lld %s %s\n", mode, (unsigned long)st.st_nlink,
         pw ? pw->pw_name : "?", (long long)st.st_size, date, name);
}

// print the names in a column-major grid sized to the terminal width so it
// doesn't get very long
static void print_grid(char *const *names, size_t count) {
  if (count == 0)
    return;

  size_t maxlen = 0;
  for (size_t i = 0; i < count; i++) {
    size_t len = strlen(names[i]);
    if (len > maxlen)
      maxlen = len;
  }

  size_t colwidth = maxlen + 2;
  size_t term_width = (size_t)get_terminal_width();
  size_t cols = term_width / colwidth;
  if (cols < 1)
    cols = 1;
  if (cols > count)
    cols = count;
  size_t rows = (count + cols - 1) / cols;

  for (size_t r = 0; r < rows; r++) {
    for (size_t c = 0; c < cols; c++) {
      size_t idx = c * rows + r;
      if (idx >= count)
        continue;
      int last_in_row = (c == cols - 1) || (idx + rows >= count);
      if (last_in_row)
        printf("%s\n", names[idx]);
      else
        printf("%-*s", (int)colwidth, names[idx]);
    }
  }
}

static void print_names(char *const *names, size_t count) {
  if (is_output_tty())
    print_grid(names, count);
  else
    for (size_t i = 0; i < count; i++)
      puts(names[i]);
}

static int list(const char *path, const Options *o, int heading) {
  struct stat st;
  if (lstat(path, &st)) {
    perror(path);
    return 1;
  }

  if (!S_ISDIR(st.st_mode)) {
    if (o->long_format)
      show_long(path, path);
    else
      puts(path);
    return 0;
  }

  DIR *dir = opendir(path);
  if (!dir) {
    perror(path);
    return 1;
  }

  char **names = NULL;
  size_t count = 0, cap = 0;
  struct dirent *entry;
  while ((entry = readdir(dir))) {
    if (!o->all && entry->d_name[0] == '.')
      continue;
    if (count == cap) {
      size_t next = cap ? cap * 2 : 32;
      char **more = realloc(names, next * sizeof(*names));
      if (!more) {
        perror("pls");
        closedir(dir);
        return 1;
      }
      names = more;
      cap = next;
    }
    names[count] = strdup(entry->d_name);
    if (!names[count++]) {
      perror("pls");
      closedir(dir);
      return 1;
    }
  }
  closedir(dir);
  qsort(names, count, sizeof(*names), compare);

  if (heading)
    printf("%s:\n", path);

  int status = 0;

  if (o->long_format) {
    long long total = 0;
    for (size_t i = 0; i < count; i++) {
      char child[4096];
      if (snprintf(child, sizeof(child), "%s/%s", path, names[i]) >=
          (int)sizeof(child))
        continue;
      struct stat cst;
      if (!lstat(child, &cst))
        total += cst.st_blocks;
    }
    printf("total %lld\n", total / 2);

    for (size_t i = 0; i < count; i++) {
      char child[4096];
      if (snprintf(child, sizeof(child), "%s/%s", path, names[i]) >=
          (int)sizeof(child)) {
        fprintf(stderr, "pls: path too long\n");
        status = 1;
      } else {
        show_long(child, names[i]);
      }
    }
  } else {
    print_names(names, count);
  }

  if (o->recursive)
    for (size_t i = 0; i < count; i++) {
      if (!strcmp(names[i], ".") || !strcmp(names[i], ".."))
        continue;
      char child[4096];
      struct stat child_st;
      snprintf(child, sizeof(child), "%s/%s", path, names[i]);
      if (!lstat(child, &child_st) && S_ISDIR(child_st.st_mode)) {
        putchar('\n');
        if (list(child, o, 1))
          status = 1;
      }
    }

  for (size_t i = 0; i < count; i++)
    free(names[i]);
  free(names);
  return status;
}

int main(int argc, char **argv) {
  Options o = {0};
  int i = 1;
  for (; i < argc; i++) {
    if (!strcmp(argv[i], "-a"))
      o.all = 1;
    else if (!strcmp(argv[i], "-l"))
      o.long_format = 1;
    else if (!strcmp(argv[i], "-R"))
      o.recursive = 1;
    else if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
      usage(argv[0]);
      return 0;
    } else if (!strcmp(argv[i], "--")) {
      i++;
      break;
    } else if (argv[i][0] == '-' && argv[i][1] != '\0') {
      // support combined short options like -la
      int ok = 1;
      for (const char *f = argv[i] + 1; *f; f++) {
        if (*f == 'a')
          o.all = 1;
        else if (*f == 'l')
          o.long_format = 1;
        else if (*f == 'R')
          o.recursive = 1;
        else {
          ok = 0;
          break;
        }
      }
      if (!ok) {
        fprintf(stderr, "%s: unknown option: %s\n", argv[0], argv[i]);
        return 1;
      }
    } else
      break;
  }

  int inputs = argc - i, status = 0;
  if (!inputs)
    return list(".", &o, 0);

  for (int first = 1; i < argc; i++, first = 0) {
    if (!first)
      putchar('\n');
    if (list(argv[i], &o, inputs > 1))
      status = 1;
  }
  return status;
}
