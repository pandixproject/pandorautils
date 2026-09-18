#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  int no_newline;
  int interpret;
} Options;

int print_interpreted(const char *s) {
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
      case 'f':
        fputc('\f', stdout);
        break;
      case 'v':
        fputc('\v', stdout);
        break;
      case 'e':
        fputc('\033', stdout);
        break;
      case 'c':
        return 1;
      case '0': {
        int val = 0, n = 0;
        i++;
        while (n < 3 && s[i] >= '0' && s[i] <= '7') {
          val = val * 8 + (s[i] - '0');
          i++;
          n++;
        }
        i--;
        fputc((char)val, stdout);
        break;
      }
      case 'x': {
        int val = 0, n = 0;
        i++;
        while (n < 2 && isxdigit((unsigned char)s[i])) {
          char c = s[i];
          int d = (c >= '0' && c <= '9')   ? c - '0'
                  : (c >= 'a' && c <= 'f') ? c - 'a' + 10
                                           : c - 'A' + 10;
          val = val * 16 + d;
          i++;
          n++;
        }
        i--;
        if (n == 0) {
          fputc('\\', stdout);
          fputc('x', stdout);
        } else {
          fputc((char)val, stdout);
        }
        break;
      }
      default:
        fputc('\\', stdout);
        fputc(s[i], stdout);
        break;
      }
    } else {
      fputc(s[i], stdout);
    }
  }
  return 0;
}

void print_usage(const char *prog) {
  printf("Usage: %s [SHORT-OPTION]... [STRING]...\n", prog);
  printf("  or:  %s LONG-OPTION\n", prog);
  printf("Echo the STRING(s) to standard output.\n\n");
  printf("  -n             do not output the trailing newline\n");
  printf("  -e             enable interpretation of backslash escapes\n");
  printf("  -E             disable interpretation of backslash escapes "
         "(default)\n");
  printf("  -h, --help     display this help and exit\n");
  printf("      --version  output version information and exit\n\n");
  printf("If -e is in effect, the following sequences are recognized:\n\n");
  printf("  \\\\     backslash\n");
  printf("  \\a     alert (BEL)\n");
  printf("  \\b     backspace\n");
  printf("  \\c     produce no further output\n");
  printf("  \\e     escape\n");
  printf("  \\f     form feed\n");
  printf("  \\n     new line\n");
  printf("  \\r     carriage return\n");
  printf("  \\t     horizontal tab\n");
  printf("  \\v     vertical tab\n");
  printf("  \\0NNN  byte with octal value NNN (1 to 3 digits)\n");
  printf("  \\xHH   byte with hexadecimal value HH (1 to 2 digits)\n");
}

int main(int argc, char *argv[]) {
  Options opt = {0};
  int i = 1;

  for (; i < argc; i++) {
    if (strcmp(argv[i], "--help") == 0) {
      print_usage(argv[0]);
      return 0;
    } else if (strcmp(argv[i], "--version") == 0) {
      printf("echo (custom coreutils clone)\n");
      return 0;
    } else if (strcmp(argv[i], "--") == 0) {
      i++;
      break;
    } else if (argv[i][0] == '-' && argv[i][1] != '\0') {
      // Check every char is a valid flag; if not, treat whole arg as string
      int all_valid = 1;
      for (char *p = argv[i] + 1; *p != '\0'; p++) {
        if (*p != 'n' && *p != 'e' && *p != 'E') {
          all_valid = 0;
          break;
        }
      }
      if (!all_valid)
        break;

      for (char *p = argv[i] + 1; *p != '\0'; p++) {
        if (*p == 'n')
          opt.no_newline = 1;
        else if (*p == 'e')
          opt.interpret = 1;
        else if (*p == 'E')
          opt.interpret = 0;
      }
    } else {
      break;
    }
  }

  int stop = 0;
  for (int j = i; j < argc && !stop; j++) {
    if (opt.interpret) {
      stop = print_interpreted(argv[j]);
    } else {
      fputs(argv[j], stdout);
    }
    if (!stop && j < argc - 1)
      fputc(' ', stdout);
  }

  if (!opt.no_newline && !stop)
    fputc('\n', stdout);

  return 0;
}
