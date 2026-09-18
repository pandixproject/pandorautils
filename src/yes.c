#include <stdio.h>
#include <string.h>

static void usage(const char *prog) {
  printf("Usage: %s [STRING]...\nRepeatedly output a line with all specified STRING(s), or 'y'.\n", prog);
}

int main(int argc, char *argv[]) {
  if (argc == 2 && (!strcmp(argv[1], "-h") || !strcmp(argv[1], "--help"))) {
    usage(argv[0]);
    return 0;
  }
  if (argc < 2) {
    while (1) {
      if (fputs("y\n", stdout) == EOF)
        return 0;
    }
  }

  while (1) {
    for (int i = 1; i < argc; i++) {
      fputs(argv[i], stdout);
      if (i < argc - 1)
        fputc(' ', stdout);
    }

    if (fputc('\n', stdout) == EOF)
      return 0;
  }
  return 0;
}
