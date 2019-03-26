//
//  main.c
//  SocketClient
//
//  Created by Shen guo on 6/21/18.
//  Copyright © 2018 Shen guo. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT "9034"
#define LENGTH_BUFF 1024

enum command_type{
    REGISTER,
    LOGIN,
    LOGOUT,
    SEND,
    BROADCAST,
    GROUPSET,
    GROUPSEND,
    GROUPADD,
    GROUPREMOVE,
    GROUPMEMBERS,
    QUIT,
    ERROR
};

char * command_str[] = {"register", "login", "logout", "send", "broadcast", "groupset", "groupsend", "groupadd", "groupremove", "groupmembers", "quit","error"};


int sockfd = 0;

/* forward declarations */
void *get_in_addr(struct sockaddr *sa);
enum command_type get_command(char * token);
void home_screen_();
char * trim_(char * string);
int have_space(char * string);
int is_null(char * string);
void register_login_handler(int sockfd, char * cmd, char * msg);
void logout_handler(int sockfd, char * cmd);
void send_handler(int sockfd, char * cmd, char * msg);
void broadcast_handler(int sockfd, char * cmd, char * msg);
void groupset_groupmembers_handler(int  sockfd, char * cmd, char * msg);
void groupadd_groupremove_handler(int sockfd, char * cmd, char * msg);
void groupsend_handler(int sockfd, char * cmd, char * msg);
void quit_handler();
void compose_and_send_message(int sockefd, char* cmd, char* msg1, char* msg2);
void * send_message(void * sockfd);
void * recv_message(void * sockfd);

void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void home_screen_() {
    printf("====================== Welcome to Chatting Room =====================\n");
    printf("Please make sure you and your friends have registered and logged in.\n");
    printf("Please read the instruction carefully.\n");
    printf("register:name password\n");
    printf("login:name password\n");
    printf("logout:\n");
    printf("send:receiver's name msg\n");
    printf("broadcast:msg\n");
    printf("groupset:groupname\n");
    printf("groupmembers:groupname\n");
    printf("groupsend:groupname msg\n");
    printf("groupadd:groupname groupmember1 groupmember2 ...\n");
    printf("groupremove:groupname groupmember1 groupmember2 ...\n");
    printf("quit:\n");
    printf("NO SPACE in USERNAME and PASSWORD.\n");
    printf("=====================================================================\n");
}

//  login :Amy
//  >Tom :
char * trim_(char * string) {
    //trim from the start
    int i = 0;
    while(string[i] == ' ') {
        i++;
    }
    
    int j = 0;
    for(; j < strlen(string) - i; j++) {
        string[j] = string[j + i];
    }
    string[j] = '\0';
    
    //trim from the end
    
    i = strlen(string) - 1;
    while(string[i] == ' ') {
        i--;
    }
    i++;
    string[i] = '\0';
    
    return string;
}

enum command_type get_command(char * token) {
    enum command_type command;
    if(!strcmp(token, command_str[REGISTER])) {
        command = REGISTER;
    } else if (!strcmp(token, command_str[LOGIN])) {
        command = LOGIN;
    } else if (!strcmp(token, command_str[LOGOUT])) {
        command = LOGOUT;
    } else if (!strcmp(token, command_str[SEND])) {
        command = SEND;
    } else if (!strcmp(token, command_str[BROADCAST])) {
        command = BROADCAST;
    } else if (!strcmp(token, command_str[GROUPSET])) {
        command = GROUPSET;
    } else if (!strcmp(token, command_str[GROUPMEMBERS])) {
        command = GROUPMEMBERS;
    } else if (!strcmp(token, command_str[GROUPSEND])) {
        command = GROUPSEND;
    } else if (!strcmp(token, command_str[GROUPADD])) {
        command = GROUPADD;
    } else if (!strcmp(token, command_str[GROUPREMOVE])) {
        command = GROUPREMOVE;
    } else if (!strcmp(token, command_str[QUIT])) {
        command = QUIT;
    } else {
        command = ERROR;
    }
    return command;
}

//1 for space; 0 for no space
int have_space(char * string) {
    int len = strlen(string);
    int i = 0;
    for (; i < len; i++) {
        if(string[i] == ' ' || string[i] == '\t') {
            return 1;
        }
    }
    return 0;
}

//1 for is null; 0 for is not null
int is_null(char * string) {
    if(string == NULL) {
        return 1;
    }
    return 0;
}

void register_login_handler(int sockfd, char * cmd, char * msg) {
    char * nickname = strtok(msg, " ");
    char * password = strtok(NULL, "\n");
    
    const int nickname_valid = !is_null(nickname);
    const int password_valid = !is_null(password) && !have_space(password);
    if (!nickname_valid) {
        printf("Username cannot be void.\n");
        return;
    }
    
    if (!password_valid) {
        printf("Password cannot be void and cannot have space.\n");
        return;
    }
    
    compose_and_send_message(sockfd, cmd, nickname, password);
}

void logout_handler(int sockfd, char * cmd) {
    compose_and_send_message(sockfd, cmd, NULL, NULL);
}

void send_handler(int sockfd, char * cmd, char * msg) {
    char * receiver = strtok(msg, " ");
    char * send_msg = strtok(NULL, "\n");
    
    const int receiver_valid = !is_null(receiver) && !have_space(receiver);
    const int send_msg_valid = !is_null(send_msg);
    
    if (!receiver_valid) {
        printf("Receiver's nickname cannot be void and cannot have space.\n");
        return;
    }
    if (!send_msg_valid) {
        printf("Your message cannot be void.\n");
        return;
    }
    
    compose_and_send_message(sockfd, cmd, receiver, send_msg);
}

void broadcast_handler(int sockfd, char * cmd, char * msg) {
    char * send_msg = strtok(msg, "\n");
    
    const int send_msg_valid = !is_null(send_msg);
    
    if (!send_msg_valid) {
        printf("Your message cannot be void.\n");
        return;
    }
    compose_and_send_message(sockfd, cmd, send_msg, NULL);
}

void groupset_groupmembers_handler(int sockfd, char * cmd, char * msg) {
    char * group_name = strtok(msg, "\n");
    
    const int group_name_valid = !is_null(group_name) && !have_space(group_name);
    
    if (!group_name_valid) {
        printf("Group name cannot be void and cannot have space.\n");
        return;
    }
    
    compose_and_send_message(sockfd, cmd, group_name, NULL);
}

void groupadd_groupremove_handler(int sockfd, char * cmd, char * msg) {
    char * group_name = strtok(msg, " ");
    char * group_members = strtok(NULL, "\n");
    
    const int group_name_valid = !is_null(group_name) && !have_space(group_name);
    const int group_members_valid = !is_null(group_members);
    
    if (!group_name_valid) {
        printf("Group name cannot be void and cannot have space.\n");
        return;
    }
    if (!group_members_valid) {
        printf("Group member's nickname cannot be void.\n");
        return;
    }
    
    compose_and_send_message(sockfd, cmd, group_name, group_members);
}


void groupsend_handler(int sockfd, char * cmd, char * msg) {
    char * group_name = strtok(msg, " ");
    char * send_msg = strtok(NULL, "\n");
    
    const int group_name_valid = !is_null(group_name) && !have_space(group_name);
    const int send_msg_valid = !is_null(send_msg);

    if (!group_name_valid) {
        printf("Group name cannot be void and cannot have space.\n");
        return;
    }
    
    if (!send_msg_valid) {
        printf("Your message cannot be void.\n");
        return;
    }

    compose_and_send_message(sockfd, cmd, group_name, send_msg);
}

void compose_and_send_message(const int sockfd, char* cmd, char* msg1, char* msg2) {
    if (!cmd && !msg1 && !msg2) {
        return;
    }
    char send_buff[LENGTH_BUFF] = {0};
    strcpy(send_buff, cmd);
    strcat(send_buff, ":");
    if (msg1) {
        strcat(send_buff, msg1);
    }
    if (msg2) {
        strcat(send_buff, " ");
        strcat(send_buff, msg2);
    }
    send(sockfd, send_buff, strlen(send_buff), 0);
}

void quit_handler(int sockfd, char * cmd, char * msg) {
    compose_and_send_message(sockfd, cmd, NULL, NULL);
}

/*
 login : amy
 > tom : haha
 */
void * send_message(void * sockfd) {
    char input_buf[LENGTH_BUFF] = {0};
    
    home_screen_();
    while(1) {
        fgets(input_buf, LENGTH_BUFF, stdin);
        
        char * cmd = strtok(input_buf, ":");
        char * msg = strtok(NULL, "\n");
        char * trim_cmd = trim_(cmd);
        
        enum command_type cmd_type = get_command(trim_cmd);
        
        switch(cmd_type) {
            case REGISTER:
            case LOGIN:
                register_login_handler((int)sockfd, trim_cmd, msg);
                break;
            case LOGOUT:
                logout_handler((int)sockfd, trim_cmd);
                break;
            case SEND:
                send_handler((int)sockfd, trim_cmd, msg);
                break;
            case BROADCAST:
                broadcast_handler((int)sockfd, trim_cmd, msg);
                break;
            case GROUPSET:
            case GROUPMEMBERS:
                groupset_groupmembers_handler((int)sockfd, trim_cmd, msg);
                break;
            case GROUPADD:
            case GROUPREMOVE:
                groupadd_groupremove_handler((int)sockfd, trim_cmd, msg);
                break;
            case GROUPSEND:
                groupsend_handler((int)sockfd, trim_cmd, msg);
                break;
            case QUIT:
                quit_handler((int)sockfd, trim_cmd, msg);
                break;
            default:
                printf("Input Error!\n");
                printf("Please read the instruction!\n");
        }
    }
    return NULL;
}

void * recv_message(void * sockfd) {
    char recv_buf[LENGTH_BUFF] = {0};
    
    while (1) {
        memset(recv_buf, 0, LENGTH_BUFF);
        if (recv((int)sockfd, recv_buf, LENGTH_BUFF, 0) == -1) {
            perror("recv");
            exit(1);
        }
        if (strlen(recv_buf) == 0) {
            exit(0);
        }
        printf("%s\n", recv_buf);
    }
    return NULL;
}

int main() {
    int sockfd;
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];
    
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }
    
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }
        
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("client: connect");
            continue;
        }
        
        break;
    }
    
    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }
    
    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof s);
    printf("client: connecting to %s\n", s);
    
    freeaddrinfo(servinfo);
    
    pthread_t g_sender;
    pthread_t g_receiver;
    
    pthread_create(&g_sender, NULL, send_message, (void*)((long)sockfd));
    pthread_create(&g_receiver, NULL, recv_message, (void*)((long)sockfd));
    
    pthread_join(g_sender, NULL);
    pthread_join(g_receiver, NULL);
    
    close(sockfd);
    return 0;
}
