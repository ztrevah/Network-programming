#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>

int main() {
    // Tạo socket cho server
    int server = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    if(server == -1) {
        printf("Cannot create server socket\n");
        return 1;
    }
    printf("Server socket created\n");

    // Binding socket
    struct sockaddr_in sa;
    sa.sin_family = AF_INET;
    sa.sin_port = htons(9000);
    sa.sin_addr.s_addr = htonl(INADDR_ANY);
    int ret = bind(server,(struct sockaddr* )&sa,sizeof(sa));
    if(ret != 0) {
        printf("Problem occured with binding\n");
        return 1;
    }
    printf("Successful binding\n");

    int ret1 = listen(server,10);
    if(ret1 != 0) {
        printf("Problem occured when creating listening queue\n");
        return 1;
    }

    printf("Waiting data from client...\n");

    while(1) {
        int s1 = accept(server,NULL,NULL);
        if(s1 != -1) {
            char msg[256] = "";
            int ret2 = recv(s1,msg,sizeof(msg),0);
            if(ret2 == -1) {
                printf("Cannot receive data from client.\n");
            }
            printf("%s\n",msg);
        }
    }
}