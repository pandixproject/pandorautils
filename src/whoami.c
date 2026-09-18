#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct {
  int real;
  int show_uid;
  int show_all;
} Options;

void print_usage(const char *prog) {
  printf("Usage: %s [OPTION]...\n", prog);
  printf(
      "Print the user name associated with the current effective user ID.\n\n");
  printf(
      "  -r             use the real user ID instead of the effective one\n");
  printf("  -u             also print the numeric user ID\n");
  printf("  -a             print uid, gid, and supplementary group IDs\n");
  printf("  -h, --help     display this help and exit\n");
}

int main(int argc, char *argv[]) {
  Options opt = {0};

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-r") == 0) {
      opt.real = 1;
    } else if (strcmp(argv[i], "-u") == 0) {
      opt.show_uid = 1;
    } else if (strcmp(argv[i], "-a") == 0) {
      opt.show_all = 1;
    } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
      print_usage(argv[0]);
      return 0;
    } else {
      fprintf(stderr, "%s: unknown option: %s\n", argv[0], argv[i]);
      print_usage(argv[0]);
      return 1;
    }
  }

  uid_t uid = opt.real ? getuid() : geteuid();
  struct passwd *pw = getpwuid(uid);

  if (!pw) {
    perror("pwhoami");
    return 1;
  }

  if (opt.show_all) {
    gid_t gid = opt.real ? getgid() : getegid();
    printf("uid=%d(%s) gid=%d", uid, pw->pw_name, gid);

    struct group *gr = getgrgid(gid);
    if (gr)
      printf("(%s)", gr->gr_name);

    int ngroups = getgroups(0, NULL);
    if (ngroups > 0) {
      gid_t *groups = malloc((size_t)ngroups * sizeof(*groups));
      if (!groups) {
        perror("pwhoami");
        return 1;
      }
      if (getgroups(ngroups, groups) == -1) {
        perror("pwhoami");
        free(groups);
        return 1;
      }
      printf(" groups=");
      for (int i = 0; i < ngroups; i++) {
        struct group *g = getgrgid(groups[i]);
        if (i > 0)
          printf(",");
        printf("%d", groups[i]);
        if (g)
          printf("(%s)", g->gr_name);
      }
      free(groups);
    }
    printf("\n");
    return 0;
  }

  if (opt.show_uid) {
    printf("%s (%d)\n", pw->pw_name, uid);
  } else {
    printf("%s\n", pw->pw_name);
  }

  return 0;
}
