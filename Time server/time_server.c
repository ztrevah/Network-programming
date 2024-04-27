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
#include <time.h>
struct datestruct{
    int year;
    int month;
    int date;
};
int checkLeapYear(int year) {
    if((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)) return 1;
    else return 0;
}
struct datestruct getCurrDate(int curr_timestamp) {
    struct datestruct tmp;
    int second_per_day = 3600 * 24;
    int daysfrom01011970 = curr_timestamp / second_per_day;

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
// Check client response, return 0 if wrong format, otherwise return format type
int checkClientResponse(char* response) {
    int l = strlen(response);
    if(l<=17) return 0;
    char command[10] = "";
    for(int i=0;i<9;i++) command[i] = response[i];
    if(strcmp(command,"GET_TIME ") != 0) return 0;

    int i = 9;
    char format[16] = "";
    while(response[i] != '\0') {
        format[i-9] = response[i];
        ++i;
        if((i-9) > 11) return 0;
    }   
    if(format[i-9-1] == '\n') format[i-9-1] = '\0';
    if(strcmp(format,"dd/mm/yyyy") == 0) return 1;
    else if(strcmp(format,"dd/mm/yy") == 0) return 2;
    else if(strcmp(format,"mm/dd/yyyy") == 0) return 3;
    else if(strcmp(format,"mm/dd/yy") == 0) return 4;
    return 0;
}
// unsigned int to char array
char* itoa(unsigned int n) {
    if(n == 0) return "0";
    char tmp[100]; int i=0;
    while(n>0) {
        tmp[i] = '0' + (n%10);
        n = n/10; i++;
    }
    char* res = malloc(sizeof(char) * (i+1));
    for(int j=0;j<i;j++) res[j] = tmp[i-1-j];
    res[i] = '\0';
    return res;
}
// datestruct to char array 
char* date_to_charArray(struct datestruct d,int format) {
    char* res = malloc(sizeof(char) * 16);
    // dd/mm/yyyy
    if(format == 1) {
        if(d.date < 10) strcat(res,"0");
        strcat(res,itoa(d.date));
        strcat(res,"/");

        if(d.month < 10) strcat(res,"0");
        strcat(res,itoa(d.month));
        strcat(res,"/");

        strcat(res,itoa(d.year));
        strcat(res,"\0");
    }
    // dd/mm/yy
    else if(format == 2) {
        if(d.date < 10) strcat(res,"0");
        strcat(res,itoa(d.date));
        strcat(res,"/");

        if(d.month < 10) strcat(res,"0");
        strcat(res,itoa(d.month));
        strcat(res,"/");
        
        char* tmp = itoa(d.year);
        strcat(res,tmp+2);
        strcat(res,"\0");
    }
    // mm/dd/yyyy
    else if(format == 3) {
        if(d.month < 10) strcat(res,"0");
        strcat(res,itoa(d.month));
        strcat(res,"/");

        if(d.date < 10) strcat(res,"0");
        strcat(res,itoa(d.date));
        strcat(res,"/");
        
        strcat(res,itoa(d.year));
        strcat(res,"\0");
    }
    // mm/dd/yy
    else if(format == 4) {
        if(d.month < 10) strcpy(res,"0");
        strcat(res,itoa(d.month));
        strcat(res,"/");

        if(d.date < 10) strcpy(res,"0");
        strcat(res,itoa(d.date));
        strcat(res,"/");
        
        char* tmp = itoa(d.year);
        strcat(res,tmp+2);
        strcat(res,"\0");
    }
    return res;
}
void signalHandler(int signo) {
    int stat;
    pid_t pid = wait(&stat);
    printf("Child %d terminated.\n", pid);
}

int main() {
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

                int check = checkClientResponse(buf);
                if(check == 0) {
                    char wrongFormatInform[] = "Server: Wrong format!\n";
                    send(client,wrongFormatInform,sizeof(wrongFormatInform),0);
                }
                else {
                    int curr_timestamp = time(NULL);
                    curr_timestamp += 7*3600; // convert to GMT +7
                    struct datestruct curr_date = getCurrDate(curr_timestamp);
                    char *msg = date_to_charArray(curr_date,check);
                    strcat(msg,"\n");
                    printf("%s",msg);
                    send(client,msg,16,0);
                }
                close(client);
            }
        }
    }
    wait(NULL);
}