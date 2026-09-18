#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int find_command(const char *command) {
  const char *path;
  char *paths;
  char *dir;
  char fullpath[PATH_MAX];

  if (strchr(command, '/') != NULL) {
    if (access(command, X_OK) == 0) {
      printf("%s\n", command);
      return 0;
    }

    return 1;
  }

  path = getenv("PATH");
  if (path == NULL)
    return 1;

  paths = strdup(path);
  if (paths == NULL)
    return 1;

  dir = strtok(paths, ":");

  while (dir != NULL) {
    if (*dir == '\0')
      dir = ".";

    if (snprintf(fullpath, sizeof(fullpath), "%s/%s", dir, command) <
        (int)sizeof(fullpath)) {
      if (access(fullpath, X_OK) == 0) {
        printf("%s\n", fullpath);
        free(paths);
        return 0;
      }
    }

    dir = strtok(NULL, ":");
  }

  free(paths);
  return 1;
}

int main(int argc, char **argv) {
  int i;
  int status = 1;

  if (argc < 2) {
    fprintf(stderr, "usage: pwhich COMMAND...\n");
    return EXIT_FAILURE;
  }

  for (i = 1; i < argc; i++) {
    if (find_command(argv[i]) == 0)
      status = 0;
  }

  return status;
}
