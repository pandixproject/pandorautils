#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  int number_lines;
  int number_nonblank;
  int show_ends;
  int squeeze_blank;
} Options;

void process_stream(FILE *f, const Options *opt, long *line_num) {
  char buf[4096];

  if (!opt->number_lines && !opt->number_nonblank && !opt->show_ends &&
      !opt->squeeze_blank) {
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

    if (opt->show_ends) {
      if (len > 0 && line[len - 1] == '\n') {
        fwrite(line, 1, len - 1, stdout);
        fputs("$\n", stdout);
      } else {
        fwrite(line, 1, len, stdout);
        fputs("$", stdout);
      }
    } else {
      fwrite(line, 1, len, stdout);
    }
  }
  free(line);
}

void print_usage(const char *prog) {
  printf("Usage: %s [OPTION]... [FILE]...\n", prog);
  printf("Concatenate FILE(s) to standard output.\n\n");
  printf("  -n         number all output lines\n");
  printf("  -b         number nonempty output lines\n");
  printf("  -E         display $ at end of each line\n");
  printf("  -s         squeeze multiple adjacent blank lines\n");
  printf("  -h, --help display this help and exit\n\n");
  printf("With no FILE, or when FILE is -, read standard input.\n");
}

int main(int argc, char *argv[]) {
  Options opt = {0};
  int i = 1;

  for (; i < argc; i++) {
    if (argv[i][0] == '-' && argv[i][1] != '\0') {
      if (strcmp(argv[i], "-n") == 0)
        opt.number_lines = 1;
      else if (strcmp(argv[i], "-b") == 0)
        opt.number_nonblank = 1;
      else if (strcmp(argv[i], "-E") == 0)
        opt.show_ends = 1;
      else if (strcmp(argv[i], "-s") == 0)
        opt.squeeze_blank = 1;
      else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
        print_usage(argv[0]);
        return 0;
      } else if (strcmp(argv[i], "-") == 0) {
        break;
      } else {
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
