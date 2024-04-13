#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>

int main(int argc,char* argv[]){

    struct sockaddr_in sa;
    sa.sin_family = AF_INET;
    sa.sin_port = htons(atoi(argv[2]));
    inet_aton(argv[1],&(sa.sin_addr));
    
    while(1) {
        printf("Message you want to send to server: ");
        char msg[256];
        fgets(msg,sizeof(msg),stdin);
        printf("\n");

        int client = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
        if(client == -1) {
            printf("Cannot create client socket\n");
            return 1;
        }
        printf("Client socket initialized... ");

        printf("Connecting to server... ");
        int ret1 = connect(client,(struct sockaddr*) &sa,sizeof(sa));
        if(ret1 != 0) {
            printf("Problem with connecting to server\n");
            return 1;
        }
        printf("Socket connected to server created... ");

        int ret2 = send(client,msg,sizeof(msg),0);
        if(ret2 == -1) {
            printf("Error while sending message from client to server\n");
        }
        printf("Message to server sent!\n");

        char buf[1024];
        int ret3 = recv(client,buf,sizeof(buf),0);
        if(strlen(buf) == 0) break;
        if(ret3 != -1) {
            printf("Server response: \n%s",buf);
        }
        
        shutdown(client,SHUT_RDWR);
    }


    
}