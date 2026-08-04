#include <stdio.h>

int main(int argc, char *argv[]) {
  if (argc < 2) {
    while (1)
      printf("y\n");
  }
  while (1) {
    for (int i = 1; i < argc; i++) {
      fputs(argv[i], stdout);
      if (i < argc - 1)
        fputc(' ', stdout);
    }
    fputc('\n', stdout);
  }
  return 0;
}
