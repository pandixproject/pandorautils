#include <dirent.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[]) {
  DIR *proc;
  struct dirent *entry;

  if (argc != 2) {
    fprintf(stderr, "Usage: pangrep PATTERN\n");
    return 2;
  }

  proc = opendir("/proc");

  if (proc == NULL) {
    perror("pangrep: /proc");
    return 2;
  }

  while ((entry = readdir(proc)) != NULL) {
    char path[268];
    FILE *file;
    char name[256];

    if (strspn(entry->d_name, "0123456789") != strlen(entry->d_name))
      continue;

    snprintf(path, sizeof(path), "/proc/%s/comm", entry->d_name);

    file = fopen(path, "r");

    if (file == NULL)
      continue;

    if (fgets(name, sizeof(name), file) != NULL) {
      name[strcspn(name, "\n")] = '\0';

      if (strstr(name, argv[1]) != NULL)
        printf("%s %s\n", entry->d_name, name);
    }

    fclose(file);
  }

  closedir(proc);

  return 0;
}
