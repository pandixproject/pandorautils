#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <utime.h>

typedef struct {
  int access_time;
  int modification_time;
  int no_create;
  const char *reference;
} Options;

static void print_usage(const char *prog) {
  printf("Usage: %s [OPTION]... FILE...\n", prog);
  printf("Update the access and modification times of each FILE.\n\n");
  printf("  -a             change only the access time\n");
  printf("  -c             do not create any files\n");
  printf("  -m             change only the modification time\n");
  printf("  -r FILE        use FILE's times instead of the current time\n");
  printf("  -h, --help     display this help and exit\n");
}

int main(int argc, char *argv[]) {
  Options opt = {0};
  int i;
  struct utimbuf reference_times;
  int has_reference = 0;

  for (i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-a") == 0) {
      opt.access_time = 1;
    } else if (strcmp(argv[i], "-c") == 0) {
      opt.no_create = 1;
    } else if (strcmp(argv[i], "-m") == 0) {
      opt.modification_time = 1;
    } else if (strcmp(argv[i], "-r") == 0) {
      if (++i >= argc) {
        fprintf(stderr, "%s: -r requires an argument\n", argv[0]);
        return 1;
      }
      opt.reference = argv[i];
    } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
      print_usage(argv[0]);
      return 0;
    } else if (strcmp(argv[i], "--") == 0) {
      i++;
      break;
    } else if (argv[i][0] == '-' && argv[i][1] != '\0') {
      fprintf(stderr, "%s: unknown option: %s\n", argv[0], argv[i]);
      return 1;
    } else {
      break;
    }
  }

  if (i == argc) {
    print_usage(argv[0]);
    return 1;
  }

  if (opt.reference) {
    struct stat st;
    if (stat(opt.reference, &st) != 0) {
      perror(opt.reference);
      return 1;
    }
    reference_times.actime = st.st_atime;
    reference_times.modtime = st.st_mtime;
    has_reference = 1;
  }

  int status = 0;
  for (; i < argc; i++) {
    struct stat st;
    if (stat(argv[i], &st) != 0) {
      if (errno == ENOENT && !opt.no_create) {
        FILE *file = fopen(argv[i], "ab");
        if (!file) {
          perror(argv[i]);
          status = 1;
          continue;
        }
        fclose(file);
        if (stat(argv[i], &st) != 0) {
          perror(argv[i]);
          status = 1;
          continue;
        }
      } else if (errno != ENOENT || !opt.no_create) {
        perror(argv[i]);
        status = 1;
        continue;
      } else {
        continue;
      }
    }

    if (!has_reference && !opt.access_time && !opt.modification_time) {
      if (utime(argv[i], NULL) != 0) {
        perror(argv[i]);
        status = 1;
      }
      continue;
    }
    if (!opt.access_time && !opt.modification_time) {
      if (utime(argv[i], &reference_times) != 0) {
        perror(argv[i]);
        status = 1;
      }
      continue;
    }
    struct utimbuf times = has_reference ? reference_times :
                                              (struct utimbuf){st.st_atime, st.st_mtime};
    if (!opt.access_time)
      times.actime = st.st_atime;
    if (!opt.modification_time)
      times.modtime = st.st_mtime;
    if (utime(argv[i], &times) != 0) {
      perror(argv[i]);
      status = 1;
    }
  }
  return status;
}
