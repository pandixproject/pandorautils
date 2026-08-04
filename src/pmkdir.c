#include <stdio.h>
#include <sys/stat.h>

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Usage: pmkdir <directory>...\n");
    return 1;
  }
  int status = 0;
  for (int i = 1; i < argc; i++) {
    if (mkdir(argv[i], 0755) != 0) {
      perror(argv[i]);
      status = 1;
    }
  }
  return status;
}
