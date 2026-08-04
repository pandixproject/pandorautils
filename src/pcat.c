#include <stdio.h>

int main(int argc,char *argv[]){
    char buf[4096];size_t n;
    if(argc<2){
        while((n=fread(buf,1,sizeof(buf),stdin))>0){
            fwrite(buf, 1, n, stdout);
        }return 0;
    }int status = 0;
    for(int i=1;i<argc;i++){
        FILE *f = fopen(argv[i],"rb");
        if(!f){
            perror(argv[i]);
            status=1;
            continue;
        }while((n=fread(buf,1,sizeof(buf),f))>0){
            fwrite(buf,1,n,stdout);
        }fclose(f);
    }return status;
}
