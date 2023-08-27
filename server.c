#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <time.h>
#include <math.h>
#include <stdbool.h>

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

int total_clients = 0;
int total_active_clients = 0;
int total_sessions = 0;
char curr_session[50];
struct timeval curr_time;

struct clientList {
    char name[50];
    char pswd[50];
    int sock_fd;
    struct sockaddr_storage from;
    char session_id[50];
    struct timeval active_time;
};

struct message {
    unsigned int type;
    unsigned int size;
    unsigned char source[50];
    unsigned char data[500];
} msg;

struct sessionList {
    char session_id[50];
    int num;
} all_sessions[20];

struct clientList client_list[15];
struct clientList active_list[15];

void initialize_client_list() {
    strcpy(client_list[0].name, "Noah_Williams");
    strcpy(client_list[1].name, "Daniel_Martinez");
    strcpy(client_list[2].name, "Isabella_Wilson");
    strcpy(client_list[0].pswd, "J@zzHands_22");
    strcpy(client_list[1].pswd, "S3cur1tyL0ck$");
    strcpy(client_list[2].pswd, "Qu1ck$ilver");
    total_clients = 3;
}

bool login_client(char buf[],int cli_num) {
    char* temp2 = strtok(NULL, ":");
    char* temp3 = strtok(NULL, ":");
    char* temp4 = strtok(NULL, ":");
    for(int i = 0; i < total_active_clients; i++)
    {
        if(strcmp(active_list[i].name, temp3) == 0 && strcmp(active_list[i].pswd, temp4) == 0)
            return false;
    }
    for(int i = 0; i < total_clients; i++) {
            if(strcmp(client_list[i].name, temp3) == 0 && strcmp(client_list[i].pswd, temp4) == 0) {
                strcpy(active_list[total_active_clients].name, temp3);
                strcpy(active_list[total_active_clients].pswd, temp4);
                strcpy(active_list[total_active_clients].session_id, "");
                gettimeofday(&active_list[total_active_clients].active_time, NULL);
                return true;
            }
        }
    return false;
}

bool new_session(char buf[], int sock_fd) {
    char* temp2 = strtok(NULL, ":");
    char* temp3 = strtok(NULL, ":");
    char* temp4 = strtok(NULL, ":");
    for(int i = 0; i < total_active_clients; i++){
        if(strcmp(active_list[i].name, temp3) == 0){
            gettimeofday(&active_list[i].active_time, NULL);
        }
    }
    for(int i = 0; i < total_active_clients; i++) {
        if(strcmp(active_list[i].name, temp3) == 0 && sock_fd == active_list[i].sock_fd) {
            strcpy(active_list[i].session_id, temp4);
            strcpy(all_sessions[total_sessions].session_id, temp4);
            all_sessions[total_sessions].num = 1;
            total_sessions++;
            return true;
        }
    }
    return false;  
}

bool join_session(char buf[], int sock_fd) {
    char* temp2 = strtok(NULL, ":");
    char* temp3 = strtok(NULL, ":");
    char* temp4 = strtok(NULL, ":");
    for(int i = 0; i < total_active_clients; i++){
        if(strcmp(active_list[i].name, temp3) == 0){
            gettimeofday(&active_list[i].active_time, NULL);
        }
    }
    strcpy(curr_session, temp4);
    int temp = -1;
    for(int i = 0; i < total_active_clients; i++) {
        if(strcmp(active_list[i].session_id, temp4) == 0) {temp = 1;}
    }
    if(temp == -1) {return false;}
    
    for(int i = 0; i < total_active_clients; i++) {
        if(strcmp(active_list[i].name, temp3) == 0 && sock_fd == active_list[i].sock_fd) {
            strcpy(active_list[i].session_id, temp4);
            return true;
        }
    }
}

bool leave_session(char buf[], int sock_fd) {
    char* temp2 = strtok(NULL, ":");
    char* temp3 = strtok(NULL, ":");
    char* temp4 = strtok(NULL, ":");
    for(int i = 0; i < total_active_clients; i++) {
        if(strcmp(active_list[i].name, temp3) == 0) {
            gettimeofday(&active_list[i].active_time, NULL);
        }
    }
    for(int i = 0; i < total_active_clients; i++) {
        if(strcmp(active_list[i].name, temp3) == 0 && sock_fd == active_list[i].sock_fd) {
            strcpy(active_list[i].session_id, "");
            return true;
        }
    }
}

void list_gen(char buf[], int sock_fd) {
    char* temp2 = strtok(NULL, ":");
    char* temp3 = strtok(NULL, ":");
    char* temp4 = strtok(NULL, ":");
    for(int i = 0; i < total_active_clients; i++) {
        if(strcmp(active_list[i].name, temp3) == 0){
            gettimeofday(&active_list[i].active_time, NULL);
        }
    }
    char full_list[1000];
    char final_msg[1000];
    memset(full_list, 0, 1000);
    memset(final_msg, 0, 1000);
    int full_length;
    for(int i = 0; i < total_active_clients; i++) {
        if(strcmp(active_list[i].session_id, "") != 0) {
            strcat(full_list, active_list[i].name);
            strcat(full_list, " is in session ");
            strcat(full_list, active_list[i].session_id);
            strcat(full_list, ", ");
        }
    }
    msg.type = QU_ACK;
    full_length = strlen(full_list);
    msg.size = full_length;
    strcpy(msg.source, "source");
    strcpy(msg.data, full_list);
    full_length = sprintf(final_msg, "%d:%d:%s:%s:", msg.type, msg.size, msg.source, msg.data);
    send(sock_fd, final_msg, full_length, 0);
    return;
}

int main(int argc, char* argv[]) {
    //First we connect to socket then we bind. Then we recv
    int cli_num = 0;
    fd_set master;    //master file descriptor list
    fd_set read_fds;  //temp file descriptor list for select()
    int fdmax, newfd;
    char send_msg[1000];
    
    int server_sock_fd, rv, nbytes;
    struct addrinfo hints;
    struct addrinfo *res;
    struct sockaddr_storage their_addr = {0};
    struct sockaddr_storage remoteaddr; //client address
    socklen_t addrlen;
    
    //First we initialize the client list.
    initialize_client_list();
    //Initializing the server socket
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if(argc < 2) {
        printf("server: Too less arguments\n");
        return -1;
    }
    else if(argc > 2) {
        printf("server: Too many arguments\n");
        return -1;
    }
    rv = getaddrinfo(NULL, argv[1], &hints, &res);
    if (rv != 0) {
        printf("getaddrinfo:\n");
        return 1;
    }
    server_sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(server_sock_fd == -1) {
        printf("server: failed to connect to socket\n");
        return 1;
    }
    if(bind(server_sock_fd, res->ai_addr, res->ai_addrlen) < 0) {
        close(server_sock_fd);
        printf("server: failed to bind socket\n");
        return 1;
    }

    if(listen(server_sock_fd, 5) < 0) {
        close(server_sock_fd);
        printf("server: failed to become listener \n");
        return 1;
    }
    
    FD_ZERO(&master);    //clear the master and temp sets
    FD_ZERO(&read_fds);
    FD_SET(server_sock_fd, &master);
    //keep track of the biggest file descriptor
    fdmax = server_sock_fd;
    struct timeval tv;
    tv.tv_sec = 2;
    tv.tv_usec = 500000;
    while(1){
        for(int a = 0; a < total_active_clients; a++){
            strcpy(msg.data, "!@#$%^&*Disconnected!@#$%^&*");
            msg.size = strlen(msg.data);
            gettimeofday(&curr_time, NULL);
            if(curr_time.tv_sec - active_list[a].active_time.tv_sec > 60 && active_list[a].active_time.tv_sec != 0){
                send(active_list[a].sock_fd, msg.data, msg.size, 0);
                active_list[a].active_time.tv_sec = 0;
            }
        }
        read_fds = master;
        if (select(fdmax+1, &read_fds, NULL, NULL, &tv) == -1) {
            printf("select: error\n");
            exit(4);
        }
        for(int i = 0; i <= fdmax; i++) {
            if (FD_ISSET(i, &read_fds)) {
                if (i == server_sock_fd) { 
                    //handle new connections
                    addrlen = sizeof remoteaddr;
                    newfd = accept(server_sock_fd, (struct sockaddr *)&remoteaddr, &addrlen);
                    if (newfd == -1) {
                        printf("accept: error\n");
                    } 
                    else {
                        FD_SET(newfd, &master); //add to master set
                        if (newfd > fdmax)     //keep track of the max
                            fdmax = newfd;
                    }
                }
                else {
                    char buf[1000];
                    memset(buf, 0, 1000);
                    nbytes = recv(i, buf, 1000, 0);
                    if(nbytes == 0) {
                        bool stat = false;
                        for(int j = 0; j < total_active_clients; j++) {
                            if(i == active_list[j].sock_fd) {
                                strcpy(active_list[j].name, "");
                                strcpy(active_list[j].pswd,"");
                                strcpy(active_list[j].session_id,"");
                                stat = true;
                                continue;
                            }
                            if(stat == true && j-1 >= 0)
                            {
                                strcpy(active_list[j-1].name, active_list[j].name);
                                strcpy(active_list[j-1].pswd, active_list[j].pswd);
                                strcpy(active_list[j-1].session_id, active_list[j].session_id);
                                strcpy(active_list[j].name, "");
                                strcpy(active_list[j].pswd,"");
                                strcpy(active_list[j].session_id,"");
                                active_list[j-1].sock_fd = active_list[j].sock_fd;
                            }
                        }
                        close(i);
                        FD_CLR(i, &master);
                        continue;
                    }
                    char* temp1 = strtok(buf, ":");
                    if(atoi(temp1) == LOGIN) {
                        bool login_stat = login_client(buf, cli_num);
                        if(login_stat == true) {
                            active_list[total_active_clients].sock_fd = newfd;
                            active_list[total_active_clients].from = remoteaddr;
                            msg.type = LO_ACK;
                            msg.size = 0;
                            strcpy(msg.source, "server");
                            strcpy(msg.data, "");
                            int sz = sprintf(send_msg, "%d:%d:%s:%s:", msg.type, msg.size, msg.source, msg.data);
                            send(i, send_msg, sz, 0);
                            total_active_clients++;
                            FD_SET(newfd, &master); //add to master set
                            if (newfd > fdmax)     //keep track of the max
                                fdmax = newfd;
                            continue;
                        }
                        else {
                            msg.type = LO_NAK;
                            strcpy(msg.source, "server");
                            strcpy(msg.data, "Incorrect login details entered");
                            msg.size = strlen(msg.data);
                            int sz = sprintf(send_msg, "%d:%d:%s:%s:", msg.type, msg.size, msg.source, msg.data);
                            send(i, send_msg, sz, 0);
                        }
                    }
                    if(atoi(temp1) == NEW_SESS) {
                        bool new_sess_stat = new_session(buf, i);
                        if(new_sess_stat) {
                            msg.type = NS_ACK;
                            msg.size = 0;
                            strcpy(msg.source, "server");
                            strcpy(msg.data, "");
                            int sz = sprintf(send_msg, "%d:%d:%s:%s:", msg.type, msg.size, msg.source, msg.data);
                            send(i, send_msg, sz, 0);
                        }
                    }
                    if(atoi(temp1) == JOIN) {
                        bool join_sess_stat = join_session(buf, i);
                        if(join_sess_stat) {
                            msg.type = JN_ACK;
                            strcpy(msg.source, "server");
                            strcpy(msg.data, curr_session);
                            msg.size = strlen(msg.data);
                            int sz = sprintf(send_msg,"%d:%d:%s:%s:", msg.type, msg.size, msg.source, msg.data);
                            send(i, send_msg, sz, 0);
                        }
                        else {
                            msg.type = JN_NAK;
                            strcpy(msg.source, "server");
                            strcpy(msg.data, "Error in joining - session not available");
                            msg.size = strlen(msg.data);
                            int sz = sprintf(send_msg, "%d:%d:%s:%s:", msg.type, msg.size, msg.source, msg.data);
                            send(i, send_msg, sz, 0);
                        }
                    }
                    if(atoi(temp1) == LEAVE_SESS) {
                        bool leave_sess_stat = leave_session(buf, i);
                    }
                    if(atoi(temp1) == MESSAGE) {
                        char info[100];
                        char* temp2 = strtok(NULL, ":");
                        char* temp3 = strtok(NULL, ":");
                        char* temp4 = strtok(NULL, "\n");
                        for(int i = 0; i < total_active_clients; i++){
                            if(strcmp(active_list[i].name, temp3) == 0){
                                gettimeofday(&active_list[i].active_time, NULL);
                            }
                        }
                        strcpy(info, temp4);
                        int len = strlen(info);
                        for(int i = 0; i < total_active_clients; i++) {
                            if(strcmp(active_list[i].name, temp3) == 0) {
                                strcpy(active_list[i].session_id, curr_session);
                            }
                        }
                        for(int i = 0; i < total_active_clients; i++) {
                            if(strcmp(active_list[i].name, temp3) != 0 && strcmp(active_list[i].session_id, curr_session) == 0) {
                                int actual = send(active_list[i].sock_fd, info, len, 0);
                            }
                        }    
                    }
                    if(atoi(temp1) == MESSAGE_ONE) {
                        char info[100];
                        char* temp2 = strtok(NULL, ":");
                        char* temp3 = strtok(NULL, ":");
                        char* temp4 = strtok(NULL, ":");
                        char* temp5 = strtok(NULL, "\n");
                        for(int i = 0; i < total_active_clients; i++){
                            if(strcmp(active_list[i].name, temp3) == 0){
                                gettimeofday(&active_list[i].active_time, NULL);
                            }
                        }
                        strcpy(info, temp3);
                        strcat(info," sent you - ");
                        strcat(info, temp5);
                        int len = strlen(info);
                        for(int i = 0; i < total_active_clients; i++) {
                            if(strcmp(active_list[i].name, temp4) == 0 && strcmp(active_list[i].name, temp3) != 0) {
                                int actual = send(active_list[i].sock_fd, info, len, 0);

                            }
                        }   
                    }
                    if(atoi(temp1) == QUERY) {
                        list_gen(buf, i);
                    }
                }  
            }
        }
    }
    return 0;
}