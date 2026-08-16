#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <utime.h>

typedef struct {
  int recursive;
  int verbose;
  int preserve;
  int force;
  int interactive;
} Options;

int copy_file(const char *src_path, const char *dst_path, const Options *opt) {
  struct stat st;
  if (stat(src_path, &st) == -1) {
    perror(src_path);
    return 1;
  }

  if (opt->interactive && !opt->force) {
    struct stat dst_st;
    if (stat(dst_path, &dst_st) == 0) {
      fprintf(stderr, "overwrite '%s'? (y/n) ", dst_path);
      int c = getchar();
      while (c != '\n' && c != EOF)
        getchar();
      if (c != 'y' && c != 'Y')
        return 0;
    }
  }

  FILE *src = fopen(src_path, "rb");
  if (!src) {
    perror(src_path);
    return 1;
  }

  FILE *dst = fopen(dst_path, "wb");
  if (!dst) {
    perror(dst_path);
    fclose(src);
    return 1;
  }

  char buf[4096];
  size_t n;
  int status = 0;

  while ((n = fread(buf, 1, sizeof(buf), src)) > 0) {
    if (fwrite(buf, 1, n, dst) != n) {
      perror(dst_path);
      status = 1;
      break;
    }
  }

  if (ferror(src)) {
    perror(src_path);
    status = 1;
  }

  fclose(src);
  fclose(dst);

  if (status == 0 && opt->preserve) {
    chmod(dst_path, st.st_mode);
    struct utimbuf times;
    times.actime = st.st_atime;
    times.modtime = st.st_mtime;
    utime(dst_path, &times);
  }

  if (status == 0 && opt->verbose) {
    printf("'%s' -> '%s'\n", src_path, dst_path);
  }

  return status;
}

int copy_dir(const char *src_path, const char *dst_path, const Options *opt) {
  struct stat st;
  if (stat(src_path, &st) == -1) {
    perror(src_path);
    return 1;
  }

  if (mkdir(dst_path, st.st_mode) == -1 && errno != EEXIST) {
    perror(dst_path);
    return 1;
  }

  DIR *d = opendir(src_path);
  if (!d) {
    perror(src_path);
    return 1;
  }

  int status = 0;
  struct dirent *entry;

  while ((entry = readdir(d)) != NULL) {
    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
      continue;

    char src_child[4096];
    char dst_child[4096];
    snprintf(src_child, sizeof(src_child), "%s/%s", src_path, entry->d_name);
    snprintf(dst_child, sizeof(dst_child), "%s/%s", dst_path, entry->d_name);

    struct stat child_st;
    if (lstat(src_child, &child_st) == -1) {
      perror(src_child);
      status = 1;
      continue;
    }

    if (S_ISDIR(child_st.st_mode)) {
      if (copy_dir(src_child, dst_child, opt) != 0)
        status = 1;
    } else if (S_ISREG(child_st.st_mode)) {
      if (copy_file(src_child, dst_child, opt) != 0)
        status = 1;
    }
  }

  closedir(d);

  if (opt->preserve) {
    chmod(dst_path, st.st_mode);
  }

  if (opt->verbose) {
    printf("'%s' -> '%s'\n", src_path, dst_path);
  }

  return status;
}

void print_usage(const char *prog) {
  printf("Usage: %s [OPTION]... SOURCE DEST\n", prog);
  printf("Copy SOURCE to DEST.\n\n");
  printf("  -r         copy directories recursively\n");
  printf("  -p         preserve mode and timestamps\n");
  printf("  -v         explain what is being done\n");
  printf("  -i         prompt before overwrite\n");
  printf("  -f         force overwrite, no prompt\n");
  printf("  -h, --help display this help and exit\n");
}

int main(int argc, char *argv[]) {
  Options opt = {0};
  int i = 1;

  for (; i < argc; i++) {
    if (argv[i][0] == '-' && argv[i][1] != '\0') {
      if (strcmp(argv[i], "-r") == 0)
        opt.recursive = 1;
      else if (strcmp(argv[i], "-v") == 0)
        opt.verbose = 1;
      else if (strcmp(argv[i], "-p") == 0)
        opt.preserve = 1;
      else if (strcmp(argv[i], "-i") == 0)
        opt.interactive = 1;
      else if (strcmp(argv[i], "-f") == 0)
        opt.force = 1;
      else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
        print_usage(argv[0]);
        return 0;
      } else {
        fprintf(stderr, "%s: unknown option: %s\n", argv[0], argv[i]);
        print_usage(argv[0]);
        return 1;
      }
    } else {
      break;
    }
  }

  if (argc - i != 2) {
    print_usage(argv[0]);
    return 1;
  }

  const char *src_path = argv[i];
  const char *dst_path = argv[i + 1];

  struct stat st;
  if (stat(src_path, &st) == -1) {
    perror(src_path);
    return 1;
  }

  if (S_ISDIR(st.st_mode)) {
    if (!opt.recursive) {
      fprintf(stderr, "%s: omitting directory '%s' (use -r)\n", argv[0],
              src_path);
      return 1;
    }
    return copy_dir(src_path, dst_path, &opt);
  }
  return copy_file(src_path, dst_path, &opt);
}
