#include <limits.h>
#include <stdio.h>
#include <unistd.h>

int main(void) {
  char buf[PATH_MAX];
  if (getcwd(buf, sizeof(buf)) != NULL) {
    printf("%s\n", buf);
    return 0;
  }
  perror("ppwd");
  return 1;
}
