#include <stdio.h>
#include <string.h>

static void usage(const char *p) { printf("Usage: %s NAME\nPrint NAME with its last component removed.\n", p); }
int main(int argc, char **argv) {
  if (argc == 2 && (!strcmp(argv[1], "-h") || !strcmp(argv[1], "--help"))) { usage(argv[0]); return 0; }
  if (argc != 2) { usage(argv[0]); return 1; }
  char path[4096];
  if (strlen(argv[1]) >= sizeof(path)) { fprintf(stderr, "%s: path too long\n", argv[0]); return 1; }
  strcpy(path, argv[1]);
  size_t n = strlen(path);
  while (n > 1 && path[n - 1] == '/') path[--n] = '\0';
  while (n > 0 && path[n - 1] != '/') n--;
  while (n > 1 && path[n - 1] == '/') n--;
  if (n == 0) puts("."); else { path[n] = '\0'; puts(path); }
  return 0;
}
