#include <stdio.h>
#include <time.h>

int main(void) {
    time_t t=time(NULL);struct tm *tm_info=localtime(&t);char buf[64];
    strftime(buf,sizeof(buf),"%a %d %b %Y %H:%M:%S %Z",tm_info);
    printf("%s\n",buf);return 0;
}
