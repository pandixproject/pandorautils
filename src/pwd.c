#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct {
  int logical;
  int physical;
} Options;

void print_usage(const char *prog) {
  printf("Usage: %s [OPTION]...\n", prog);
  printf("Print the full pathname of the current working directory.\n\n");
  printf("  -L             use PWD from environment, even if it contains "
         "symlinks (default)\n");
  printf("  -P             avoid all symlinks, print the physical path\n");
  printf("  -h, --help     display this help and exit\n");
}

// checks that path actually refers to the same directory as the real cwd,
// so a stale or spoofed PWD env var doesn't get printed
int matches_real_cwd(const char *path) {
  char real[PATH_MAX];
  if (getcwd(real, sizeof(real)) == NULL)
    return 0;

  char resolved[PATH_MAX];
  if (realpath(path, resolved) == NULL)
    return 0;

  return strcmp(real, resolved) == 0;
}

int main(int argc, char *argv[]) {
  Options opt = {1, 0};

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-L") == 0) {
      opt.logical = 1;
      opt.physical = 0;
    } else if (strcmp(argv[i], "-P") == 0) {
      opt.physical = 1;
      opt.logical = 0;
    } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
      print_usage(argv[0]);
      return 0;
    } else {
      fprintf(stderr, "%s: unknown option: %s\n", argv[0], argv[i]);
      print_usage(argv[0]);
      return 1;
    }
  }

  if (opt.logical) {
    const char *pwd = getenv("PWD");
    if (pwd != NULL && pwd[0] == '/' && matches_real_cwd(pwd)) {
      printf("%s\n", pwd);
      return 0;
    }
  }

  char buf[PATH_MAX];
  if (getcwd(buf, sizeof(buf)) != NULL) {
    printf("%s\n", buf);
    return 0;
  }

  perror("ppwd");
  return 1;
}
