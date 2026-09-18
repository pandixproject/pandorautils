#include <dirent.h>
#include <fnmatch.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static int matches_type(const char *path, char type) {
  struct stat st;

  if (lstat(path, &st) == -1)
    return 0;

  switch (type) {
  case 'f':
    return S_ISREG(st.st_mode);
  case 'd':
    return S_ISDIR(st.st_mode);
  case 'l':
    return S_ISLNK(st.st_mode);
  default:
    return 0;
  }
}

static void search(const char *path, const char *pattern, char type) {
  DIR *dir;
  struct dirent *entry;

  if (pattern != NULL &&
      fnmatch(pattern, strrchr(path, '/') ? strrchr(path, '/') + 1 : path, 0) ==
          0) {
    if (type == 0 || matches_type(path, type))
      printf("%s\n", path);
  }

  if (type != 0) {
    struct stat st;

    if (lstat(path, &st) == -1 || !S_ISDIR(st.st_mode))
      return;
  }

  dir = opendir(path);
  if (dir == NULL)
    return;

  while ((entry = readdir(dir)) != NULL) {
    char child[PATH_MAX];

    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
      continue;

    if (snprintf(child, sizeof(child), "%s/%s", path, entry->d_name) >=
        (int)sizeof(child))
      continue;

    search(child, pattern, type);
  }

  closedir(dir);
}

int main(int argc, char **argv) {
  const char *path = ".";
  const char *pattern = NULL;
  char type = 0;
  int i;

  for (i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-name") == 0) {
      if (i + 1 >= argc) {
        fprintf(stderr, "pfind: option '-name' requires an argument\n");
        return EXIT_FAILURE;
      }

      pattern = argv[++i];
    } else if (strcmp(argv[i], "-type") == 0) {
      if (i + 1 >= argc) {
        fprintf(stderr, "pfind: option '-type' requires an argument\n");
        return EXIT_FAILURE;
      }

      type = argv[++i][0];

      if (type != 'f' && type != 'd' && type != 'l') {
        fprintf(stderr, "pfind: unknown file type\n");
        return EXIT_FAILURE;
      }
    } else if (argv[i][0] != '-') {
      path = argv[i];
    } else {
      fprintf(stderr, "pfind: unknown option: %s\n", argv[i]);
      return EXIT_FAILURE;
    }
  }

  if (pattern == NULL)
    pattern = "*";

  search(path, pattern, type);

  return EXIT_SUCCESS;
}
