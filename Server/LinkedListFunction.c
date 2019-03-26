//
//  LinkedListFunction.c
//  SocketServer
//
//  Created by Shen guo on 6/24/18.
//  Copyright © 2018 Shen guo. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "LinkedListFunction.h"

void append(ListNode ** head_ref, void * value) {
    ListNode * new_node = create_node();
    ListNode * last = *head_ref;
    
    new_node->value = value;
    new_node->next = NULL;
    
    if (*head_ref == NULL) {
        *head_ref = new_node;
        return;
    }
    while (last->next != NULL) {
        last = last->next;
    }
    last->next = new_node;
}

ListNode * create_node() {
    return (ListNode*) malloc (sizeof(ListNode));
}

ListNode ** delete_key(ListNode ** head_ref, void * value) {
    if(*head_ref == NULL) {
        return NULL;
    }
    ListNode * temp = *head_ref;
    
    if (!strcmp((*head_ref)->value, value)) {
        *head_ref = temp->next;
        free(temp);
        return head_ref;
    }
    while (temp->next != NULL) {
        if(!strcmp(temp->next->value, value)) {
            free(temp->next);
            temp->next = temp->next->next;
            return head_ref;
        }
        temp = temp->next;
    }
    return NULL;
}

ListNode * find_key(ListNode * head_ref, void * value) {
    ListNode * temp = head_ref;
    while(temp != NULL) {
        if(!strcmp(temp->value, value)) {
            return temp;
        }
        temp = temp->next;
    }
    return NULL;
}

/*
void insert_after (node * head, int val, void * value) {
    node * existing_node = find_node(head, val);
    if (existing_node == NULL) {
        return;
    }
    node* new_node = create_node();
    
    new_node->data = new_data;
    new_node->next = existing_node->next;
    existing_node->next = new_node;
}

node * last_k (node * head, int k) {
    node * current = head, *behind;
    for(int i = 1; i < k; i++ ) {
        if( current->next ) {
            current = current->next;
        } else {
            return NULL;
        }
    }
    behind = head;
    while(current->next != NULL) {
        current = current->next;
        behind = behind->next;
    }
    return behind;
}

int length (node * head) {
    int len = 0;
    node * temp = head;
    while(temp != NULL) {
        temp = temp->next;
        len++;
    }
    return len;
}

void reverse(node ** head_ref) {
    node * prev = NULL;
    node * curr = *head_ref;
    node * next = NULL;
    
    while(curr != NULL) {
        next = curr->next;
        curr->next = prev;
        prev = curr;
        curr = next;
    }
    *head_ref = prev;
}

void print(node * head) {
    node * temp = head;
    while (temp != NULL) {
        printf("%d", temp->data);
        temp = temp->next;
    }
    printf("\n");
}

void push_front(node ** head_ref, int new_data) {
    node * new_node = create_node();
    new_node->data = new_data;
    new_node->next = *head_ref;
    *head_ref = new_node;
}*/
