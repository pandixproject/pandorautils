#include <errno.h>
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct {
  int parents;
  int verbose;
} Options;

// removes dir_path, then walks up removing each now-empty parent
int rmdir_parents(const char *dir_path, const Options *opt) {
  char tmp[4096];
  strncpy(tmp, dir_path, sizeof(tmp) - 1);
  tmp[sizeof(tmp) - 1] = '\0';

  size_t len = strlen(tmp);
  while (len > 0 && tmp[len - 1] == '/') {
    tmp[--len] = '\0';
  }

  if (rmdir(tmp) != 0) {
    perror(tmp);
    return 1;
  }
  if (opt->verbose) {
    printf("removed directory '%s'\n", tmp);
  }

  char *slash;
  while ((slash = strrchr(tmp, '/')) != NULL) {
    *slash = '\0';
    if (tmp[0] == '\0')
      break;

    if (rmdir(tmp) != 0) {
      if (errno != ENOTEMPTY && errno != EEXIST) {
        perror(tmp);
        return 1;
      }
      break;
    }
    if (opt->verbose) {
      printf("removed directory '%s'\n", tmp);
    }
  }

  return 0;
}

void print_usage(const char *prog) {
  printf("Usage: %s [OPTION]... DIRECTORY...\n", prog);
  printf("Remove the DIRECTORY(ies), if they are empty.\n\n");
  printf("  -p             remove DIRECTORY and its empty parents\n");
  printf("  -v             print a message for each removed directory\n");
  printf("  -h, --help     display this help and exit\n");
}

int main(int argc, char *argv[]) {
  Options opt = {0};
  int i = 1;

  for (; i < argc; i++) {
    if (strcmp(argv[i], "-p") == 0) {
      opt.parents = 1;
    } else if (strcmp(argv[i], "-v") == 0) {
      opt.verbose = 1;
    } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
      print_usage(argv[0]);
      return 0;
    } else if (argv[i][0] == '-' && argv[i][1] != '\0') {
      fprintf(stderr, "%s: unknown option: %s\n", argv[0], argv[i]);
      print_usage(argv[0]);
      return 1;
    } else {
      break;
    }
  }

  if (i >= argc) {
    print_usage(argv[0]);
    return 1;
  }

  int status = 0;

  for (; i < argc; i++) {
    int result;
    if (opt.parents) {
      result = rmdir_parents(argv[i], &opt);
    } else {
      result = rmdir(argv[i]);
      if (result != 0) {
        perror(argv[i]);
      } else if (opt.verbose) {
        printf("removed directory '%s'\n", argv[i]);
      }
    }
    if (result != 0)
      status = 1;
  }

  return status;
}
