#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

typedef struct {
    char name[16];
    int client_fd;
} client;
client client_list[1000];
int number_clients = 0;
pthread_mutex_t counter_mutex = PTHREAD_MUTEX_INITIALIZER;

int get_index(int client);
int remove_client(int client);
int check_exist_username(char* username);
int check_username_format(char* username);
int process_join(int client, char *buf);
int process_msg(int client, char *buf);
int process_pmsg(int client, char *buf);
int process_op(int client);
int process_kick(int client);
int process_topic(int client);
int process_quit(int client);

void* thread_proc(void* arg);
int main() {
    struct sockaddr_in sa;
    sa.sin_family = AF_INET;
    sa.sin_port = htons(9000);
    sa.sin_addr.s_addr = INADDR_ANY;

    int server = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    if(server == -1) {
        return -1;
    }
    printf("Initializing server...\n");

    int ret = bind(server,(struct sockaddr*)&sa,sizeof(sa));
    if(ret == -1) {
        printf("Binding error.\n");
        return -1;
    }

    ret = listen(server, 100);
    if(ret == -1) {
        printf("Listen() error.\n");
        return -1;
    }
    printf("Listening...\n");

    while(1) {
        int clientfd = accept(server,NULL,NULL);
        if(clientfd == -1) continue;
        printf("New client connected.\n");
        pthread_t thread_id;
        pthread_create(&thread_id,NULL,thread_proc,(void*)&clientfd);
        pthread_detach(thread_id);
    }
}
void* thread_proc(void* arg) {
    int clientfd = *((int*) arg);
    char buf[2048] = "";
    while(1) {
        int len = recv(clientfd,buf,sizeof(buf),0);
        if(len <= 0) {
            remove_client(clientfd);
            break;
        }
        buf[len] = 0;
        printf("Client %d: %s\n",clientfd,buf);
        if (strncmp(buf, "JOIN ", 5) == 0)
            process_join(clientfd, buf);
        else if (strncmp(buf, "MSG ", 4) == 0)
            process_msg(clientfd, buf);
        else if (strncmp(buf, "PMSG ", 5) == 0)
            process_pmsg(clientfd, buf);
        else if (strncmp(buf, "OP ", 3) == 0)
            process_op(clientfd);
        else if (strncmp(buf, "KICK ", 5) == 0)
            process_kick(clientfd);
        else if (strncmp(buf, "TOPIC ", 6) == 0)
            process_topic(clientfd);
        else if (strncmp(buf, "QUIT",4) == 0)
            process_quit(clientfd);
        else {
            char *msg = "999 Unknown command\n";
            send(clientfd, msg, strlen(msg), 0);
        }
    }
    close(clientfd);
}


int get_index(int clientfd) {
    for(int i=0;i<number_clients;i++) {
        if(client_list[i].client_fd == clientfd) return i;
    }
    return -1;
}
int remove_client(int clientfd) {
    int client_id = get_index(clientfd);
    if(client_id != -1) {
        char msg[32] = "";
        sprintf(msg,"QUIT %s\n",client_list[client_id].name);
        printf("%s",msg);
        for(int j=0;j<number_clients;j++) {
            if(j != client_id) send(client_list[j].client_fd,msg,sizeof(msg),0);
        }

        for(int i=client_id+1;i<number_clients;i++) {
            client_list[i-1] = client_list[i];
        }
        number_clients--;
        return 0;
        
    } 
    return -1;
}
int check_exist_username(char* username) {
    for(int i=0;i<number_clients;i++) {
        if(strcmp(client_list[i].name,username) == 0) return 1;
    }
    return 0;
}
int check_username_format(char* username) {
    int l = strlen(username);
    for(int i=0;i<l;i++) {
        if (username[i] < '0' || username[i] > 'z' || (username[i] > '9' && username[i] < 'a')) return 0;
    }
    return 1;
}
int process_join(int clientfd, char *buf) {
    if(get_index(clientfd) == -1) {
        char username[16], tmp[32];
        int n = sscanf(buf,"JOIN %s %s",username,tmp);
        if(n == 1) {
            if(!check_username_format(username)) {
                char msg[] = "201 INVALID NICK NAME\n";
                send(clientfd,msg,sizeof(msg),0);
                return 0;
            }

            if(check_exist_username(username)) {
                char msg[] = "200 NICKNAME IN USE\n";
                send(clientfd,msg,sizeof(msg),0);
                return 0;
            }
            client newclient;
            newclient.client_fd = clientfd;
            strcpy(newclient.name,username);
            client_list[number_clients++] = newclient;
            send(clientfd,"100 OK\n",8,0);
            for(int i=0;i<number_clients-1;i++) {
                char msg[32] = "";
                sprintf(msg,"JOIN %s\n",username);
                send(client_list[i].client_fd,msg,sizeof(msg),0);
            }
        }
        return 1;
    }
    char msg[] = "999 UNKNOWN ERROR\n";
    send(clientfd,msg,sizeof(msg),0);
    return 0;
}
int process_msg(int clientfd, char *buf) {
    int client_id = get_index(clientfd);
    if(client_id == -1) {
        char msg[] = "999 UNKNOWN ERROR\n";
        send(clientfd,msg,sizeof(msg),0);
        return -1;
    }
    else {
        char client_msg[3000] = "";
        sprintf(client_msg,"MSG %s %s\n",client_list[client_id].name,buf+4);
        if(strlen(buf+4) <= 1) {
            char msg[] = "999 UNKNOWN ERROR\n";
            send(clientfd,msg,sizeof(msg),0);
            return -1;
        }
        for(int i=0;i<number_clients;i++) {
            if(i != client_id) send(client_list[i].client_fd,client_msg,sizeof(client_msg),0);
        }
        send(clientfd,"100 OK\n",8,0);
        return 0;
    }
}
int process_pmsg(int clientfd, char *buf) {

}
int process_op(int clientfd) {

}
int process_kick(int clientfd) {

}
int process_topic(int clientfd) {

}
int process_quit(int clientfd) {
    int ret = remove_client(clientfd);
    if(ret != -1) {
        send(clientfd,"100 OK\n",8,0);
        close(clientfd);
    }
    return 0;
}