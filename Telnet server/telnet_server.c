#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <sys/select.h>

struct client_info {
    char username[128];
    char password[128];
    int fd;
};
int number_clients = 0;
int checkAccount(char* username,char* password,FILE* accountFile) {
    char tmp[300] = "";
    while(fgets(tmp,sizeof(tmp),accountFile)) {
        char cmp_username[128] = "";
        char cmp_password[128] = "";
        sscanf(tmp,"%[^' ']%*c%[^'\n']",cmp_username,cmp_password);
        if(strcmp(username,cmp_username) == 0 && strcmp(password,cmp_password) == 0) return 1;
    }
    return 0;
}
int main() {
    FILE* accountFile = fopen("account.txt","r");

    char requestinfomsg[] = "What is your username & password?\n";
    char requestagain[] = "Invalid username or password.\n";
    char acceptmsg[] = "Now you can send your command.\n";

    struct sockaddr_in sa;
    sa.sin_family = AF_INET;
    sa.sin_port = htons(9000);
    sa.sin_addr.s_addr = INADDR_ANY;
    
    int server = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    int ret1 = bind(server,(struct sockaddr*) &sa,sizeof(sa));
    if(ret1 == -1) {
        printf("Binding error.\n");
        fclose(accountFile);
        return 1;
    }

    int listener = listen(server,100);
    if(listener == -1) {
        printf("Cannot create queue for client connection.\n");
        fclose(accountFile);
        return 1;
    }

    printf("Initialized!\n");

    fd_set fdread;
    struct client_info clients[16];
    int allowed[16];
    struct timeval tv;

    while(1) {
        FD_ZERO(&fdread);
        FD_SET(server,&fdread);
        int maxdp = server + 1;
        for(int i=0;i<number_clients;i++) {
            FD_SET(clients[i].fd,&fdread);

            if(clients[i].fd + 1 > maxdp) maxdp = clients[i].fd + 1;
        }

        // Set thời gian timeout
        tv.tv_sec = 5;
        tv.tv_usec = 0;

        int ret = select(maxdp,&fdread,NULL,NULL,&tv);
        if(ret < 0) {
            printf("select() failed.\n");
            fclose(accountFile);
            return 1;
        }
        if (ret == 0) {
            printf("Timed out.\n"); 
            continue;
        }

        if(FD_ISSET(server,&fdread)) {
            int client = accept(server,NULL,NULL);
            clients[number_clients].fd = client;
            strcpy(clients[number_clients].username,"");
            strcpy(clients[number_clients].password,"");
            allowed[number_clients] = 0;
            number_clients++;
            send(client,requestinfomsg,sizeof(requestinfomsg),0);
        }

        char buf[1024] = ""; // Xâu ký tự lưu phản hồi của client
        for(int i=0;i<number_clients;i++) {
            if(FD_ISSET(clients[i].fd,&fdread)) {
                int ret2 = recv(clients[i].fd,buf,sizeof(buf),0);
                if(ret2 <= 0) {
                    printf("Client %s disconnected\n", clients[i].username);
                    for(int j=number_clients-2;j>=i;j--) {
                        clients[j] = clients[j+1];
                        allowed[j] = allowed[j+1];
                    }
                    number_clients--; i--;
                    continue;
                }
                // Hỏi username và password của client
                if(allowed[i] == 0) {
                    char tmp_username[128] = "";
                    char tmp_password[128]= "";
                    int n = sscanf(buf,"%[^' ']%*c%[^'\n']",tmp_username,tmp_password);
                    printf("%s %s\n",tmp_username,tmp_password);
                    if(n != 2) send(clients[i].fd,requestagain,sizeof(requestagain),0);
                    else {
                        rewind(accountFile);
                        int check = checkAccount(tmp_username,tmp_password,accountFile);
                        if(check == 1) {
                            allowed[i] = 1;
                            strcpy(clients[i].username,tmp_username);
                            strcpy(clients[i].password,tmp_password);
                            printf("Client %s connected\n", clients[i].username);
                            send(clients[i].fd,acceptmsg,sizeof(acceptmsg),0);
                        }
                        else send(clients[i].fd,requestagain,sizeof(requestagain),0);
                    }
                    
                }
                // Nếu đã xác định được client thì cho phép gửi tin nhắn 
                else {
                    char buf1[1024] = "";
                    strcat(buf1,clients[i].username);
                    strcat(buf1,": ");
                    strcat(buf1,buf);
                    printf("%s\n",buf1);

                    char command[1024] = "";
                    sscanf(buf,"%[^'\n]",command);
                    strcat(command," > out.txt");
                    int ret3 = system(command);

                    FILE* retFile = fopen("out.txt","r");
                    char responseToClientCommand[1024] = "Server: \n";
                    if(ret3 == 0) {
                        char tmp[1024] = "";
                        while(fgets(tmp,sizeof(tmp),retFile)) {
                            strcat(responseToClientCommand,tmp);
                        }
                    }
                    else strcat(responseToClientCommand,"Your command can not be executed for some reasons.\n");
                    strcat(responseToClientCommand,"\n");
                    send(clients[i].fd,responseToClientCommand,sizeof(responseToClientCommand),0);
                    fclose(retFile);
                }
            }
        }
    }
    fclose(accountFile);
}