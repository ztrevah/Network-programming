#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

void* thread_proc(void *arg);
int main() {
    struct sockaddr_in sa;
    sa.sin_family = AF_INET;
    sa.sin_port = htons(9000);
    inet_aton("127.0.0.1",&(sa.sin_addr));

    int client = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);

    int ret = connect(client,(struct sockaddr*)&sa,sizeof(sa));
    if(ret == -1) {
        printf("Cannot connect to server.\n");
        return 1;
    }

    pthread_t thread_id;
    pthread_create(&thread_id, NULL, thread_proc, (void *)&client);
    char buf[256];
    while (1) {
        fgets(buf, sizeof(buf), stdin);
        int ret1 = send(client, buf, strlen(buf), 0);
        if (ret1 == -1) {
            break;
        }
    }
    close(client);
    return 0;

}
void* thread_proc(void *arg) {
    int client = *(int *)arg;
    char buf[256];
    while (1) {
        int len = recv(client, buf, sizeof(buf), 0);
        if (len <= 0) break;
        buf[len] = 0;
        printf("%s\n", buf);
    }
    close(client);
    return 0;
}