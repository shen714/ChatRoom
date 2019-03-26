//
//  LinkedListFunction.h
//  SocketServer
//
//  Created by Shen guo on 6/24/18.
//  Copyright © 2018 Shen guo. All rights reserved.
//

#ifndef LinkedListFunction_h
#define LinkedListFunction_h

//value is ClientInfo.
typedef struct node {
    void * value;
    struct node* next;
}ListNode;

void append(ListNode ** head_ref, void * value);
ListNode * create_node();
ListNode ** delete_key(ListNode ** head_ref, void * value);
ListNode * find_key(ListNode * head, void * value);

void insert_after (ListNode * head, int val, char * value);
ListNode * last_k (ListNode * head, int k);
int length(ListNode * head);
void reverse(ListNode ** head_ref);
void print(ListNode * head);
void push_front(ListNode ** head_ref, char * value);
#endif /* LinkedListFunction_h */
