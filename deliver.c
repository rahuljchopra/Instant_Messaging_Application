#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>
#include <math.h>

#define STDIN 0
#define LOGIN 1
#define LO_ACK 2
#define LO_NAK 3
#define EXIT 4
#define JOIN 5
#define JN_ACK 6
#define JN_NAK 7
#define LEAVE_SESS 8
#define NEW_SESS 9
#define NS_ACK 10
#define MESSAGE 11
#define QUERY 12
#define QU_ACK 13
#define MESSAGE_ONE 14

struct message {
    unsigned int type;
    unsigned int size;
    unsigned char source[50];
    unsigned char data[500];
} msg;

int main(void) {
    
    int status, sockfd;
    struct addrinfo hints, *serverinfo;
    char line[200];
    char send_msg[1000];
    struct timeval tv;
    fd_set readfds;
    char recv_buf[500];
    
    tv.tv_sec = 0;
    tv.tv_usec = 0;

    FD_ZERO(&readfds);
    FD_SET(STDIN, &readfds);
    
    while(1) {
    
        printf("Enter your login and server connection details\n");
        scanf("%[^\n]%*c", line);
        char* command = strtok(line, " ");
        if(strcmp(command, "/quit") == 0) {return 0;}
        msg.type = LOGIN;
        char* client_id = strtok(NULL, " ");
        strcpy(msg.source, client_id);
        char* data = strtok(NULL, " ");
        msg.size = strlen(data);
        strcpy(msg.data, data);
        char* server_ip = strtok(NULL, " ");
        char* server_port = strtok(NULL, " ");
        
        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;
    
        if(status = getaddrinfo(server_ip, server_port, &hints, &serverinfo) != 0) {
            printf("getaddrinfo Error\n");
            continue;
        }
        
        sockfd = socket(serverinfo->ai_family, serverinfo->ai_socktype, serverinfo->ai_protocol);
        if(status = connect(sockfd, serverinfo->ai_addr, serverinfo->ai_addrlen) < 0) {
            printf("Client could not connect to the server\n");
            continue;
        }
        
        FD_SET(sockfd, &readfds);
        int sz = sprintf(send_msg, "%d:%d:%s:%s:", msg.type, msg.size, msg.source, msg.data);
        send(sockfd, send_msg, sz, 0);
        recv(sockfd, recv_buf, 500, 0);
        command = strtok(recv_buf, ":");
        if(strcmp(command, "2") == 0) {
            printf("Login Successful\n");
        }
        else {
            printf("Login Unsuccessful\n");
            command = strtok(NULL, ":");
            command = strtok(NULL, ":");
            command = strtok(NULL, ":");
            printf("%s\n", command);
            continue;
        }

        while(1) {
            FD_SET(sockfd,&readfds);
            FD_SET(STDIN,&readfds);
            select(sockfd+1, &readfds, NULL, NULL, NULL);
            if(FD_ISSET(sockfd, &readfds) == 1){
                memset(recv_buf, 0, 500);
                int rv = recv(sockfd, recv_buf, 500, MSG_DONTWAIT); //MSG_DONTWAIT
                if(rv > 0) {
                    strtok(recv_buf, ":");
                    if(strcmp(recv_buf, "!@#$%^&*Disconnected!@#$%^&*") == 0){
                        printf("Disconnected from the server due to inactivity\n");
                        return 0;
                    }
                    else{
                        printf("%s\n", recv_buf);
                        continue;
                    }
                }
            }
            
            if(FD_ISSET(STDIN, &readfds) == 1){
                fgets(line, 100, stdin);
                if(strlen(line) <= 1){continue;}
                
                command = strtok(line, " \t\r\n\f");
                if(strcmp(command, "/logout") == 0) {
                    msg.type = EXIT;
                    msg.size = 0;
                    strcpy(msg.data, "");
                    int sz = sprintf(send_msg, "%d:%d:%s:%s:", msg.type, msg.size, msg.source, msg.data);
                    send(sockfd, send_msg, sz, 0);
                    close(sockfd);
                    break;
                }
                else if(strcmp(command, "/joinsession") == 0) {
                    msg.type = JOIN;
                    data = strtok(NULL, "\t\r\n\f");
                    msg.size = strlen(data);
                    strcpy(msg.data, data);
                    int sz = sprintf(send_msg, "%d:%d:%s:%s:", msg.type, msg.size, msg.source, msg.data);
                    send(sockfd, send_msg, sz, 0);
                    recv(sockfd, recv_buf, 500, 0);
                    command = strtok(recv_buf, ":");
                    if(strcmp(command, "6") == 0) {
                        command = strtok(NULL, ":");
                        command = strtok(NULL, ":");
                        command = strtok(NULL, ":");
                        printf("Joined Session: %s\n", command);
                    }
                    else {
                        command = strtok(NULL, ":");
                        command = strtok(NULL, ":");
                        command = strtok(NULL, ":");
                        printf("%s\n", command);
                    }
                    continue;
                }
                else if(strcmp(command, "/leavesession") == 0) {
                    msg.type = LEAVE_SESS;
                    msg.size = 0;
                    strcpy(msg.data, "");
                    int sz = sprintf(send_msg, "%d:%d:%s:%s:", msg.type, msg.size, msg.source, msg.data);
                    send(sockfd, send_msg, sz, 0);
                    continue;
                }
                else if(strcmp(command, "/createsession") == 0) {
                    msg.type = NEW_SESS;
                    data = strtok(NULL, "\t\r\n\f");
                    msg.size = strlen(data);
                    strcpy(msg.data, data);
                    int sz = sprintf(send_msg, "%d:%d:%s:%s:", msg.type, msg.size, msg.source, msg.data);
                    
                    send(sockfd, send_msg, sz, 0);
                    recv(sockfd, recv_buf, 500, 0);
                    command = strtok(recv_buf, ":");
                    if(strcmp(command, "10") == 0){
                        printf("New session created\n");
                    }
                    continue;
                }
                else if(strcmp(command, "/msg") == 0) {
                    msg.type = MESSAGE_ONE;
                    data = strtok(NULL, "\t\r\n\f");
                    msg.size = strlen(data);
                    strcpy(msg.data, data);
                    int sz = sprintf(send_msg, "%d:%d:%s:%s:", msg.type, msg.size, msg.source, msg.data);
                    
                    send(sockfd, send_msg, sz, 0);
                }
                else if(strcmp(command, "/list") == 0) {
                    msg.type = QUERY;
                    msg.size = 0;
                    strcpy(msg.data, "");
                    int sz = sprintf(send_msg, "%d:%d:%s:%s:", msg.type, msg.size, msg.source, msg.data);
                    send(sockfd, send_msg, sz, 0);
                    recv(sockfd, recv_buf, 500, 0);
                    command = strtok(recv_buf, ":");
                    command = strtok(NULL, ":");
                    command = strtok(NULL, ":");
                    command = strtok(NULL, ":");
                    printf("%s\n", command);
                    continue;
                }
                else if(strcmp(command, "/quit") == 0) {
                    close(sockfd);
                    return 0;
                }
                else {
                    msg.type = MESSAGE;
                    data = strtok(NULL, "\n\t\f\r");
                    if(data != NULL) {
                        msg.size = strlen(data) + strlen(command) + 1;
                        strcpy(msg.data, data);
                        int sz = sprintf(send_msg, "%d:%d:%s:%s %s", msg.type, msg.size, msg.source, command, msg.data);
                        send(sockfd, send_msg, sz, 0);
                        continue;
                    }
                    else {
                        msg.size = strlen(command);
                        strcpy(msg.data, command);
                        int sz = sprintf(send_msg, "%d:%d:%s:%s", msg.type, msg.size, msg.source, msg.data);
                        send(sockfd, send_msg, sz, 0);
                        continue;
                    }
                }
            }
        }
    }
    return 0; 
}