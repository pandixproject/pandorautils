#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

static const char *file_type(mode_t mode) {
  if (S_ISREG(mode))
    return "regular file";
  if (S_ISDIR(mode))
    return "directory";
  if (S_ISLNK(mode))
    return "symbolic link";
  if (S_ISCHR(mode))
    return "character device";
  if (S_ISBLK(mode))
    return "block device";
  if (S_ISFIFO(mode))
    return "FIFO";
  if (S_ISSOCK(mode))
    return "socket";
  return "unknown";
}

static void print_time(const char *label, time_t value) {
  char buffer[64];
  struct tm *tm_info = localtime(&value);

  if (tm_info == NULL)
    return;

  if (strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", tm_info) == 0)
    return;

  printf("%s: %s\n", label, buffer);
}

static void print_file(const char *path) {
  struct stat st;
  struct passwd *user;
  struct group *group;

  if (lstat(path, &st) == -1) {
    perror(path);
    return;
  }

  user = getpwuid(st.st_uid);
  group = getgrgid(st.st_gid);

  printf("  File: %s\n", path);
  printf("  Size: %lld bytes\n", (long long)st.st_size);
  printf("  Type: %s\n", file_type(st.st_mode));
  printf("  Mode: %04o\n", st.st_mode & 07777);
  printf("  Owner: %s\n", user ? user->pw_name : "unknown");
  printf("  Group: %s\n", group ? group->gr_name : "unknown");
  printf("  Links: %lu\n", (unsigned long)st.st_nlink);

  print_time("Access", st.st_atime);
  print_time("Modify", st.st_mtime);
  print_time("Change", st.st_ctime);
}

int main(int argc, char **argv) {
  int i;

  if (argc < 2) {
    fprintf(stderr, "usage: pstat FILE...\n");
    return EXIT_FAILURE;
  }

  for (i = 1; i < argc; i++) {
    if (argc > 2)
      printf("  File: %s\n", argv[i]);

    print_file(argv[i]);

    if (i + 1 < argc)
      putchar('\n');
  }

  return EXIT_SUCCESS;
}
