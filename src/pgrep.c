#include <regex.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char *argv[]) {
  regex_t regex;
  int ret;
  int found = 0;

  if (argc < 2) {
    fprintf(stderr, "Usage: pgrep PATTERN [FILE...]\n");
    return 2;
  }

  ret = regcomp(&regex, argv[1], REG_EXTENDED);
  if (ret != 0) {
    char error[256];
    regerror(ret, &regex, error, sizeof(error));
    fprintf(stderr, "pgrep: %s\n", error);
    return 2;
  }

  if (argc == 2) {
    char *line = NULL;
    size_t len = 0;

    while (getline(&line, &len, stdin) != -1) {
      if (regexec(&regex, line, 0, NULL, 0) == 0) {
        fputs(line, stdout);
        found = 1;
      }
    }

    free(line);
  } else {
    for (int i = 2; i < argc; i++) {
      FILE *file = fopen(argv[i], "r");

      if (file == NULL) {
        perror(argv[i]);
        regfree(&regex);
        return 2;
      }

      char *line = NULL;
      size_t len = 0;

      while (getline(&line, &len, file) != -1) {
        if (regexec(&regex, line, 0, NULL, 0) == 0) {
          fputs(line, stdout);
          found = 1;
        }
      }
      free(line);
      fclose(file);
    }
  }

  regfree(&regex);

  return found ? 0 : 1;
}
