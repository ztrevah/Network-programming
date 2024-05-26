#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

// struct requestHeader{
//     char* name;
//     char* value;
//     struct requestHeader* next;
// };
// typedef struct requestHeader requestHeader;

// Struct for http request
typedef struct {
    char* method;
    char* url;
    // char* httpversion;
    // requestHeader* header;
    // char* body; 
} httpRequest;

// Convert request string to struct httpRequest
int a_to_request(char* reqstr,httpRequest* req) {
    char* tmp_reqstr = reqstr;
    int reqlen = strlen(tmp_reqstr);

    char* tmp1 = strchr(tmp_reqstr,' ');
    if(tmp1 == NULL) return -1;
    int method_len = tmp1 - tmp_reqstr;
    req->method = calloc(method_len + 1,sizeof(char));
    strncpy(req->method,tmp_reqstr,method_len);

    tmp_reqstr += (method_len + 1);

    tmp1 = strchr(tmp_reqstr,' ');
    if(tmp1 == NULL) return -1;
    int url_len = tmp1 - tmp_reqstr;
    req->url = calloc(url_len + 1,sizeof(char));
    strncpy(req->url,tmp_reqstr,url_len);

    // tmp_reqstr += (url_len + 1);

    // tmp1 = strstr(tmp_reqstr,"\r\n");
    // int version_len = tmp1 - tmp_reqstr;
    // req->httpversion = calloc(version_len + 1,sizeof(char));
    // strncpy(req->httpversion,tmp_reqstr,version_len);

    // tmp_reqstr += (version_len + 2);
    // if(tmp_reqstr == NULL) return -1;

    // requestHeader* ptr = req->header;
    // while(tmp_reqstr != NULL) {
    //     tmp1 = strstr(tmp_reqstr,"\r\n");
    //     int header_len = tmp1 - tmp_reqstr;
    //     if(header_len == 0) break;
    //     char* header = calloc(header_len + 1,sizeof(char));
    //     strncpy(header,tmp_reqstr,header_len);

    //     char* tmp2 = strchr(header,':');
    //     int headername_len = tmp2 - header;
    //     ptr->name = calloc(headername_len + 1,sizeof(char));
    //     strncpy(ptr->name,header,headername_len);

    //     header += headername_len + 2;
    //     ptr->value = header;

    //     requestHeader* nextheader;
    //     ptr->next = nextheader;
    //     ptr = ptr->next;
    // }
    return 1;
}
int getURLParam(httpRequest req,double* a,double* b,char* cmd) {
    int ret = sscanf(req.url,"/calc?a=%lf&b=%lf&cmd=%s",a,b,cmd);
    if(ret < 3) {
        return -1;
    }
    return 1;
}
void* thread_proc(void* arg);
int main() {
    struct sockaddr_in sa;
    sa.sin_family = AF_INET;
    sa.sin_port = htons(9000);
    sa.sin_addr.s_addr = INADDR_ANY;

    int server = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    int ret = bind(server,(struct sockaddr*)&sa,sizeof(sa));
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
    printf("Listening...\n");

    while(1) {
        int client = accept(server,NULL,NULL);
        if(client == -1) continue;

        printf("New client connected: %d\n",client);
        // Tạo luồng xử lý riêng cho mỗi client
        pthread_t thread_id;
        pthread_create(&thread_id,NULL,thread_proc,(void*) &client);
        pthread_detach(thread_id);
    }
    close(server);
}
void* thread_proc(void* arg) {
    int client = *(int*) arg;
    char buf[1024] = ""; // Buffer for client request
    char response[1024] = "";
    int len = recv(client,buf,sizeof(buf),0);
    if(len <= 0) {
        printf("Client %d disconnected.\n",client);
        close(client);
        pthread_exit(NULL);
    }

    buf[len] = 0;
    printf("Request from client %d:\n%s",client,buf);
    // Analyze client request
    httpRequest clientRequest;
    int ret1 = a_to_request(buf,&clientRequest);
    if(ret1 == -1) {
        strcpy(response,"HTTP/1.1 400 Bad Request\r\nContent-Type: text/html\r\n\r\n<h1>400 Bad Request</h1>");
        send(client,response,sizeof(response),0);
        close(client);
        pthread_exit(NULL);
    }

    // Check request method
    if(strcmp(clientRequest.method,"GET") == 0 || strcmp(clientRequest.method,"POST") == 0) {
        // Check params
        double a,b; char* cmd = calloc(16,sizeof(char));
        int check = getURLParam(clientRequest,&a,&b,cmd);
        printf("%lf %lf %s\n",a,b,cmd);
        char invalidmsg[] = "Invalid parameter(s).";
        
        if(check == -1) {
            sprintf(response,"HTTP/1.1 400 Bad Request\r\nContent-Type: text/html\r\n\r\n<h1>%s</h1>",invalidmsg);
            send(client,response,sizeof(response),0);
        }
        else if(strcmp(cmd,"add") == 0) {
            sprintf(response,"HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<h1>Result: %lf</h1>",a + b);
            send(client,response,sizeof(response),0);
        }
        else if(strcmp(cmd,"sub") == 0) {
            sprintf(response,"HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<h1>Result: %lf</h1>",a - b);
            send(client,response,sizeof(response),0);
        }
        else if(strcmp(cmd,"mul") == 0) {
            sprintf(response,"HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<h1>Result: %lf</h1>",a * b);
            send(client,response,sizeof(response),0);
        }
        else if(strcmp(cmd,"div") == 0) {
            sprintf(response,"HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<h1>Result: %lf</h1>",a / b);
            send(client,response,sizeof(response),0);
        }
        else {
            sprintf(response,"HTTP/1.1 400 Bad Request\r\nContent-Type: text/html\r\n\r\n<h1>%s</h1>",invalidmsg);
            send(client,response,sizeof(response),0);
        }    
    }

    close(client);
    pthread_exit(NULL);
}