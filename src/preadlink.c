#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void usage(const char *p) { printf("Usage: %s [-f] FILE\nPrint the value of a symbolic link.\n", p); }
int main(int argc, char **argv) {
  int canonical = 0, i = 1;
  if (argc > 1 && !strcmp(argv[1], "-f")) { canonical = 1; i++; }
  if (argc > 1 && (!strcmp(argv[1], "-h") || !strcmp(argv[1], "--help"))) { usage(argv[0]); return 0; }
  if (argc - i != 1) { usage(argv[0]); return 1; }
  char path[PATH_MAX];
  if (canonical) {
    if (!realpath(argv[i], path)) { perror(argv[i]); return 1; }
    puts(path); return 0;
  }
  ssize_t n = readlink(argv[i], path, sizeof(path) - 1);
  if (n < 0) { perror(argv[i]); return 1; }
  path[n] = '\0'; puts(path); return 0;
}
