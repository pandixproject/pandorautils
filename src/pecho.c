#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  int no_newline;
  int interpret;
} Options;

void print_interpreted(const char *s) {
  for (size_t i = 0; s[i] != '\0'; i++) {
    if (s[i] == '\\' && s[i + 1] != '\0') {
      i++;
      switch (s[i]) {
      case 'n':
        fputc('\n', stdout);
        break;
      case 't':
        fputc('\t', stdout);
        break;
      case 'r':
        fputc('\r', stdout);
        break;
      case '\\':
        fputc('\\', stdout);
        break;
      case 'a':
        fputc('\a', stdout);
        break;
      case 'b':
        fputc('\b', stdout);
        break;
      case '0':
        fputc('\0', stdout);
        break;
      default:
        fputc('\\', stdout);
        fputc(s[i], stdout);
        break;
      }
    } else {
      fputc(s[i], stdout);
    }
  }
}

void print_usage(const char *prog) {
  printf("Usage: %s [OPTION]... [STRING]...\n", prog);
  printf("Echo the STRING(s) to standard output.\n\n");
  printf("  -n             do not output the trailing newline\n");
  printf(
      "  -e             interpret backslash escapes (\\n, \\t, \\\\, etc.)\n");
  printf("  -h, --help     display this help and exit\n");
}

int main(int argc, char *argv[]) {
  Options opt = {0};
  int i = 1;

  for (; i < argc; i++) {
    if (strcmp(argv[i], "-n") == 0) {
      opt.no_newline = 1;
    } else if (strcmp(argv[i], "-e") == 0) {
      opt.interpret = 1;
    } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
      print_usage(argv[0]);
      return 0;
    } else if (strcmp(argv[i], "--") == 0) {
      i++;
      break;
    } else {
      break;
    }
  }

  for (int j = i; j < argc; j++) {
    if (opt.interpret) {
      print_interpreted(argv[j]);
    } else {
      fputs(argv[j], stdout);
    }
    if (j < argc - 1)
      fputc(' ', stdout);
  }

  if (!opt.no_newline)
    fputc('\n', stdout);

  return 0;
}
