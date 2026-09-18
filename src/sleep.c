#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char **argv) {
  char *end;
  double seconds;
  struct timespec request;
  struct timespec remaining;

  if (argc != 2) {
    fprintf(stderr, "usage: psleep SECONDS\n");
    return EXIT_FAILURE;
  }

  errno = 0;
  seconds = strtod(argv[1], &end);

  if (errno != 0 || *end != '\0' || seconds < 0) {
    fprintf(stderr, "psleep: invalid time: %s\n", argv[1]);
    return EXIT_FAILURE;
  }

  request.tv_sec = (time_t)seconds;
  request.tv_nsec = (long)((seconds - request.tv_sec) * 1000000000.0);

  while (nanosleep(&request, &remaining) == -1) {
    if (errno != EINTR) {
      perror("psleep");
      return EXIT_FAILURE;
    }

    request = remaining;
  }

  return EXIT_SUCCESS;
}
