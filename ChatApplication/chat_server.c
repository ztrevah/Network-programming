#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <sys/select.h>
#include <time.h>

struct client_info {
    char id[128];
    char name[128];
    int fd;
};
struct datetime{
    int year;
    int month;
    int date;
    int hour;
    int minute;
    int second;
};
int checkLeapYear(int year) {
    if((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)) return 1;
    else return 0;
}
struct datetime curr_datetime(int curr_timestamp) {
    struct datetime tmp;
    int second_per_day = 3600 * 24;
    int daysfrom01011970 = curr_timestamp / second_per_day;
    int secondsfrom0h00 = curr_timestamp % second_per_day;
    tmp.hour = secondsfrom0h00 / 3600;
    tmp.minute = (secondsfrom0h00 - tmp.hour * 3600) / 60;
    tmp.second = secondsfrom0h00 - tmp.hour * 3600 - tmp.minute * 60;

    tmp.year = 1970;
    tmp.month = 1;
    tmp.date = 1;
    int dayscount = daysfrom01011970;
    while(dayscount >= 0) {
        if(checkLeapYear(tmp.year)) dayscount -= 366;
        else dayscount -= 365;
        tmp.year ++;
    }
    tmp.year --;
    if(checkLeapYear(tmp.year)) dayscount += 366;
    else dayscount += 365;

    int days_of_month[] = {31,28,31,30,31,30,31,31,30,31,30,31};
    if(checkLeapYear(tmp.year)) days_of_month[1] = 29;
    while(dayscount > days_of_month[tmp.month - 1]) {
        dayscount -= days_of_month[tmp.month - 1];
        tmp.month ++;
    }
    tmp.date += dayscount;
    return tmp;
}
int checkClientResponse (char *response) {
    int l = strlen(response);
    int count = 0;
    int tmp;
    for(int i=0;i<l;i++) {
        if(response[i] == ':') {
            count ++;
            tmp = i;
        }
    }
    if(count != 1 || tmp + 2 > l - 1) return -1;
    for(int i=0;i<tmp;i++) {
        if(response[i] == ' ') return -1;
    }
    if(response[tmp+1] != ' ') return -1;
    for(int i=tmp+2;i<l;i++) {
        if(response[i] == ' ') return -1;
    }
    return tmp;
}
int number_clients = 0;
int main() {
    char requestinfomsg[] = "What is your id: name?\n";
    char requestagain[] = "Send your id: name correctly.\n";
    char acceptmsg[] = "Now you can send your messsage to other clients.\n";

    printf("Initializing server at port 9000...\n");
    struct sockaddr_in sa;
    sa.sin_family = AF_INET;
    sa.sin_port = htons(9000);
    sa.sin_addr.s_addr = INADDR_ANY;

    int server = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    int ret1 = bind(server,(struct sockaddr*) &sa,sizeof(sa));
    if(ret1 == -1) {
        printf("Binding error.\n");
        return 1;
    }

    int listener = listen(server,100);
    if(listener == -1) {
        printf("Cannot create queue for client connection.\n");
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
            return 1;
        }
        if (ret == 0) {
            printf("Timed out.\n"); 
            continue;
        }

        if(FD_ISSET(server,&fdread)) {
            int client = accept(server,NULL,NULL);
            clients[number_clients].fd = client;
            strcpy(clients[number_clients].id,"");
            strcpy(clients[number_clients].name,"");
            allowed[number_clients] = 0;
            number_clients++;
            send(client,requestinfomsg,sizeof(requestinfomsg),0);
        }

        char buf[1024] = "";
        for(int i=0;i<number_clients;i++) {
            if(FD_ISSET(clients[i].fd,&fdread)) {
                int ret2 = recv(clients[i].fd,buf,sizeof(buf),0);
                if(ret2 <= 0) {
                    printf("Client %s disconnected\n", clients[i].id);
                    for(int j=number_clients-2;j>=i;j--) {
                        clients[j] = clients[j+1];
                        allowed[j] = allowed[j+1];
                    }
                    number_clients--; i--;
                    continue;
                }
                // Hỏi tên client
                if(allowed[i] == 0) {
                    int pos = checkClientResponse(buf);
                    if(pos != -1) {
                        allowed[i] = 1;
                        for(int j=0;j<pos;j++) clients[i].id[j] = buf[j];
                        // for(int j=pos+2;j<strlen(buf);j++) clients[i].name[j-pos-2] = buf[j];
                        printf("Client %s connected\n", clients[i].id);
                        send(clients[i].fd,acceptmsg,sizeof(acceptmsg),0);
                    }
                    else send(clients[i].fd,requestagain,sizeof(requestagain),0);
                }
                // Nếu đã xác định được client thì cho phép gửi tin nhắn 
                else {
                    char buf1[1024] = "";
                    int t = time(NULL);
                    t += 7 * 3600; // GMT +7
                    struct datetime dt = curr_datetime(t);
                    sprintf(buf1,"%d-%d-%d %d:%d:%d ",dt.year,dt.month,dt.date,dt.hour,dt.minute,dt.second);
                    strcat(buf1,clients[i].id);
                    strcat(buf1,": ");
                    strcat(buf1,buf);
                    for(int j=0;j<number_clients;j++) if(j != i && allowed[j] != 0) {
                        send(clients[j].fd,buf1,sizeof(buf1),0);
                    }
                    printf("%s\n",buf1);
                }
            }
        }
    }
}