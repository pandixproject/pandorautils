#include <errno.h>
#include <stdio.h>
#include <string.h>

static void usage(const char *p) { printf("Usage: %s [-a] [FILE]...\nCopy standard input to each FILE and standard output.\n", p); }
int main(int argc, char **argv) {
  int append = 0, i = 1;
  if (argc > 1 && !strcmp(argv[1], "-a")) { append = 1; i++; }
  if (argc > 1 && (!strcmp(argv[1], "-h") || !strcmp(argv[1], "--help"))) { usage(argv[0]); return 0; }
  FILE *files[argc > 1 ? argc - 1 : 1]; int count = 0, status = 0;
  for (; i < argc; i++) { files[count] = fopen(argv[i], append ? "ab" : "wb"); if (!files[count]) { perror(argv[i]); status = 1; } else count++; }
  char buf[8192]; size_t n;
  while ((n = fread(buf, 1, sizeof(buf), stdin)) > 0) {
    if (fwrite(buf, 1, n, stdout) != n) status = 1;
    for (int j = 0; j < count; j++) if (fwrite(buf, 1, n, files[j]) != n) { perror("ptee"); status = 1; }
  }
  if (ferror(stdin)) { perror("stdin"); status = 1; }
  for (int j = 0; j < count; j++) if (fclose(files[j])) status = 1;
  return status;
}
