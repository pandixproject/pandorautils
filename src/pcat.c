#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  int number_lines;
  int number_nonblank;
  int show_ends;
  int squeeze_blank;
  int show_tabs;
  int show_nonprinting;
} Options;

void process_stream(FILE *f, const Options *opt, long *line_num) {
  char buf[4096];

  if (!opt->number_lines && !opt->number_nonblank && !opt->show_ends &&
      !opt->squeeze_blank && !opt->show_tabs && !opt->show_nonprinting) {
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), f)) > 0) {
      fwrite(buf, 1, n, stdout);
    }
    return;
  }

  char *line = NULL;
  size_t cap = 0;
  ssize_t len;
  int blank_count_in_a_row = 0;

  while ((len = getline(&line, &cap, f)) != -1) {
    int is_blank = (len == 1 && line[0] == '\n');

    if (opt->squeeze_blank) {
      if (is_blank) {
        blank_count_in_a_row++;
        if (blank_count_in_a_row > 1)
          continue;
      } else {
        blank_count_in_a_row = 0;
      }
    }

    if (opt->number_lines) {
      printf("%6ld\t", ++(*line_num));
    } else if (opt->number_nonblank) {
      if (!is_blank)
        printf("%6ld\t", ++(*line_num));
    }

    int has_newline = (len > 0 && line[len - 1] == '\n');
    ssize_t content_len = has_newline ? len - 1 : len;

    for (ssize_t j = 0; j < content_len; j++) {
      unsigned char c = (unsigned char)line[j];

      if (c == '\t') {
        if (opt->show_tabs) {
          fputs("^I", stdout);
        } else {
          fputc(c, stdout);
        }
      } else if (opt->show_nonprinting && c != '\n') {
        if (c >= 128) {
          fputs("M-", stdout);
          c -= 128;
        }
        if (c < 32) {
          fputc('^', stdout);
          fputc(c + 64, stdout);
        } else if (c == 127) {
          fputs("^?", stdout);
        } else {
          fputc(c, stdout);
        }
      } else {
        fputc(c, stdout);
      }
    }

    if (opt->show_ends) {
      fputc('$', stdout);
    }
    if (has_newline) {
      fputc('\n', stdout);
    }
  }
  free(line);
}

void print_usage(const char *prog) {
  printf("Usage: %s [OPTION]... [FILE]...\n", prog);
  printf("Concatenate FILE(s) to standard output.\n\n");
  printf("  -A, --show-all           equivalent to -vET\n");
  printf("  -b, --number-nonblank    number nonempty output lines\n");
  printf("  -e                       equivalent to -vE\n");
  printf("  -E, --show-ends          display $ at end of each line\n");
  printf("  -n, --number             number all output lines\n");
  printf("  -s, --squeeze-blank      squeeze multiple adjacent blank lines\n");
  printf("  -t                       equivalent to -vT\n");
  printf("  -T, --show-tabs          display TAB characters as ^I\n");
  printf("  -u                       (ignored)\n");
  printf("  -v, --show-nonprinting   use ^ and M- notation, except for LFD and "
         "TAB\n");
  printf("  -h, --help               display this help and exit\n\n");
  printf("With no FILE, or when FILE is -, read standard input.\n");
}

int parse_long_opt(const char *arg, Options *opt) {
  if (strcmp(arg, "--number") == 0)
    opt->number_lines = 1;
  else if (strcmp(arg, "--number-nonblank") == 0)
    opt->number_nonblank = 1;
  else if (strcmp(arg, "--show-ends") == 0)
    opt->show_ends = 1;
  else if (strcmp(arg, "--squeeze-blank") == 0)
    opt->squeeze_blank = 1;
  else if (strcmp(arg, "--show-tabs") == 0)
    opt->show_tabs = 1;
  else if (strcmp(arg, "--show-nonprinting") == 0)
    opt->show_nonprinting = 1;
  else if (strcmp(arg, "--show-all") == 0) {
    opt->show_nonprinting = 1;
    opt->show_ends = 1;
    opt->show_tabs = 1;
  } else if (strcmp(arg, "--help") == 0)
    return 2;
  else
    return 0;
  return 1;
}

int parse_short_flag(char c, Options *opt) {
  switch (c) {
  case 'n':
    opt->number_lines = 1;
    return 1;
  case 'b':
    opt->number_nonblank = 1;
    return 1;
  case 'E':
    opt->show_ends = 1;
    return 1;
  case 's':
    opt->squeeze_blank = 1;
    return 1;
  case 'T':
    opt->show_tabs = 1;
    return 1;
  case 'v':
    opt->show_nonprinting = 1;
    return 1;
  case 'u':
    return 1;
  case 'A':
    opt->show_nonprinting = 1;
    opt->show_ends = 1;
    opt->show_tabs = 1;
    return 1;
  case 'e':
    opt->show_nonprinting = 1;
    opt->show_ends = 1;
    return 1;
  case 't':
    opt->show_nonprinting = 1;
    opt->show_tabs = 1;
    return 1;
  case 'h':
    return 2;
  default:
    return 0;
  }
}

int main(int argc, char *argv[]) {
  Options opt = {0};
  int i = 1;

  for (; i < argc; i++) {
    if (argv[i][0] == '-' && argv[i][1] == '-' && argv[i][2] != '\0') {
      int r = parse_long_opt(argv[i], &opt);
      if (r == 2) {
        print_usage(argv[0]);
        return 0;
      } else if (r == 0) {
        fprintf(stderr, "%s: unknown option: %s\n", argv[0], argv[i]);
        print_usage(argv[0]);
        return 1;
      }
    } else if (argv[i][0] == '-' && argv[i][1] != '\0') {
      if (strcmp(argv[i], "-") == 0)
        break;

      int ok = 1;
      for (char *p = argv[i] + 1; *p != '\0'; p++) {
        int r = parse_short_flag(*p, &opt);
        if (r == 2) {
          print_usage(argv[0]);
          return 0;
        } else if (r == 0) {
          ok = 0;
          break;
        }
      }
      if (!ok) {
        fprintf(stderr, "%s: unknown option: %s\n", argv[0], argv[i]);
        print_usage(argv[0]);
        return 1;
      }
    } else {
      break;
    }
  }

  if (opt.number_nonblank)
    opt.number_lines = 0;

  long line_num = 0;
  int status = 0;

  if (i >= argc) {
    process_stream(stdin, &opt, &line_num);
    return 0;
  }

  for (; i < argc; i++) {
    FILE *f;
    int is_stdin = strcmp(argv[i], "-") == 0;

    if (is_stdin) {
      f = stdin;
    } else {
      f = fopen(argv[i], "rb");
      if (!f) {
        perror(argv[i]);
        status = 1;
        continue;
      }
    }

    process_stream(f, &opt, &line_num);

    if (!is_stdin)
      fclose(f);
  }
  return status;
}
