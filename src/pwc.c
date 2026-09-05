#include <ctype.h>
#include <stdio.h>
#include <string.h>

typedef struct {
  unsigned long long lines;
  unsigned long long words;
  unsigned long long bytes;
} Counts;

typedef struct {
  int lines;
  int words;
  int bytes;
} Options;

static void print_usage(const char *prog) {
  printf("Usage: %s [OPTION]... [FILE]...\n", prog);
  printf("Print newline, word, and byte counts for each FILE.\n\n");
  printf("  -c             print byte counts\n");
  printf("  -l             print newline counts\n");
  printf("  -w             print word counts\n");
  printf("  -h, --help     display this help and exit\n");
}

static int count_stream(FILE *file, Counts *counts) {
  int character;
  int in_word = 0;
  while ((character = fgetc(file)) != EOF) {
    counts->bytes++;
    if (character == '\n')
      counts->lines++;
    if (isspace((unsigned char)character)) {
      in_word = 0;
    } else if (!in_word) {
      counts->words++;
      in_word = 1;
    }
  }
  return ferror(file) ? 1 : 0;
}

static void print_counts(const Counts *counts, const Options *opt, const char *name) {
  if (opt->lines)
    printf("%7llu", counts->lines);
  if (opt->words)
    printf("%7llu", counts->words);
  if (opt->bytes)
    printf("%7llu", counts->bytes);
  if (name)
    printf(" %s", name);
  putchar('\n');
}

int main(int argc, char *argv[]) {
  Options opt = {0};
  int i;
  for (i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-c") == 0) {
      opt.bytes = 1;
    } else if (strcmp(argv[i], "-l") == 0) {
      opt.lines = 1;
    } else if (strcmp(argv[i], "-w") == 0) {
      opt.words = 1;
    } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
      print_usage(argv[0]);
      return 0;
    } else if (strcmp(argv[i], "--") == 0) {
      i++;
      break;
    } else if (argv[i][0] == '-' && argv[i][1] != '\0' && strcmp(argv[i], "-") != 0) {
      fprintf(stderr, "%s: unknown option: %s\n", argv[0], argv[i]);
      return 1;
    } else {
      break;
    }
  }
  if (!opt.lines && !opt.words && !opt.bytes)
    opt.lines = opt.words = opt.bytes = 1;

  if (i == argc) {
    Counts counts = {0};
    int status = count_stream(stdin, &counts);
    print_counts(&counts, &opt, NULL);
    return status;
  }

  Counts total = {0};
  int successful_files = 0;
  int status = 0;
  for (; i < argc; i++) {
    FILE *file = strcmp(argv[i], "-") == 0 ? stdin : fopen(argv[i], "r");
    if (!file) {
      perror(argv[i]);
      status = 1;
      continue;
    }
    Counts counts = {0};
    if (count_stream(file, &counts) != 0) {
      perror(argv[i]);
      status = 1;
    } else {
      print_counts(&counts, &opt, argv[i]);
      total.lines += counts.lines;
      total.words += counts.words;
      total.bytes += counts.bytes;
      successful_files++;
    }
    if (file != stdin)
      fclose(file);
  }
  if (successful_files > 1)
    print_counts(&total, &opt, "total");
  return status;
}
