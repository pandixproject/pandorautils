#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

typedef struct {
  int parents;
  int verbose;
  mode_t mode;
} Options;

// creates dir_path along with any missing parent directories
int mkdir_parents(const char *dir_path, mode_t mode, const Options *opt) {
  char tmp[4096];
  strncpy(tmp, dir_path, sizeof(tmp) - 1);
  tmp[sizeof(tmp) - 1] = '\0';

  size_t len = strlen(tmp);
  if (len == 0)
    return 0;
  if (tmp[len - 1] == '/')
    tmp[len - 1] = '\0';

  for (char *p = tmp + 1; *p; p++) {
    if (*p == '/') {
      *p = '\0';
      if (mkdir(tmp, mode) != 0 && errno != EEXIST) {
        perror(tmp);
        return 1;
      } else if (opt->verbose && errno != EEXIST) {
        printf("created directory '%s'\n", tmp);
      }
      *p = '/';
    }
  }

  if (mkdir(tmp, mode) != 0) {
    if (errno != EEXIST) {
      perror(tmp);
      return 1;
    }
  } else if (opt->verbose) {
    printf("created directory '%s'\n", tmp);
  }

  return 0;
}

void print_usage(const char *prog) {
  printf("Usage: %s [OPTION]... DIRECTORY...\n", prog);
  printf("Create the DIRECTORY(ies), if they do not already exist.\n\n");
  printf("  -p             create parent directories as needed, no error if "
         "existing\n");
  printf("  -v             print a message for each created directory\n");
  printf(
      "  -m MODE        set permission mode (octal, e.g. 700), default 755\n");
  printf("  -h, --help     display this help and exit\n");
}

int main(int argc, char *argv[]) {
  Options opt = {0, 0, 0755};
  int i = 1;

  for (; i < argc; i++) {
    if (strcmp(argv[i], "-p") == 0) {
      opt.parents = 1;
    } else if (strcmp(argv[i], "-v") == 0) {
      opt.verbose = 1;
    } else if (strcmp(argv[i], "-m") == 0) {
      if (i + 1 >= argc) {
        fprintf(stderr, "%s: -m requires an argument\n", argv[0]);
        return 1;
      }
      opt.mode = (mode_t)strtol(argv[++i], NULL, 8);
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
      result = mkdir_parents(argv[i], opt.mode, &opt);
    } else {
      result = mkdir(argv[i], opt.mode);
      if (result != 0) {
        perror(argv[i]);
      } else if (opt.verbose) {
        printf("created directory '%s'\n", argv[i]);
      }
    }
    if (result != 0)
      status = 1;
  }

  return status;
}
