#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

static void print_usage(const char *prog) {
  printf("Usage: %s MODE FILE...\n", prog);
  printf("Change the mode of each FILE to the octal MODE.\n\n");
  printf("  -h, --help     display this help and exit\n");
}

static int parse_octal_mode(const char *text, mode_t *mode) {
  char *end;
  errno = 0;
  unsigned long value = strtoul(text, &end, 8);
  if (errno != 0 || *text == '\0' || *end != '\0' || value > 07777)
    return -1;
  *mode = (mode_t)value;
  return 0;
}

static int parse_symbolic_mode(const char *text, mode_t old, mode_t *mode) {
  const char *p = text;
  mode_t result = old;
  while (*p) {
    int who = 0;
    while (*p == 'u' || *p == 'g' || *p == 'o' || *p == 'a') {
      who |= *p == 'u' ? 1 : *p == 'g' ? 2 : *p == 'o' ? 4 : 7;
      p++;
    }
    if (!who) who = 7;
    char op = *p++;
    if (op != '+' && op != '-' && op != '=') return -1;
    mode_t bits = 0;
    int saw_permission = 0;
    while (*p && *p != ',') {
      saw_permission = 1;
      if (*p == 'r') { if (who & 1) bits |= S_IRUSR; if (who & 2) bits |= S_IRGRP; if (who & 4) bits |= S_IROTH; }
      else if (*p == 'w') { if (who & 1) bits |= S_IWUSR; if (who & 2) bits |= S_IWGRP; if (who & 4) bits |= S_IWOTH; }
      else if (*p == 'x' || (*p == 'X' && (S_ISDIR(old) || (old & 0111)))) { if (who & 1) bits |= S_IXUSR; if (who & 2) bits |= S_IXGRP; if (who & 4) bits |= S_IXOTH; }
      else if (*p == 's') { if (who & 1) bits |= S_ISUID; if (who & 2) bits |= S_ISGID; }
      else if (*p == 't') { if (who & 4) bits |= 01000; }
      else return -1;
      p++;
    }
    if (!saw_permission && op != '=') return -1;
    mode_t mask = 0;
    if (who & 1) mask |= S_IRWXU | S_ISUID;
    if (who & 2) mask |= S_IRWXG | S_ISGID;
    if (who & 4) mask |= S_IRWXO | 01000;
    if (op == '+') result |= bits;
    else if (op == '-') result &= ~bits;
    else result = (result & ~mask) | bits;
    if (*p == ',') p++;
  }
  *mode = result;
  return 0;
}

int main(int argc, char *argv[]) {
  if (argc == 2 && (strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)) {
    print_usage(argv[0]);
    return 0;
  }
  if (argc < 3) {
    print_usage(argv[0]);
    return 1;
  }

  int status = 0;
  for (int i = 2; i < argc; i++) {
    struct stat st;
    if (stat(argv[i], &st) != 0) {
      perror(argv[i]);
      status = 1;
      continue;
    }
    mode_t mode;
    if (parse_octal_mode(argv[1], &mode) != 0 &&
        parse_symbolic_mode(argv[1], st.st_mode, &mode) != 0) {
      fprintf(stderr, "%s: invalid mode: %s\n", argv[0], argv[1]);
      return 1;
    }
    if (chmod(argv[i], mode) != 0) {
      perror(argv[i]);
      status = 1;
    }
  }
  return status;
}
