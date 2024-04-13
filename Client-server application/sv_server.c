#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <time.h>
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
int main(int argc,char* argv[]){
    int server = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    struct sockaddr_in sa;
    sa.sin_family = AF_INET;
    sa.sin_port = htons(atoi(argv[1]));
    sa.sin_addr.s_addr = htonl(INADDR_ANY);

    printf("Binding... \n");
    int ret1 = bind(server,(struct sockaddr*) &sa,sizeof(sa));
    if(ret1 == -1) {
        printf("Problem with binding server occured\n");
        return 1;
    }
    
    printf("Creating connection queue... \n");
    int ret2 = listen(server,10);
    if(ret2 == -1) {
        printf("Cannot create queue on server socket\n");
        return 1;
    }
    
    while(1) {
        printf("Listening...\n");
        int s1 = accept(server,NULL,NULL);
        if(s1 == -1) {
            printf("Error while connecting to client\n");
            continue;
        }
        printf("Connected. Waiting data from client\n");

        int curr_timestamp = time(NULL);
        curr_timestamp += 7 * 3600; // chuyển về múi giờ GMT+7
        struct datetime sent_time = curr_datetime(curr_timestamp);

        // Xác định địa chỉ IP của client
        struct sockaddr client_sa;
        socklen_t client_sa_len = sizeof(client_sa);
        getpeername(s1,&client_sa,&client_sa_len);
        struct sockaddr_in client_sa_ipv4;
        memcpy(&client_sa_ipv4,&client_sa,client_sa_len);

        char *client_ipaddr = inet_ntoa(client_sa_ipv4.sin_addr); // Xâu ký tự địa chỉ IPv4 của client

        FILE* logFile = fopen(argv[2],"a");
        char bufMSSV[10],bufHoTen[30],bufNgaySinh[12]; double diem;
        int ret3 = recv(s1,bufMSSV,sizeof(bufMSSV),0);
        if(ret3 == -1) {
            printf("Error while receiving data from client.\n");
            continue;
        }
        ret3 = recv(s1,bufHoTen,sizeof(bufHoTen),0);
        if(ret3 == -1) {
            printf("Error while receiving data from client.\n");
            continue;
        }
        ret3 = recv(s1,bufNgaySinh,sizeof(bufNgaySinh),0);
        if(ret3 == -1) {
            printf("Error while receiving data from client.\n");
            continue;
        }
        ret3 = recv(s1,&diem,sizeof(diem),0);
        if(ret3 == -1) {
            printf("Error while receiving data from client.\n");
            continue;
        }

        printf("Data received!\n");
        fprintf(logFile,"%s %d-",client_ipaddr,sent_time.year);

        if(sent_time.month < 10) fprintf(logFile,"0%d-",sent_time.month);
        else fprintf(logFile,"%d-",sent_time.month);

        if(sent_time.date < 10) fprintf(logFile,"0%d ",sent_time.date);
        else fprintf(logFile,"%d ",sent_time.date);

        if(sent_time.hour < 10) fprintf(logFile,"0%d:",sent_time.hour);
        else fprintf(logFile,"%d:",sent_time.hour);

        if(sent_time.minute < 10) fprintf(logFile,"0%d:",sent_time.minute);
        else fprintf(logFile,"%d:",sent_time.minute);

        if(sent_time.second < 10) fprintf(logFile,"0%d ",sent_time.second);
        else fprintf(logFile,"%d ",sent_time.second);

        fprintf(logFile,"%s %s %s %.2lf\n",bufMSSV,bufHoTen,bufNgaySinh,diem);
        fclose(logFile);
        shutdown(s1,SHUT_RDWR);
    }
}