#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static void print_usage(const char *prog) {
  printf("Usage: %s [OPTION]... [FILE]...\n", prog);
  printf("Print the last lines of each FILE.\n\n");
  printf("  -n NUMBER      print the last NUMBER lines instead of 10\n");
  printf("  -c NUMBER      print the last NUMBER bytes\n");
  printf("  -f             output appended data as the file grows\n");
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

static int print_tail_bytes(FILE *file, size_t count) {
  if (!count) return 0;
  unsigned char *bytes = malloc(count);
  if (!bytes) return 1;
  size_t total = 0;
  int c;
  while ((c = fgetc(file)) != EOF) bytes[total++ % count] = (unsigned char)c;
  size_t output = total < count ? total : count, start = total < count ? 0 : total % count;
  for (size_t i = 0; i < output; i++) fputc(bytes[(start + i) % count], stdout);
  free(bytes);
  return ferror(file) ? 1 : 0;
}

static int follow_file(FILE *file) {
  clearerr(file);
  for (;;) {
    int c;
    while ((c = fgetc(file)) != EOF) {
      if (fputc(c, stdout) == EOF) return 0;
    }
    if (ferror(file)) return 1;
    clearerr(file);
    if (fflush(stdout) == EOF) return 0;
    sleep(1);
  }
}

int main(int argc, char *argv[]) {
  size_t count = 10;
  int bytes = 0, follow = 0;
  int i;
  for (i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-n") == 0) {
      if (++i >= argc || parse_count(argv[i], &count) != 0) {
        fprintf(stderr, "%s: invalid line count\n", argv[0]);
        return 1;
      }
    } else if (strcmp(argv[i], "-c") == 0) {
      if (++i >= argc || parse_count(argv[i], &count) != 0) {
        fprintf(stderr, "%s: invalid byte count\n", argv[0]);
        return 1;
      }
      bytes = 1;
    } else if (strcmp(argv[i], "-f") == 0) {
      follow = 1;
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
  if (files == 0) {
    int result = bytes ? print_tail_bytes(stdin, count) : print_tail(stdin, count);
    return result || !follow ? result : follow_file(stdin);
  }
  if (follow && files != 1) {
    fprintf(stderr, "%s: -f currently supports one file\n", argv[0]);
    return 1;
  }

  for (int index = i; index < argc; index++) {
    FILE *file = strcmp(argv[index], "-") == 0 ? stdin : fopen(argv[index], "r");
    if (!file) {
      perror(argv[index]);
      status = 1;
      continue;
    }
    if (files > 1)
      printf("%s==> %s <==\n", index == i ? "" : "\n", argv[index]);
    if ((bytes ? print_tail_bytes(file, count) : print_tail(file, count)) != 0) {
      perror(argv[index]);
      status = 1;
    }
    if (follow && !status) {
      int result = follow_file(file);
      if (result) { perror(argv[index]); status = 1; }
    }
    if (file != stdin) fclose(file);
  }
  return status;
}
