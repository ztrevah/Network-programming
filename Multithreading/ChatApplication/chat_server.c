#include <stdio.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
// Cấu trúc của kết nối giữa client và partner của client đó
typedef struct {
    int client_fd;
    int partner_fd;
} client_connection;
void* thread_proc(void *arg);
int main() {
    // Khởi tạo server
    struct sockaddr_in sa;
    sa.sin_family = AF_INET;
    sa.sin_port = htons(9000);
    sa.sin_addr.s_addr = INADDR_ANY;
    
    int server = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    int ret = bind(server,(struct sockaddr*) &sa,sizeof(sa));
    if(ret == -1) {
        printf("Binding error.\n");
        return 1;
    }
    printf("Server initialized!\n");

    ret = listen(server,100);
    if(ret == -1) {
        printf("Cannot create queue for connection requests.\n");
        return 1;
    }

    
    int number_clients = 0;
    client_connection* clientQueue[1000]; // Hàng đợi các client chưa được bắt cặp
    while(1) {
        int client = accept(server,NULL,NULL);
        if(client == -1) continue;

        printf("New client connected: %d\n",client);
        client_connection* cc = (client_connection*) malloc(sizeof(client_connection));
        cc->client_fd = client;
        cc->partner_fd = -1;
        // Tạo luồng xử lý riêng cho mỗi client
        pthread_t thread_id;
        pthread_create(&thread_id,NULL,thread_proc,(void*) cc);
        pthread_detach(thread_id);

        clientQueue[number_clients] = cc;
        number_clients++;

        if(number_clients >= 2) {
            clientQueue[0]->partner_fd = clientQueue[1]->client_fd;
            clientQueue[1]->partner_fd = clientQueue[0]->client_fd;
            for(int i=0;i<number_clients-2;i++) {
                clientQueue[i] = clientQueue[i+2];
            }
            number_clients -= 2;
        }
    }
    return 0;
}
void* thread_proc(void* arg) {
    client_connection* cc = (client_connection*) arg;
    int client = cc->client_fd;
    
    char queueingMessage[] = "Server: Queueing...\n";
    send(client,queueingMessage,sizeof(queueingMessage),0);

    // Chờ đến khi được bắt cặp
    while(cc->partner_fd == -1) {}

    int partner = cc->partner_fd;
    printf("Pairing client %d and %d.\n",client,partner);
    char foundPartnerMessage[] = "Server has found a partner for you. You can now chat with him/her.\n";
    send(client,foundPartnerMessage,sizeof(foundPartnerMessage),0);
    
    while(1) {
        char buf[1024] = ""; // Xâu ký tự lưu lại tin nhắn client gửi
        int len = recv(client,buf,sizeof(buf),0);
        // Nếu client này đã ngắt kết nối thì thông báo lại cho partner của client đó và đóng socket của cả 2
        if(len <= 0) {
            char msg1[] = "Your partner has been disconnected. You will be disconnected too.\n";
            send(partner,msg1,sizeof(msg1),0);
            printf("Client %d disconnected.\n",client);
            close(partner);
            break;
        }
        
        char buf1[2048] = ""; // Tin nhắn được định dạng lại để gửi lại cho partner của client
        sprintf(buf1,"Client %d: %s\n",client,buf);
        printf("%s",buf1); 
        int ret = send(partner,buf1,sizeof(buf),0);
        // Nếu partner của client này đã ngắt kết nối thì thông báo lại cho client
        if(ret == -1) {
            char msg1[] = "Your partner has been disconnected. You will be disconnected too.\n";
            send(client,msg1,sizeof(msg1),0);
            break;
        }
    }
    close(client);
}