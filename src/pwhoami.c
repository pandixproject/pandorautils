#include <pwd.h>
#include <stdio.h>
#include <unistd.h>

int main(void) {
  uid_t uid = geteuid();
  struct passwd *pw = getpwuid(uid);
  if (pw) {
    printf("%s\n", pw->pw_name);
    return 0;
  }
  perror("pwhoami");
  return 1;
}
