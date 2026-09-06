#include <errno.h>
#include <fcntl.h>
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
  printf("Usage: %s [OPTION]... SOURCE... DEST\n", prog);
  printf("Rename SOURCE(s) to DEST.\n\n");
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

static int copy_across_file_systems(const char *source, const char *destination) {
  struct stat st;
  if (lstat(source, &st) != 0) { perror(source); return 1; }
  if (S_ISLNK(st.st_mode)) {
    char target[PATH_MAX]; ssize_t n = readlink(source, target, sizeof(target) - 1);
    if (n < 0) { perror(source); return 1; }
    target[n] = '\0'; if (symlink(target, destination) != 0) { perror(destination); return 1; }
    return unlink(source) ? (perror(source), 1) : 0;
  }
  if (!S_ISREG(st.st_mode)) { fprintf(stderr, "pmv: cannot move directory '%s' across file systems\n", source); return 1; }
  int in = open(source, O_RDONLY), out;
  if (in < 0) { perror(source); return 1; }
  out = open(destination, O_WRONLY | O_CREAT | O_TRUNC, st.st_mode & 0777);
  if (out < 0) { perror(destination); close(in); return 1; }
  char buffer[8192]; ssize_t n; int status = 0;
  while ((n = read(in, buffer, sizeof(buffer))) > 0) {
    ssize_t offset = 0; while (offset < n) { ssize_t written = write(out, buffer + offset, (size_t)(n - offset)); if (written < 0) { perror(destination); status = 1; break; } offset += written; }
    if (status) break;
  }
  if (n < 0) { perror(source); status = 1; }
  if (close(in) || close(out)) status = 1;
  if (!status && unlink(source)) { perror(source); status = 1; }
  return status;
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

  if (argc - i < 2) {
    print_usage(argv[0]);
    return 1;
  }

  const char *final_destination = argv[argc - 1];
  struct stat destination_stat;
  int destination_is_dir = stat(final_destination, &destination_stat) == 0 && S_ISDIR(destination_stat.st_mode);
  int sources = argc - i - 1;
  if (sources > 1 && !destination_is_dir) { fprintf(stderr, "%s: target '%s' is not a directory\n", argv[0], final_destination); return 1; }
  int status = 0;
  for (; i < argc - 1; i++) {
    const char *source = argv[i], *destination = final_destination; char destination_path[PATH_MAX];
    if (destination_is_dir) { int written = snprintf(destination_path, sizeof(destination_path), "%s/%s", final_destination, base_name(source)); if (written < 0 || (size_t)written >= sizeof(destination_path)) { fprintf(stderr, "%s: destination path is too long\n", argv[0]); status = 1; continue; } destination = destination_path; }
    if (opt.interactive && !opt.force && lstat(destination, &destination_stat) == 0 && !confirm(destination)) continue;
    if (rename(source, destination) != 0) {
      if (errno != EXDEV) { perror(source); status = 1; continue; }
      if (copy_across_file_systems(source, destination)) { status = 1; continue; }
    }
    if (opt.verbose) printf("'%s' -> '%s'\n", source, destination);
  }
  return status;
}
