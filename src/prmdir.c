#include <stdio.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Usage: prmdir <directory>...\n");
    return 1;
  }
  int status = 0;
  for (int i = 1; i < argc; i++) {
    if (rmdir(argv[i]) != 0) {
      perror(argv[i]);
      status = 1;
    }
  }
  return status;
}
