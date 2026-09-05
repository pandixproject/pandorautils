#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void print_usage(const char *prog) {
  printf("Usage: %s [OPTION]... [FILE]...\n", prog);
  printf("Print the last lines of each FILE.\n\n");
  printf("  -n NUMBER      print the last NUMBER lines instead of 10\n");
  printf("  -h, --help     display this help and exit\n");
}

static int parse_count(const char *text, size_t *count) {
  char *end;
  errno = 0;
  unsigned long value = strtoul(text, &end, 10);
  if (errno != 0 || *text == '\0' || *end != '\0')
    return -1;
  *count = (size_t)value;
  return 0;
}

static int print_tail(FILE *file, size_t count) {
  if (count == 0)
    return 0;
  char **lines = calloc(count, sizeof(*lines));
  char *line = NULL;
  size_t capacity = 0;
  size_t total = 0;
  if (!lines)
    return 1;

  while (getline(&line, &capacity, file) != -1) {
    size_t index = total % count;
    size_t length = strlen(line) + 1;
    char *copy = malloc(length);
    if (!copy) {
      free(line);
      for (size_t j = 0; j < count; j++)
        free(lines[j]);
      free(lines);
      return 1;
    }
    memcpy(copy, line, length);
    free(lines[index]);
    lines[index] = copy;
    total++;
  }
  free(line);

  size_t output_count = total < count ? total : count;
  size_t start = total < count ? 0 : total % count;
  for (size_t j = 0; j < output_count; j++) {
    size_t index = (start + j) % count;
    fputs(lines[index], stdout);
    free(lines[index]);
  }
  free(lines);
  return ferror(file) ? 1 : 0;
}

int main(int argc, char *argv[]) {
  size_t count = 10;
  int i;
  for (i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-n") == 0) {
      if (++i >= argc || parse_count(argv[i], &count) != 0) {
        fprintf(stderr, "%s: invalid line count\n", argv[0]);
        return 1;
      }
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

  int files = argc - i;
  int status = 0;
  if (files == 0)
    return print_tail(stdin, count);

  for (int index = i; index < argc; index++) {
    FILE *file = strcmp(argv[index], "-") == 0 ? stdin : fopen(argv[index], "r");
    if (!file) {
      perror(argv[index]);
      status = 1;
      continue;
    }
    if (files > 1)
      printf("%s==> %s <==\n", index == i ? "" : "\n", argv[index]);
    if (print_tail(file, count) != 0) {
      perror(argv[index]);
      status = 1;
    }
    if (file != stdin)
      fclose(file);
  }
  return status;
}
