#include <ctype.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void usage(const char *name) {
  printf("Usage: %s FORMAT [ARGUMENT]...\n", name);
  puts("Print FORMAT, substituting %s, %c, %d, %i, %u, %o, %x, %X, and %%.");
}

static const char *escape(const char *s, int *stopped) {
  s++;
  switch (*s) {
  case 'n': putchar('\n'); return s + 1;
  case 't': putchar('\t'); return s + 1;
  case 'r': putchar('\r'); return s + 1;
  case '\\': putchar('\\'); return s + 1;
  case 'a': putchar('\a'); return s + 1;
  case 'b': putchar('\b'); return s + 1;
  case 'f': putchar('\f'); return s + 1;
  case 'v': putchar('\v'); return s + 1;
  case 'c': *stopped = 1; return s + 1;
  case 0: putchar('\\'); return s;
  default: putchar('\\'); return s;
  }
}

static long long number(const char *s) {
  char *end;
  errno = 0;
  long long n = strtoll(s, &end, 0);
  return (errno || end == s) ? 0 : n;
}

int main(int argc, char **argv) {
  if (argc == 2 && (!strcmp(argv[1], "--help") || !strcmp(argv[1], "-h"))) { usage(argv[0]); return 0; }
  if (argc < 2) { usage(argv[0]); return 1; }
  const char *fmt = argv[1];
  int arg = 2, stopped = 0;
  do {
    while (*fmt && !stopped) {
      if (*fmt == '\\') {
        fmt = escape(fmt, &stopped); continue;
      }
      if (*fmt != '%') { putchar(*fmt++); continue; }
      fmt++;
      if (*fmt == '%') { putchar('%'); fmt++; continue; }
      char spec[128]; size_t n = 0;
      spec[n++] = '%';
      while (*fmt && strchr("-+ #0", *fmt) && n < sizeof(spec)-2) spec[n++] = *fmt++;
      while (*fmt && isdigit((unsigned char)*fmt) && n < sizeof(spec)-2) spec[n++] = *fmt++;
      if (*fmt == '.') { spec[n++] = *fmt++; while (*fmt && isdigit((unsigned char)*fmt) && n < sizeof(spec)-2) spec[n++] = *fmt++; }
      if (!*fmt) { fputs("printf: incomplete format\n", stderr); return 1; }
      char conv = *fmt++; spec[n++] = conv; spec[n] = 0;
      const char *value = arg < argc ? argv[arg++] : "";
      switch (conv) {
      case 's': printf(spec, value); break;
      case 'c': printf(spec, *value ? *value : '\0'); break;
      case 'd': case 'i': printf(spec, (int)number(value)); break;
      case 'u': printf(spec, (unsigned int)number(value)); break;
      case 'o': printf(spec, (unsigned int)number(value)); break;
      case 'x': case 'X': printf(spec, (unsigned int)number(value)); break;
      default: fprintf(stderr, "printf: unsupported conversion: %%%c\n", conv); return 1;
      }
    }
  } while (!stopped && arg < argc);
  return ferror(stdout) ? 1 : 0;
}
