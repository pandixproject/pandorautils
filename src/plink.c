#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

static void usage(const char *p) { printf("Usage: %s [-s] [-f] TARGET LINK_NAME\nCreate a hard link, or a symbolic link with -s.\n", p); }
int main(int argc, char **argv) {
  int symbolic = 0, force = 0, i = 1;
  for (; i < argc; i++) {
    if (!strcmp(argv[i], "-s")) symbolic = 1;
    else if (!strcmp(argv[i], "-f")) force = 1;
    else if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) { usage(argv[0]); return 0; }
    else break;
  }
  if (argc - i != 2) { usage(argv[0]); return 1; }
  if (force && unlink(argv[i + 1]) && errno != ENOENT) { perror(argv[i + 1]); return 1; }
  if ((symbolic ? symlink(argv[i], argv[i + 1]) : link(argv[i], argv[i + 1])) != 0) { perror(argv[i + 1]); return 1; }
  return 0;
}
