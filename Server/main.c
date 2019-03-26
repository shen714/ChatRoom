//
//  main.c
//  SocketServer
//
//  Created by Shen guo on 6/1/18.
//  Copyright © 2018 Shen guo. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "HashTableFuction.h"
#include "ClientInfo.h"
#include "LinkedListFunction.h"

#define PORT "9034"

/* MESSAGEs */
#define BACKLOG 10   /* how many pending connections queue will hold */
#define LENGTH_BUFF 200 /* 200 bytes */

const char reg_dir[LENGTH_BUFF] = "/Users/shenguo/Desktop/C_progamming/ChattingRoom/SocketServer/SocketServer/clientinfo";
const char group_dir[LENGTH_BUFF] = "/Users/shenguo/Desktop/C_progamming/ChattingRoom/SocketServer/SocketServer/group/";

enum command_type{
    REGISTER,
    LOGIN,
    LOGOUT,
    SEND,
    BROADCAST,
    GROUPSET,
    GROUPMEMBERS,
    GROUPSEND,
    GROUPADD,
    GROUPREMOVE
};

char * command_str[] = {"register", "login", "logout", "send", "broadcast", "groupset", "groupmembers", "groupsend", "groupadd", "groupremove"};

HashMap * clientmap = NULL; /*The map to store login client information whose key is client's nickname*/
char * clientarr[BACKLOG] = {0}; /*The array to store login client information whose index is client's sockID*/
HashMap * groupmap = NULL; /*The map to store group information whose key is groupname and value is group members*/

/* forward declarations */
void * get_in_addr(const struct sockaddr *sa);
char * is_registered_(const char * nickname_checking);
int is_logged_in(int sockfd, char * clientarr[SIZE]);
void register_handler(const int sockfd, char * msg);
void login_handler(const int sockfd, char * msg, HashMap * clientmap, char * clientarr[BACKLOG]);
void logout_handler(const int sockfd, HashMap * clientmap, char * clientarr[BACKLOG]);
char * set_send_buff_(char * sender_nickname, char * sending_msg);
void send_handler(const int sockfd, char * msg, HashMap * clientmap, char * clientarr[BACKLOG]);
void broadcast_handler(const int sockfd, char * msg, HashMap * clientmap, char * clientarr[BACKLOG]);
void save_groupinfo(char * group_name, HashMap * groupmap);
void groupset_handler(const int sockfd, char * msg, HashMap * groupmap, char ** clientarr);
void groupmembers_handler(const int sockfd, char * msg, HashMap * groupmap, char ** clientarr);
void groupsend_handler(const int sockfd, char * msg, HashMap * clientmap, char * clientarr[BACKLOG], HashMap * groupmap);
void groupadd_handler(int sockfd, char * msg, char * clientarr[BACKLOG], HashMap * groupmap);
void groupremove_handler(int sockfd, char * msg, char * clientarr[BACKLOG], HashMap * groupmap);
void broadcast_handler(const int sockfd, char * msg, HashMap * clientmap, char * clientarr[BACKLOG]);
void save_groupinfo(char * group_name, HashMap * groupmap);
void groupset_handler(const int sockfd, char * msg, HashMap * groupmap, char ** clientarr);
void groupsend_handler(const int sockfd, char * msg, HashMap * clientmap, char * clientarr[BACKLOG], HashMap * groupmap);
void groupadd_handler(int sockfd, char * msg, char * clientarr[BACKLOG], HashMap * groupmap);
void groupremove_handler(int sockfd, char * msg, char * clientarr[BACKLOG], HashMap * groupmap);

/*
 get_in_addr: get sockaddr, IPv4 or IPv6.
 @param *sa: a pointer to socket address structure.
 */
void * get_in_addr(const struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    }
    
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

/**
 is_registered_:search into the file to see if the nickname has been used. Return
    NULL if the name has not been used; the nickname and password if it has been used.
 @param nickname_checking the name needed to be checked.
 */
char * is_registered_(const char * nickname_checking) {
    char ch;
    char buff[LENGTH_BUFF] = {0};
    char * buff_cpy = (char *) malloc(LENGTH_BUFF);
    char * nickname;
    FILE * fp = fopen(reg_dir, "r");

    int i = 0;
    while((ch = fgetc(fp)) != EOF) {
        if(ch != ',') {
            buff[i++] = ch;
        } else {
            buff[i] = '\0';
            strcpy(buff_cpy, buff);//copy the buff to store its value.
            nickname = strtok(buff, " ");
            if(!strcmp(nickname, nickname_checking)) {
                return buff_cpy;
            }
            i = 0;
        }
    }
    
    free(buff_cpy);
    fclose(fp);
    return NULL;
}

//0 for not log in; 1 for log in
int is_logged_in(int sockfd, char * clientarr[SIZE]) {
    if(clientarr[sockfd] == NULL) {
        return 0;
    }
    return 1;
}

/**
 register_handler: append the client to the clientmap, set client's status to 
    REG and sent corresponding msg back to client. not store client's sockfd.
 @param sockfd the client's socket
 @param msg the msg client sent, contains client's nickname and password.
 **/
void register_handler(const int sockfd, char * msg) {
    char * nickname = strtok(msg, " ");
    char * password = strtok(NULL, "\n");
    
    if (is_registered_(nickname) == NULL) { // not registered
        //save their nickname and password to database
        FILE * fp;
        fp = fopen(reg_dir, "a");
        
        fprintf(fp, "%s %s,", nickname, password);
        free(is_registered_(nickname));
        fclose(fp);

        char * reg_success = "Successfully register.";
        send(sockfd, reg_success, strlen(reg_success), 0);
        return;
    }
    
    char * name_exist = "The name has been used.";
    send(sockfd, name_exist, strlen(name_exist), 0);
    
    return;
}

/**
 login_handler: change client's status to REG, store client's sockfd and sent 
    corresponding msg back to client.
 @param sockfd the client's socket
 @param msg the msg client sent, contains client's nickname and password.
 **/
void login_handler(const int sockfd, char * msg, HashMap * clientmap, char * clientarr[BACKLOG]) {
    char * nickname = strtok(msg, " ");
    char * password = strtok(NULL, "\n");
    
    if (is_registered_(nickname) == NULL) { // not registered
        char * name_not_exist = "You have not registered.";
        send(sockfd, name_not_exist, strlen(name_not_exist), 0);
        return;
    }
    
    char register_res[LENGTH_BUFF] = {0};
    strcpy(register_res, is_registered_(nickname));
    char * reg_nickname = strtok(register_res, " ");
    char * reg_password = strtok(NULL, "\n");
    
    if (strcmp(reg_password, password)) { //wrong pw.
        char * pw_error = "Your password is wrong.";
        send(sockfd, pw_error, strlen(pw_error), 0);
    } else if(clientarr[sockfd] != NULL) { // repeatedly log in.
        char * repreat_log = "You have already logged in.";
        send(sockfd, repreat_log, strlen(repreat_log), 0);
    } else { //Successfully log in.
        //set the hashmap
        ClientInfo * inst_value = (ClientInfo *) malloc(sizeof(ClientInfo));
        inst_value->key = (char *) malloc (strlen(nickname));
        
        strcpy(inst_value->key, nickname);
        inst_value->value = sockfd;
        hashSet(clientmap, nickname, inst_value);
        
        //set the hasharr
        clientarr[sockfd] = (char *) malloc (strlen(nickname));
        strcpy(clientarr[sockfd], nickname);
        
        char * login_success = "Successfully log in.";
        send(sockfd, login_success, strlen(login_success), 0);
    }
}

/**
 logout_handler: remove the client information from clientmap and sent
    corresponding msg back to client.
 @param sockfd the client's socket
 @param msg the msg client sent, contains client's nickname.
 **/
void logout_handler(const int sockfd, HashMap * clientmap, char * clientarr[BACKLOG]) {
    if (!is_logged_in(sockfd, clientarr)) { // not login
        char * not_log = "You have not logged in.";
        send(sockfd, not_log, strlen(not_log), 0);
        return;
    }
    
    //check for nickname and delete from hasharr
    char * nickname = (char *) malloc(strlen(clientarr[sockfd]));
    strcpy(nickname, clientarr[sockfd]);
    clientarr[sockfd] = NULL;
    
    //delete client from hashmap
    hashDelete(clientmap, nickname);
    
    char * logout_success = "Successfully log out.";
    send(sockfd, logout_success, strlen(logout_success), 0);
}

/**
 set_send_buff_: reformat the msg sending to recvers.
 */
char * set_send_buff_(char * sender_nickname, char * sending_msg) {
    char * sent_buff = (char *) malloc (sizeof(char) * LENGTH_BUFF);
    strcpy(sent_buff, "From ");
    strcat(sent_buff, sender_nickname);
    strcat(sent_buff, ":");
    strcat(sent_buff, sending_msg);
    
    return sent_buff;
}

//sent to one client
void send_handler(const int sockfd, char * msg, HashMap * clientmap, char * clientarr[BACKLOG]) {
    char * receiver_nickname = strtok(msg, " ");
    char * sending_msg = strtok(NULL, "\n");
    
    if (!is_logged_in(sockfd, clientarr)) { // sender not login
        char * not_log = "You have not logged in.";
        send(sockfd, not_log, strlen(not_log), 0);
        return;
    }
    
    ClientInfo * receiver_info = hashGet(clientmap, receiver_nickname);
    if(receiver_info == NULL) {// receiver not logged in
        char * not_log = "The person you sent to has not logged in.";
        send(sockfd, not_log, strlen(not_log), 0);
        return;
    }
    
    //both sender and receiver have logged in
    char * sender_nickname = clientarr[sockfd];
    int receiver_sock = receiver_info->value;
    
    char * send_buff = set_send_buff_(sender_nickname, sending_msg);
    
    if (send(receiver_sock, send_buff, strlen(send_buff), 0) == -1) {
        perror("send");
    } else {
        char * send_success = "Your Msg was sent.";
        send(sockfd, send_success, strlen(send_success), 0);
    }
}

//send msg to client who have logged in
void broadcast_handler(const int sockfd, char * msg, HashMap * clientmap, char * clientarr[BACKLOG]) {
    char * sending_msg = strtok(msg, "\n");
    
    if (!is_logged_in(sockfd, clientarr)) { // sender not login
        char * not_log = "You have not logged in.";
        send(sockfd, not_log, strlen(not_log), 0);
        return;
    }
    
    char * sender_nickname = clientarr[sockfd];
    
    //send msg to client who have logged in
    char receiver_list[LENGTH_BUFF] = {0};
    for(int i = 0; i < BACKLOG; i++) {
        if(clientarr[i] == NULL) {
            continue;
        } else if (i != sockfd){
            //get receiver's sockID and nickname
            int receiver_sock = i;
            char * receiver_nickname = clientarr[i];
            char * send_buff = set_send_buff_(sender_nickname, sending_msg);
            
            send(receiver_sock, send_buff, strlen(send_buff), 0);
            
            strcat(receiver_list, receiver_nickname);
            strcat(receiver_list, " ");
        }
    }
    char send_success[LENGTH_BUFF] = "Your msg was sent to:";
    strcat(send_success, receiver_list);
    send(sockfd, send_success, strlen(send_success), 0);
}

void save_groupinfo(char * group_name, HashMap * groupmap) {
    //get group name and store it in database
    char file_dir[LENGTH_BUFF] = {0};
    strcat(file_dir, group_dir);
    strcat(file_dir, group_name);
    FILE * fp;
    
    fp = fopen(file_dir, "w");
    fprintf(fp, "%s ", group_name);
    
    //get group members's name and save it into file
    ListNode * group_members =(ListNode *)hashGet(groupmap, group_name);
    ListNode * temp = group_members;
    while(temp != NULL) {
        fprintf(fp, "%s ", temp->value);
        temp = temp->next;
    }
    fclose(fp);
}

//member can be added into group as long as he has registered.
void groupset_handler(const int sockfd, char * msg, HashMap * groupmap, char ** clientarr) {
    if (!is_logged_in(sockfd, clientarr)) { // sender not login
        char * not_log = "You have not logged in.";
        send(sockfd, not_log, strlen(not_log), 0);
        return;
    }
    
    char * group_name = strtok(msg, " ");
    if(hashSearch(groupmap, group_name) == NULL) { //groupname has been used
        char * name_exist = "The name has been used.";
        send(sockfd, name_exist, strlen(name_exist), 0);
        return;
    }
    
    hashSet(groupmap, group_name, NULL);
    save_groupinfo(group_name, groupmap);
    char * set_success = "The group has been set.";
    send(sockfd, set_success, strlen(set_success), 0);
}

void groupmembers_handler(const int sockfd, char * msg, HashMap * groupmap, char * clientarr[BACKLOG]) {
    if (!is_logged_in(sockfd, clientarr)) { // sender not login
        char * not_log = "You have not logged in.";
        send(sockfd, not_log, strlen(not_log), 0);
        return;
    }

    char * group_name = strtok(msg, "\n");
    if(!hashSearch(groupmap, group_name)) {//group not exist
        char * group_not_exist = "The group is not exist.";
        send(sockfd, group_not_exist, strlen(group_not_exist), 0);
        return;
    }
    
    ListNode * group_members = hashGet(groupmap, group_name);
    if(!group_members) {// group is empty
        char * group_no_members = "The group is empty.";
        send(sockfd, group_no_members, strlen(group_no_members), 0);
        return;
    }
    
    ListNode * temp = group_members;
    char group_members_res[LENGTH_BUFF] = {0};
    while(temp != NULL) {
        strcat(group_members_res, temp->value);
        strcat(group_members_res, " ");
        temp = temp->next;
    }
    
    send(sockfd, group_members_res, strlen(group_members_res), 0);
}

void groupsend_handler(const int sockfd, char * msg, HashMap * clientmap, char * clientarr[BACKLOG], HashMap * groupmap) {
    
    if (!is_logged_in(sockfd, clientarr)) { // sender not login
        char * not_log = "You have not logged in.";
        send(sockfd, not_log, strlen(not_log), 0);
        return;
    }
    
    char * sender_nickname = clientarr[sockfd];
    
    //check group name, get group members if the group exist
    char * group_name = strtok(msg, " ");
    char * send_msg = strtok(NULL, "\n");
    
    if(!hashSearch(groupmap, group_name)) {//group not exist
        char * group_not_exist = "The group is not exist.";
        send(sockfd, group_not_exist, strlen(group_not_exist), 0);
        return;
    }
    
    ListNode * group_members = hashGet(groupmap, group_name);
    if(!group_members) {// group is empty
        char * group_no_members = "The group is empty.";
        send(sockfd, group_no_members, strlen(group_no_members), 0);
        return;
    }

    //check group member's status and send msg to group members who have logged in
    ListNode * temp = group_members;
    char send_success_members[LENGTH_BUFF] = {0};
    char not_login[LENGTH_BUFF] = {0};
    while(temp != NULL) {
        //check group members' status
        ClientInfo * group_member = hashGet(groupmap, temp->value);
        if(group_member == NULL) {// group member has not logged in
            strcat(not_login, temp->value);
            strcat(not_login, " ");
            temp = temp->next;
        } else { // group member has logged in
            int receiver_sock = group_member->value;
            strcat(send_success_members, temp->value);
            strcat(send_success_members, " ");
            char * sending_msg = set_send_buff_(sender_nickname, send_msg);
            if (send(receiver_sock, sending_msg, strlen(sending_msg), 0) == -1) {
                char send_error[LENGTH_BUFF] = "Failed to sent to ";
                strcat(send_error, temp->value);
                send(sockfd, send_error,strlen(send_error), 0);
            }
        }
    }
    
    char send_success[LENGTH_BUFF] = {0};
    strcat(send_success, "Your Msg was sent to :");
    strcat(send_success, send_success_members);
    strcat(send_success, "\n");
    strcat(send_success, "Other members have not logged in.");
    send(sockfd, send_success, strlen(send_success), 0);

}

void groupadd_handler(int sockfd, char * msg, char * clientarr[BACKLOG], HashMap * groupmap) {
    if (!is_logged_in(sockfd, clientarr)) { // sender not login
        char * not_log = "You have not logged in.";
        send(sockfd, not_log, strlen(not_log), 0);
        return;
    }
    
    //get the group from groupmap
    char * group_name = strtok(msg, " ");
    HashNode * group_info = hashSearch(groupmap, group_name);
    if(group_info == NULL) { //check if the group exist
        char * group_not_exist = "The group is not exist.";
        send(sockfd, group_not_exist, strlen(group_not_exist), 0);
        return;
    }
    
    char * group_member;
    char * group_members[SIZE] = {0};
    int i = 0;
    while((group_member = strtok(NULL, " ")) != NULL) {
        char* group_member_cpy = (char*) malloc(strlen(group_member) + 1);
        strcat(group_member_cpy, group_member);
        group_members[i++] = group_member_cpy;
    }
    
    char added_member[LENGTH_BUFF] = {0};
    
    ListNode * exist_group_members = hashGet(groupmap, group_name);
    i = 0;
    //check every's group members' status
    while(group_members[i] != NULL) {
        if(is_registered_(group_members[i]) != NULL) {//has registered
            if(!find_key(exist_group_members, group_members[i])) {//was not in the group
                //insert group member into groupmember list
                char * inst_gmember = (char *) malloc (strlen(group_members[i]) + 1);
                strcpy(inst_gmember, group_members[i]);
                append(&exist_group_members, inst_gmember);
                
                strcat(added_member, group_members[i]);
            }
        }
        i++;
    }
    
    if(!strcmp(added_member, "")) {
        char * group_error = "Please make sure the people you add has registered and is not in the group;";
        send(sockfd, group_error, strlen(group_error), 0);
    } else {
        //add the group into groupmap
        group_info->value = exist_group_members;
        //save the group in database
        save_groupinfo(group_name, groupmap);
        
        char add_success[LENGTH_BUFF] = "Added members are :";
        strcat(add_success, added_member);
        send(sockfd, add_success, strlen(add_success), 0);
        
        return;
    }
}

void groupremove_handler(int sockfd, char * msg, char * clientarr[BACKLOG], HashMap * groupmap) {
    if (!is_logged_in(sockfd, clientarr)) { // sender not login
        char * not_log = "You have not logged in.";
        send(sockfd, not_log, strlen(not_log), 0);
        return;
    }
    
    //get the group from groupmap
    char * group_name = strtok(msg, " ");
    ListNode * exist_group_members = (ListNode *) hashGet(groupmap, group_name);
    
    if(exist_group_members == NULL) {
        char * group_not_exist = "The group has not been set.";
        send(sockfd, group_not_exist, strlen(group_not_exist), 0);
        return;
    }
    
    //put each group member into an array
    char * group_member;
    char * group_members[SIZE] = {0};
    int i = 0;
    while((group_member = strtok(NULL, " ")) != NULL) {
        group_members[i++] = group_member;
    }
    
    i = 0;
    
    //search group member and remove if found.
    char removed_members[LENGTH_BUFF] = {0};
    while(group_members[i] != NULL) {
        if(delete_key(&exist_group_members, group_members[i]) != NULL) {
            strcat(removed_members, group_members[i]);
        }
        i++;
    }
    
    if(!strcmp(removed_members, "")) {
        char * group_error = "Please make sure the people you removed has registered and is in the group;";
        send(sockfd, group_error, strlen(group_error), 0);
    } else {
        //add the group into groupmap
        hashSet(groupmap, group_name, exist_group_members);
        //save the group in database
        save_groupinfo(group_name, groupmap);
        
        char remove_success[LENGTH_BUFF] = "Removed members are :";
        strcat(remove_success, removed_members);
        send(sockfd, remove_success, strlen(remove_success), 0);
        
        return;
    }
}

//parse the command.
enum command_type get_command(const char * token) {
    enum command_type command;
    if(!strcmp(token, command_str[REGISTER])) {
        command = REGISTER;
    } else if (!strcmp(token, command_str[LOGIN])) {
        command = LOGIN;
    } else if (!strcmp(token, command_str[LOGOUT])) {
        command = LOGOUT;
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
    }
    return command;
}

/**
 msg_handler:go into the corresponding function sccording to the command
 @param sockfd the client's socket
 @param buff the whole msg client sent, contains the command.
 */
void msg_handler(const int sockfd, char buff[], HashMap * clientmap, char * clientarr[BACKLOG], HashMap * groupmap) {
    char * cmd = strtok(buff, ":");
    char * msg = strtok(NULL, "\n");
    enum command_type command = get_command(cmd);
    
    switch(command) {
        case REGISTER:
            register_handler(sockfd, msg);
            break;
        case LOGIN:
            login_handler(sockfd, msg, clientmap, clientarr);
            break;
        case LOGOUT:
            logout_handler(sockfd, clientmap, clientarr);
            break;
        case SEND:
            send_handler(sockfd, msg, clientmap, clientarr);
            break;
        case BROADCAST:
            broadcast_handler(sockfd, msg, clientmap, clientarr);
            break;
        case GROUPSET:
            groupset_handler(sockfd, msg, groupmap, clientarr);
            break;
        case GROUPMEMBERS:
            groupmembers_handler(sockfd, msg, groupmap, clientarr);
            break;
        case GROUPSEND:
            groupsend_handler(sockfd, msg, clientmap, clientarr, groupmap);
            break;
        case GROUPADD:
            groupadd_handler(sockfd, msg, clientarr, groupmap);
            break;
        case GROUPREMOVE:
            groupremove_handler(sockfd, msg, clientarr, groupmap);
            break;
        }
}

void read_groupinfo(HashMap * groupmap) {
    struct dirent * entry;
    DIR * d = opendir(group_dir);
    
    if(d == 0) {
        perror("opendir");
        return;
    }
    
    while((entry = readdir(d)) != 0) {
        if(entry->d_name[0] == '.') {
            continue;
        }
        char fileName[LENGTH_BUFF] = {0};
        strcat(fileName, group_dir);
        strcat(fileName, entry->d_name);
        
        FILE * fp = fopen(fileName, "r");
        
        if (fp == NULL) {
            return;
        }
        
        char buff[LENGTH_BUFF] = {0};
        char * group_name;
        char * group_member;
        
        char ch;
        int i = 0;
        while((ch = fgetc(fp)) != EOF) {
            buff[i++] = ch;
        }
        
        group_name = strtok(buff, " ");
        ListNode * group_member_node = NULL;
        while((group_member = strtok(NULL, " ")) != NULL) {
            char* group_member_cpy = (char*) malloc(strlen(group_member) + 1);
            strcat(group_member_cpy, group_member);
            append(&group_member_node, group_member_cpy);
        }
        hashSet(groupmap, group_name, group_member_node);
    }
    closedir(d);
}

enum map_type {CLIENT, GROUP};

void free_storage(enum map_type type, HashMap* hm) {
    if (!hm) {
        return;
    }
    ListNode * res_list = hashKeyValuePair(hm);
    ListNode * temp = res_list;
    while(temp != NULL) {
        Pair* pair = temp->value;
        free(pair->key);
        if (type == CLIENT) {
            ClientInfo* client_info = pair->value;
            free(client_info->key);
            free(client_info);
        } else if (type == GROUP) {
            ListNode * groupmember_node = ((Pair *)temp->value)->value;
            ListNode * sub_temp = groupmember_node;
            while (sub_temp != NULL) {
                free(sub_temp->value);
                ListNode * sub_next = sub_temp->next;
                free(sub_temp);
                sub_temp = sub_next;
            }
        }
        free(pair);
        ListNode* next = temp->next;
        free(temp);
        temp = next;
    }
    hashClear(hm);
    free(hm);
}

int main(int argc, char const *argv[]) {
    int listener, newfd;
    fd_set read_master, read_fds;
    struct sockaddr_storage remoteaddr;
    int fdmax;
    int yes = 1;
    char remoteIP[INET6_ADDRSTRLEN];
    struct addrinfo hints, *ai, *p;
    
    FD_ZERO(&read_master);
    FD_ZERO(&read_fds);

    struct sockaddr_in address;
    int addrlen = sizeof(address);
    int rv;
    
    HashMap * clientmap = (HashMap *) calloc (1, sizeof(HashMap));
    HashMap * groupmap = (HashMap *) calloc (1, sizeof(HashMap));

    read_groupinfo(groupmap);
    
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if ((rv = getaddrinfo(NULL, PORT, &hints, &ai)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        exit(1);
    }
    printf("Server is on connection.\n");
    
    for(p = ai; p != NULL; p = p->ai_next) {
        listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (listener < 0) {
            perror("server: socket");
            continue;
        }
        
        if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            perror("setsockopt");
            exit(1);
        }

        if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
            close(listener);
            perror("server: bind");
            continue;
        }
        break;
    }
    freeaddrinfo(ai);
    
    if (p == NULL) {
        fprintf(stderr, "selectserver: failed to bind\n");
        exit(2);
    }
    
    if (listen(listener, BACKLOG) == -1) {
        perror("listen");
        exit(3);
    }
    
    FD_SET(listener, &read_master);
    fdmax = listener;

    for(;;) {
        read_fds = read_master;
        if (select(fdmax+1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("select");
            exit(4);
        }
        
        for(int i = 0; i <= fdmax; i++) {
            char buff[LENGTH_BUFF] = {0};
            if (FD_ISSET(i, &read_fds)) {
                if (i == listener) {
                    addrlen = sizeof remoteaddr;
                    newfd = accept(listener, (struct sockaddr *)&remoteaddr, &addrlen);
                    
                    if (newfd == -1) {
                        perror("accept");
                    } else {
                        FD_SET(newfd, &read_master);
                        if (newfd > fdmax) {
                            fdmax = newfd;
                        }
                        printf("selectserver: new connection from %s on "
                               "socket %d\n",
                               inet_ntop(remoteaddr.ss_family, get_in_addr((struct sockaddr*)&remoteaddr), remoteIP, INET6_ADDRSTRLEN),  newfd);
                    }
                } else if (FD_ISSET(i, &read_fds) ) {
                    if(recv(i, buff, sizeof buff, 0) == 0) {
                        //automatically logout when client is offline.
                        logout_handler(i, clientmap, clientarr);
                        printf("selectserver: socket %d hung up\n", i);
                        close(i);
                        FD_CLR(i, &read_master);
                    } else {
                        printf("%s\n", buff);
                        if(!strcmp(buff, "quit:")) {
                            free_storage(CLIENT, clientmap);
                            clientmap = NULL;
                            free_storage(GROUP, groupmap);
                            groupmap = NULL;
                            exit(0);
                        }
                        msg_handler(i, buff, clientmap, clientarr, groupmap);
                    }
                }
            }
        }
    }
    
    return 0;
}
