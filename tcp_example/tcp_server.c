#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <time.h>

// Truyền tham số dòng lệnh gồm cổng, file chứa câu chào của server và file chứa tin nhắn từ client
int main(int argc,char* argv[]){
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
    sa.sin_port = htons(atoi(argv[1]));
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

    while(1) {
        printf("Listening...\n");
        int s1 = accept(server,NULL,NULL);
        if(s1 != -1) {
            printf("Connection accepted, awaiting message...\n");
            char buf[256];
            int ret2 = recv(s1,buf,sizeof(buf),0);
            if(ret2 > 0) {
                printf("Message from client received\n");

                FILE* hellobackFile = fopen(argv[2],"r");
                FILE* clientmsgFile = fopen(argv[3],"a");
                
                fprintf(clientmsgFile,"%s",buf);

                char hellomsg[1024] = "";
                char tmp[256] = "";
                while(fgets(tmp,sizeof(tmp),hellobackFile)) {
                    strcat(hellomsg,tmp);
                }
                int ret3 = send(s1,hellomsg,sizeof(hellomsg),0);
                if(ret3 == -1) {
                    printf("Cannot send message back to client\n");
                    break;
                }
                else printf("Sent message back to client\n",hellomsg);

                fclose(hellobackFile);
                fclose(clientmsgFile);
            }

            else printf("Cannot receive message\n");

        }
        shutdown(s1,SHUT_RDWR);
    }
    return 0;


}