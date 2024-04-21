#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>

int main() {
    char buf[256];
    getcwd(buf,sizeof(buf));

    struct sockaddr_in sa;
    sa.sin_family = AF_INET;
    sa.sin_port = htons(9000);
    inet_aton("127.0.0.1",&(sa.sin_addr));

    DIR* FD = opendir(buf);
    struct dirent* file_dir;
    FILE* file;
    struct stat st;
    while(file_dir = readdir(FD)) {
        if (file_dir->d_type != DT_REG) continue;
        stat(file_dir->d_name,&st);
        int file_size = st.st_size;
        char buf[256] = "";
        strcat(buf,file_dir->d_name);
        strcat(buf," - ");
        char tmp[100] = ""; int i = 0;
        while(file_size > 0) {
            tmp[i] = (file_size % 10) + '0';
            file_size /= 10;
            i++;
        }
        if(i == 0) strcat(buf,"0");
        for(int j=i-1;j>=0;j--) strncat(buf,&(tmp[j]),1);
        strcat(buf," bytes");

        int client = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
        int ret = connect(client,(struct sockaddr*) &sa,sizeof(sa));
        if(ret == -1) {
            printf("Cannot connect to server.\n");
        }
        
        int ret1 = send(client,buf,sizeof(buf),0);
        if(ret1 == -1) {
            printf("Unable to send data to server.\n");
            return 1;
        }
    }
    printf("Sending completed!\n");
}