#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

typedef struct {
  int force;
  int interactive;
  int verbose;
} Options;

static void print_usage(const char *prog) {
  printf("Usage: %s [OPTION]... SOURCE DEST\n", prog);
  printf("Rename SOURCE to DEST.\n\n");
  printf("  -f             do not prompt before overwriting\n");
  printf("  -i             prompt before overwriting\n");
  printf("  -v             explain what is being done\n");
  printf("  -h, --help     display this help and exit\n");
}

static const char *base_name(const char *path) {
  const char *last = strrchr(path, '/');
  return last ? last + 1 : path;
}

static int confirm(const char *path) {
  char answer[16];
  fprintf(stderr, "overwrite '%s'? ", path);
  return fgets(answer, sizeof(answer), stdin) &&
         (answer[0] == 'y' || answer[0] == 'Y');
}

int main(int argc, char *argv[]) {
  Options opt = {0};
  int i;

  for (i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-f") == 0) {
      opt.force = 1;
      opt.interactive = 0;
    } else if (strcmp(argv[i], "-i") == 0) {
      opt.interactive = 1;
      opt.force = 0;
    } else if (strcmp(argv[i], "-v") == 0) {
      opt.verbose = 1;
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

  if (argc - i != 2) {
    print_usage(argv[0]);
    return 1;
  }

  const char *source = argv[i];
  const char *destination = argv[i + 1];
  char destination_path[PATH_MAX];
  struct stat destination_stat;

  if (stat(destination, &destination_stat) == 0 && S_ISDIR(destination_stat.st_mode)) {
    int written = snprintf(destination_path, sizeof(destination_path), "%s/%s",
                           destination, base_name(source));
    if (written < 0 || (size_t)written >= sizeof(destination_path)) {
      fprintf(stderr, "%s: destination path is too long\n", argv[0]);
      return 1;
    }
    destination = destination_path;
  }

  if (opt.interactive && !opt.force && lstat(destination, &destination_stat) == 0 &&
      !confirm(destination))
    return 0;

  if (rename(source, destination) != 0) {
    if (errno == EXDEV)
      fprintf(stderr, "%s: cannot move '%s' across file systems\n", argv[0], source);
    else
      perror(source);
    return 1;
  }

  if (opt.verbose)
    printf("'%s' -> '%s'\n", source, destination);
  return 0;
}
