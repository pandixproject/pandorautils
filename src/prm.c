#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

typedef struct {
  int recursive;
  int force;
  int interactive;
  int verbose;
} Options;

int confirm(const char *path) {
  fprintf(stderr, "remove '%s'? (y/n) ", path);
  int c = getchar();
  int extra = c;
  while (extra != '\n' && extra != EOF)
    extra = getchar();
  return (c == 'y' || c == 'Y');
}

int remove_path(const char *path, const Options *opt) {
  struct stat st;

  if (lstat(path, &st) != 0) {
    if (!opt->force)
      perror(path);
    return opt->force ? 0 : 1;
  }

  if (S_ISDIR(st.st_mode)) {
    if (!opt->recursive) {
      fprintf(stderr, "prm: cannot remove '%s': Is a directory (use -r)\n",
              path);
      return 1;
    }

    if (opt->interactive && !confirm(path))
      return 0;

    DIR *d = opendir(path);
    if (!d) {
      if (!opt->force)
        perror(path);
      return opt->force ? 0 : 1;
    }

    int status = 0;
    struct dirent *entry;

    while ((entry = readdir(d)) != NULL) {
      if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
        continue;

      char child[4096];
      snprintf(child, sizeof(child), "%s/%s", path, entry->d_name);

      // skip the confirmation prompt for children, only ask once per top-level
      // target
      Options child_opt = *opt;
      child_opt.interactive = 0;

      if (remove_path(child, &child_opt) != 0)
        status = 1;
    }

    closedir(d);

    if (rmdir(path) != 0) {
      if (!opt->force) {
        perror(path);
        status = 1;
      }
    } else if (opt->verbose) {
      printf("removed directory '%s'\n", path);
    }

    return status;
  }

  if (opt->interactive && !confirm(path))
    return 0;

  if (unlink(path) != 0) {
    if (!opt->force) {
      perror(path);
      return 1;
    }
    return 0;
  }

  if (opt->verbose) {
    printf("removed '%s'\n", path);
  }

  return 0;
}

void print_usage(const char *prog) {
  printf("Usage: %s [OPTION]... FILE...\n", prog);
  printf("Remove (unlink) the FILE(s).\n\n");
  printf(
      "  -r             remove directories and their contents recursively\n");
  printf("  -f             ignore nonexistent files, never prompt\n");
  printf("  -i             prompt before every removal\n");
  printf("  -v             explain what is being done\n");
  printf("  -h, --help     display this help and exit\n");
}

int main(int argc, char *argv[]) {
  Options opt = {0};
  int i = 1;

  for (; i < argc; i++) {
    if (strcmp(argv[i], "-r") == 0) {
      opt.recursive = 1;
    } else if (strcmp(argv[i], "-f") == 0) {
      opt.force = 1;
    } else if (strcmp(argv[i], "-i") == 0) {
      opt.interactive = 1;
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
      print_usage(argv[0]);
      return 1;
    } else {
      break;
    }
  }

  if (i >= argc) {
    if (opt.force)
      return 0;
    print_usage(argv[0]);
    return 1;
  }

  int status = 0;

  for (; i < argc; i++) {
    if (remove_path(argv[i], &opt) != 0)
      status = 1;
  }

  return status;
}
