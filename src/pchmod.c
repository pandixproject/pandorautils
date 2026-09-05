#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

static void print_usage(const char *prog) {
  printf("Usage: %s MODE FILE...\n", prog);
  printf("Change the mode of each FILE to the octal MODE.\n\n");
  printf("  -h, --help     display this help and exit\n");
}

static int parse_mode(const char *text, mode_t *mode) {
  char *end;
  errno = 0;
  unsigned long value = strtoul(text, &end, 8);
  if (errno != 0 || *text == '\0' || *end != '\0' || value > 07777)
    return -1;
  *mode = (mode_t)value;
  return 0;
}

int main(int argc, char *argv[]) {
  if (argc == 2 && (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)) {
    print_usage(argv[0]);
    return 0;
  }
  if (argc < 3) {
    print_usage(argv[0]);
    return 1;
  }

  mode_t mode;
  if (parse_mode(argv[1], &mode) != 0) {
    fprintf(stderr, "%s: invalid mode: %s\n", argv[0], argv[1]);
    return 1;
  }

  int status = 0;
  for (int i = 2; i < argc; i++) {
    if (chmod(argv[i], mode) != 0) {
      perror(argv[i]);
      status = 1;
    }
  }
  return status;
}
