#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void usage(const char *p) { printf("Usage: %s [-cdu] [INPUT [OUTPUT]]\nReport or omit repeated lines.\n  -c prefix lines by their counts\n  -d only print duplicate lines\n  -u only print unique lines\n", p); }
static void output(const char *line, unsigned long count, int counts, int duplicates, int uniques) {
  if ((duplicates && count < 2) || (uniques && count != 1)) return;
  if (counts) printf("%7lu ", count);
  fputs(line, stdout);
}
int main(int argc, char **argv) {
  int counts = 0, duplicates = 0, uniques = 0, i = 1;
  for (; i < argc; i++) {
    if (!strcmp(argv[i], "-c")) counts = 1;
    else if (!strcmp(argv[i], "-d")) duplicates = 1;
    else if (!strcmp(argv[i], "-u")) uniques = 1;
    else if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) { usage(argv[0]); return 0; }
    else break;
  }
  if (argc - i > 2) { usage(argv[0]); return 1; }
  FILE *in = i == argc || !strcmp(argv[i], "-") ? stdin : fopen(argv[i], "r");
  if (!in) { perror(argv[i]); return 1; }
  if (argc - i == 2 && !freopen(argv[i + 1], "w", stdout)) { perror(argv[i + 1]); return 1; }
  char *line = NULL, *previous = NULL; size_t cap = 0; unsigned long count = 0;
  while (getline(&line, &cap, in) != -1) {
    if (previous && !strcmp(previous, line)) { count++; continue; }
    if (previous) output(previous, count, counts, duplicates, uniques);
    free(previous); previous = strdup(line); if (!previous) { perror("puniq"); return 1; } count = 1;
  }
  if (previous) output(previous, count, counts, duplicates, uniques);
  int status = ferror(in) ? 1 : 0;
  free(previous); free(line); if (in != stdin && fclose(in)) status = 1; return status;
}
