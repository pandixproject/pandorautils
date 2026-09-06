#define _POSIX_C_SOURCE 200809L
#include <dirent.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

typedef struct { int all, long_format, recursive; } Options;
static void usage(const char *p) { printf("Usage: %s [-alR] [FILE]...\nList directory contents.\n  -a include entries starting with .\n  -l use a long listing format\n  -R list subdirectories recursively\n", p); }
static int compare(const void *a, const void *b) { return strcmp(*(const char *const *)a, *(const char *const *)b); }
static void mode_text(mode_t m, char out[11]) {
  out[0] = S_ISDIR(m) ? 'd' : S_ISLNK(m) ? 'l' : S_ISCHR(m) ? 'c' : S_ISBLK(m) ? 'b' : S_ISFIFO(m) ? 'p' : S_ISSOCK(m) ? 's' : '-';
  const mode_t bits[] = {S_IRUSR,S_IWUSR,S_IXUSR,S_IRGRP,S_IWGRP,S_IXGRP,S_IROTH,S_IWOTH,S_IXOTH};
  for (int i = 0; i < 9; i++)
    out[i + 1] = m & bits[i] ? "rwxrwxrwx"[i] : '-';
  out[10] = '\0';
}
static void show(const char *path, const char *name, const Options *o) {
  if (!o->long_format) { puts(name); return; }
  struct stat st; if (lstat(path, &st)) { perror(path); return; }
  char mode[11], date[32]; mode_text(st.st_mode, mode); struct passwd *pw = getpwuid(st.st_uid); struct tm tm;
  localtime_r(&st.st_mtime, &tm); strftime(date, sizeof(date), "%Y-%m-%d %H:%M", &tm);
  printf("%s %3lu %-8s %8lld %s %s\n", mode, (unsigned long)st.st_nlink, pw ? pw->pw_name : "?", (long long)st.st_size, date, name);
}
static int list(const char *path, const Options *o, int heading) {
  struct stat st; if (lstat(path, &st)) { perror(path); return 1; }
  if (!S_ISDIR(st.st_mode)) { show(path, path, o); return 0; }
  DIR *dir = opendir(path); if (!dir) { perror(path); return 1; }
  char **names = NULL; size_t count = 0, cap = 0; struct dirent *entry;
  while ((entry = readdir(dir))) {
    if (!o->all && entry->d_name[0] == '.') continue;
    if (count == cap) { size_t next = cap ? cap * 2 : 32; char **more = realloc(names, next * sizeof(*names)); if (!more) { perror("pls"); closedir(dir); return 1; } names = more; cap = next; }
    names[count] = strdup(entry->d_name); if (!names[count++]) { perror("pls"); closedir(dir); return 1; }
  }
  closedir(dir); qsort(names, count, sizeof(*names), compare); if (heading) printf("%s:\n", path); int status = 0;
  for (size_t i = 0; i < count; i++) { char child[4096]; if (snprintf(child, sizeof(child), "%s/%s", path, names[i]) >= (int)sizeof(child)) { fprintf(stderr, "pls: path too long\n"); status = 1; } else show(child, names[i], o); }
  if (o->recursive) for (size_t i = 0; i < count; i++) { if (!strcmp(names[i], ".") || !strcmp(names[i], "..")) continue; char child[4096]; struct stat child_st; snprintf(child, sizeof(child), "%s/%s", path, names[i]); if (!lstat(child, &child_st) && S_ISDIR(child_st.st_mode)) { putchar('\n'); if (list(child, o, 1)) status = 1; } }
  for (size_t i = 0; i < count; i++)
    free(names[i]);
  free(names);
  return status;
}
int main(int argc, char **argv) {
  Options o = {0}; int i = 1;
  for (; i < argc; i++) { if (!strcmp(argv[i], "-a")) o.all = 1; else if (!strcmp(argv[i], "-l")) o.long_format = 1; else if (!strcmp(argv[i], "-R")) o.recursive = 1; else if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) { usage(argv[0]); return 0; } else if (!strcmp(argv[i], "--")) { i++; break; } else if (argv[i][0] == '-') { fprintf(stderr, "%s: unknown option: %s\n", argv[0], argv[i]); return 1; } else break; }
  int inputs = argc - i, status = 0; if (!inputs) return list(".", &o, 0);
  for (; i < argc; i++) { if (i > argc - inputs) putchar('\n'); if (list(argv[i], &o, inputs > 1)) status = 1; } return status;
}
