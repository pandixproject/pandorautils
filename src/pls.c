#include <stdio.h>
#include <dirent.h>

int main(int argc,char *argv[]){
    const char *path=argc>1?argv[1]:".";DIR *d = opendir(path);
    if(!d){
        perror("pls");return 1;
    }struct dirent *entry;
    while((entry=readdir(d))!=NULL){
        if(entry->d_name[0]=='.')continue;printf("%s\n", entry->d_name);
    }closedir(d);return 0;
}
