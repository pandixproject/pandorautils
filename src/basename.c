#include <stdio.h>
#include <string.h>

static void usage(const char *p) {
  printf("Usage: %s NAME [SUFFIX]\nPrint NAME with any leading directory "
         "components removed.\n",
         p);
}

int main(int argc, char **argv) {
  if (argc == 2 && (!strcmp(argv[1], "-h") || !strcmp(argv[1], "--help"))) {
    usage(argv[0]);
    return 0;
  }
  if (argc < 2 || argc > 3) {
    usage(argv[0]);
    return 1;
  }
  const char *name = argv[1];
  size_t len = strlen(name);
  while (len > 1 && name[len - 1] == '/')
    len--;
  if (len == 1 && name[0] == '/') {
    puts("/");
    return 0;
  }
  const char *start = name + len;
  while (start > name && start[-1] != '/')
    start--;
  size_t outlen = len - (size_t)(start - name);
  if (argc == 3) {
    size_t suffix = strlen(argv[2]);
    if (suffix && suffix < outlen &&
        !memcmp(start + outlen - suffix, argv[2], suffix))
      outlen -= suffix;
  }
  printf("%.*s\n", (int)outlen, start);
  return 0;
}
