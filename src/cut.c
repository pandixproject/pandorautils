#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct { unsigned long first, last; } Range;
static Range ranges[256];
static size_t nranges;
static int selected(unsigned long n) {
  for (size_t i = 0; i < nranges; i++) if (n >= ranges[i].first && n <= ranges[i].last) return 1;
  return 0;
}
static int parse_list(const char *s) {
  while (*s) {
    char *end; unsigned long a, b;
    if (*s == '-') a = 1;
    else { a = strtoul(s, &end, 10); if (end == s || a == 0) return 0; s = end; }
    b = a;
    if (*s == '-') {
      s++;
      if (*s) { b = strtoul(s, &end, 10); if (end == s || b < a) return 0; s = end; }
      else b = ~0UL;
    }
    if (nranges == sizeof(ranges)/sizeof(ranges[0])) return 0;
    ranges[nranges++] = (Range){a,b};
    if (*s == ',') { s++; if (!*s) return 0; }
    else if (*s) return 0;
  }
  return nranges != 0;
}
static void usage(const char *p) {
  fprintf(stderr, "Usage: %s (-b LIST | -c LIST | -f LIST) [-d DELIM] [-s] [FILE...]\n", p);
  fputs("Select bytes, characters, or delimiter-separated fields. LIST uses N, N-M, N-, or -M ranges.\n", stderr);
}
static int process(FILE *f, const char *name, int fields, int only, unsigned char delim) {
  char *line = NULL; size_t cap = 0; ssize_t len; int status = 0;
  while ((len = getline(&line, &cap, f)) >= 0) {
    if (!fields) {
      size_t content = (size_t)len;
      int newline = content > 0 && line[content - 1] == '\n';
      if (newline) content--;
      for (size_t i = 0; i < content; i++) if (selected((unsigned long)i + 1)) putchar(line[i]);
      if (newline) putchar('\n');
    } else {
      int has_delim = memchr(line, delim, (size_t)len) != NULL;
      if (only && !has_delim) { fwrite(line, 1, (size_t)len, stdout); continue; }
      unsigned long field = 1; size_t start = 0; int emitted = 0;
      for (size_t i = 0; i <= (size_t)len; i++) if (i == (size_t)len || line[i] == delim || line[i] == '\n') {
        if (selected(field)) { if (emitted) putchar(delim); fwrite(line + start, 1, i - start, stdout); emitted = 1; }
        if (i < (size_t)len && line[i] == delim) { field++; start = i + 1; }
        else { if (i < (size_t)len && line[i] == '\n') putchar('\n'); break; }
      }
    }
  }
  if (ferror(f)) { perror(name); status = 1; }
  free(line); return status;
}
int main(int argc, char **argv) {
  int mode = 0, only = 0; unsigned char delim = '\t'; int i;
  for (i = 1; i < argc; i++) {
    if (!strcmp(argv[i], "-s")) only = 1;
    else if (!strcmp(argv[i], "-d") && i + 1 < argc) { if (!argv[++i][0] || argv[i][1]) { usage(argv[0]); return 1; } delim = (unsigned char)argv[i][0]; }
    else if ((!strcmp(argv[i], "-b") || !strcmp(argv[i], "-c") || !strcmp(argv[i], "-f")) && i + 1 < argc) {
      int m = argv[i][1] == 'f' ? 3 : argv[i][1] == 'c' ? 2 : 1;
      if (mode || !parse_list(argv[++i])) { usage(argv[0]); return 1; } mode = m;
    } else if (!strcmp(argv[i], "--")) { i++; break; }
    else if (argv[i][0] == '-') { usage(argv[0]); return 1; }
    else break;
  }
  if (!mode || (only && mode != 3)) { usage(argv[0]); return 1; }
  int status = 0;
  if (i == argc) return process(stdin, "stdin", mode == 3, only, delim);
  for (; i < argc; i++) {
    FILE *f = !strcmp(argv[i], "-") ? stdin : fopen(argv[i], "r");
    if (!f) { perror(argv[i]); status = 1; continue; }
    if (process(f, argv[i], mode == 3, only, delim)) status = 1;
    if (f != stdin) fclose(f);
  }
  return status;
}
