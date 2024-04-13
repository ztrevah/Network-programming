#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
char* removeEndLine(char a[]) {
    int len = strlen(a);
    a[len-1] = '\0';
    return a;
}

int main(int argc,char* argv[]){

    struct sockaddr_in sa;
    sa.sin_family = AF_INET;
    sa.sin_port = htons(atoi(argv[2]));
    inet_aton(argv[1],&(sa.sin_addr));

    printf("Prepare data to send to server:\n");

    char MSSV[10],HoTen[30],NgaySinh[12];
    double DiemTB;

    printf("MSSV: ");
    fgets(MSSV,sizeof(MSSV),stdin);
    printf("Ho va ten: ");
    fgets(HoTen,sizeof(HoTen),stdin);
    printf("Ngay sinh: ");
    fgets(NgaySinh,sizeof(NgaySinh),stdin);
    printf("Diem trung binh: ");
    scanf("%lf",&DiemTB);

    removeEndLine(MSSV);
    removeEndLine(HoTen);
    removeEndLine(NgaySinh);

    printf("Starting... Creating socket... \n");
    int client = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    if(client == -1) {
        printf("Error creating socket.\n");
        return 1;
    }
    
    int ret = connect(client,(struct sockaddr*) &sa,sizeof(sa));
    if(ret == -1) {
        printf("Cannot connect to server.\n");
        return 1;
    }
    printf("Connected to server. Sending data...\n");

    int retMSSV = send(client,MSSV,sizeof(MSSV),0);
    if(retMSSV == -1) {
        printf("Cannot send MSSV to server\n");
    }

    int retHoTen = send(client,HoTen,sizeof(HoTen),0);
    if(retHoTen == -1) {
        printf("Cannot send HoTen to server\n");
    }

    int retNgaySinh = send(client,NgaySinh,sizeof(NgaySinh),0);
    if(retNgaySinh == -1) {
        printf("Cannot send NgaySinh to server\n");
    }

    int retDiemTB = send(client,&DiemTB,sizeof(DiemTB),0);
    if(retDiemTB == -1) {
        printf("Cannot send DiemTB to server\n");
    }

    printf("Sending data to server completed\n");
    shutdown(client,SHUT_RDWR);

}