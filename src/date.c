#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef struct {
  int utc;
  int rfc;
  int iso;
  const char *format;
} Options;

void print_usage(const char *prog) {
  printf("Usage: %s [OPTION]...\n", prog);
  printf("Display the current date and time.\n\n");
  printf("  -u             use UTC instead of local time\n");
  printf("  -R             output in RFC 5322 format\n");
  printf("  -I             output in ISO 8601 format\n");
  printf("  -f FORMAT      output using a custom strftime format\n");
  printf("  -h, --help     display this help and exit\n");
}

int main(int argc, char *argv[]) {
  Options opt = {0};
  int i = 1;

  for (; i < argc; i++) {
    if (strcmp(argv[i], "-u") == 0) {
      opt.utc = 1;
    } else if (strcmp(argv[i], "-R") == 0) {
      opt.rfc = 1;
    } else if (strcmp(argv[i], "-I") == 0) {
      opt.iso = 1;
    } else if (strcmp(argv[i], "-f") == 0) {
      if (i + 1 >= argc) {
        fprintf(stderr, "%s: -f requires an argument\n", argv[0]);
        return 1;
      }
      opt.format = argv[++i];
    } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
      print_usage(argv[0]);
      return 0;
    } else if (argv[i][0] == '+') {
      if (opt.format) {
        fprintf(stderr, "%s: choose only one output format\n", argv[0]);
        return 1;
      }
      opt.format = argv[i] + 1;
    } else {
      fprintf(stderr, "%s: unknown option: %s\n", argv[0], argv[i]);
      print_usage(argv[0]);
      return 1;
    }
  }

  if ((opt.rfc + opt.iso + (opt.format != NULL)) > 1) {
    fprintf(stderr, "%s: choose only one of -R, -I, -f\n", argv[0]);
    return 1;
  }

  time_t t = time(NULL);
  struct tm *tm_info = opt.utc ? gmtime(&t) : localtime(&t);
  if (!tm_info) {
    perror("time conversion");
    return 1;
  }

  char buf[128];
  const char *fmt;

  if (opt.format) {
    fmt = opt.format;
  } else if (opt.rfc) {
    fmt = "%a, %d %b %Y %H:%M:%S %z";
  } else if (opt.iso) {
    fmt = opt.utc ? "%Y-%m-%dT%H:%M:%SZ" : "%Y-%m-%dT%H:%M:%S%z";
  } else {
    fmt = "%a %d %b %Y %H:%M:%S %Z";
  }

  if (strftime(buf, sizeof(buf), fmt, tm_info) == 0) {
    fprintf(stderr, "%s: format resulted in empty or too long output\n",
            argv[0]);
    return 1;
  }

  printf("%s\n", buf);
  return 0;
}
