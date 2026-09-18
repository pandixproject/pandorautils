#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int numeric, reverse_order;
static int compare(const void *a, const void *b) {
  const char *left = *(const char *const *)a, *right = *(const char *const *)b;
  int result;
  if (numeric) {
    double x = strtod(left, NULL), y = strtod(right, NULL);
    result = x < y ? -1 : x > y;
  } else result = strcmp(left, right);
  return reverse_order ? -result : result;
}
static void usage(const char *p) { printf("Usage: %s [-nru] [FILE]\nSort lines of text.\n  -n numeric sort\n  -r reverse result\n  -u output one copy of each equal line\n", p); }
int main(int argc, char **argv) {
  int unique = 0, i = 1;
  for (; i < argc; i++) {
    if (!strcmp(argv[i], "-n")) numeric = 1;
    else if (!strcmp(argv[i], "-r")) reverse_order = 1;
    else if (!strcmp(argv[i], "-u")) unique = 1;
    else if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) { usage(argv[0]); return 0; }
    else break;
  }
  if (argc - i > 1) { usage(argv[0]); return 1; }
  FILE *file = i == argc || !strcmp(argv[i], "-") ? stdin : fopen(argv[i], "r");
  if (!file) { perror(argv[i]); return 1; }
  char **lines = NULL, *line = NULL; size_t cap = 0, count = 0, allocated = 0;
  while (getline(&line, &cap, file) != -1) {
    if (count == allocated) { size_t next = allocated ? allocated * 2 : 32; char **more = realloc(lines, next * sizeof(*lines)); if (!more) { perror("psort"); return 1; } lines = more; allocated = next; }
    lines[count] = strdup(line); if (!lines[count++]) { perror("psort"); return 1; }
  }
  free(line); if (file != stdin) fclose(file);
  qsort(lines, count, sizeof(*lines), compare);
  for (size_t j = 0; j < count; j++) {
    if (!unique || j == 0 || strcmp(lines[j], lines[j - 1]))
      fputs(lines[j], stdout);
  }
  for (size_t j = 0; j < count; j++)
    free(lines[j]);
  free(lines); return 0;
}
