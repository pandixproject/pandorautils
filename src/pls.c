#include <dirent.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>

void format_time(time_t t, char *buf, size_t bufsize) {
  struct tm *tm_info = localtime(&t);
  strftime(buf, bufsize, "%Y-%m-%d %H:%M:%S", tm_info);
}

int main(int argc, char *argv[]) {
  const char *path = argc > 1 ? argv[1] : ".";
  DIR *d = opendir(path);
  if (!d) {
    perror("pls");
    return 1;
  }

  struct dirent *entry;
  while ((entry = readdir(d)) != NULL) {
    if (entry->d_name[0] == '.')
      continue;
    char fullpath[4096];
    snprintf(fullpath, sizeof(fullpath), "%s/%s", path, entry->d_name);

    struct stat st;
    if (lstat(fullpath, &st) == -1) {
      perror("stat");
      continue;
    }

    struct passwd *pw = getpwuid(st.st_uid);
    const char *owner = pw ? pw->pw_name : "unknown";

    char size_str[32];
    if (S_ISDIR(st.st_mode)) {
      snprintf(size_str, sizeof(size_str), "DIR");
    } else {
      snprintf(size_str, sizeof(size_str), "%lld", (long long)st.st_size);
    }

    char mtime_str[64];
    format_time(st.st_mtime, mtime_str, sizeof(mtime_str));

    printf("%-25s %-8s %-10s modified: %-20s\n", entry->d_name, size_str,
           owner, mtime_str);
  }
  closedir(d);
  return 0;
}
