#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>

int main() {
    struct sockaddr_in sa;
    sa.sin_family = AF_INET;
    sa.sin_port = htons(9000);
    inet_aton("127.0.0.1",&(sa.sin_addr));

    const char *client_id = "cl1";
    const char *client_name = "client1";

    int client = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    int ret = connect(client,(struct sockaddr*) &sa,sizeof(sa));

    fd_set fdread;
    FD_ZERO(&fdread);
    char buf[1024];
    while (1) {
        FD_SET(STDIN_FILENO, &fdread);
        FD_SET(client, &fdread);
        int maxdp = client + 1; // STDIN_FILENO is 0
        select(client + 1, &fdread, NULL, NULL, NULL);
        if (FD_ISSET(STDIN_FILENO, &fdread)) {
            fgets(buf, sizeof(buf), stdin);
            send(client, buf, strlen(buf), 0);
        }
        if (FD_ISSET(client, &fdread)) {
            ret = recv(client, buf, sizeof(buf), 0);
            buf[ret] = 0;
            printf("Received: %s\n", buf);
        }
    }
    
}