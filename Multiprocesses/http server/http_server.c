#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <sys/select.h>
#include <signal.h>
#include <sys/wait.h>

void signalHandler(int signo) {
    int stat;
    pid_t pid = wait(&stat);
    printf("Child %d terminated.\n", pid);
}

int main() {
    char msg[] = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<html><body><h1>Xin chao cac ban</h1></body></html>";

    signal(SIGCHLD, signalHandler);

    struct sockaddr_in sa;
    sa.sin_family = AF_INET;
    sa.sin_port = htons(9000);
    sa.sin_addr.s_addr = INADDR_ANY;
    
    int server = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);

    int ret1 = bind(server,(struct sockaddr*)&sa,sizeof(sa));
    if(ret1 == -1) {
        printf("Binding error.\n");
        return 1;
    }
    printf("Initialized...\n");

    ret1 = listen(server,100);
    if(ret1 == -1) {
        printf("Fail to create queue for clients.\n");
        return 1;
    }
    printf("Listening...\n");

    int num_processes = 10;
    for(int i=0;i<num_processes;i++) {
        if(fork() == 0) {
            while(1) {
                int client = accept(server,NULL,NULL);
                printf("New client connected: %d\n", client);

                char buf[2048] = "";
                int ret2 = recv(client,buf,sizeof(buf),0);
                if(ret2 <= 0) {
                    close(client);
                    continue;
                }
                printf("Client %d: %s\n",client,buf);

                send(client,msg,sizeof(msg),0);

                close(client);
            }
        }
    }
    wait(NULL);
}