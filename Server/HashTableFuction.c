//
//  HashTableFuction.c
//  HashTable
//
//  Created by Shen guo on 5/22/18.
//  Copyright © 2018 Shen guo. All rights reserved.
//

#include <stdlib.h>
#include <string.h>
#include "HashTableFuction.h"
#include "LinkedListFunction.h"

int hashFunc(char * key) {
    int keyIndex = 0;
    for(int i = 0; i < strlen(key); i++) {
        keyIndex += *(key + i);
    }
    keyIndex = keyIndex % SIZE;
    return keyIndex;
}

HashNode * hashSearch(HashMap* Hashmap, char * key) {
    int keyIndex = hashFunc(key);
    
    if(Hashmap->hashArr[keyIndex] == NULL) {
        return NULL;
    }
    
    HashNode * temp = Hashmap->hashArr[keyIndex];
    for( ; temp != NULL; temp = temp->next) {
        if(!strcmp(temp->key, key)) {
            return temp;
        }
    }
    return NULL;
}

void hashSet(HashMap* Hashmap, char * key, void * value) {
    int keyIndex = hashFunc(key);
    
    HashNode * toInsert = (HashNode *) malloc(sizeof(HashNode));
    toInsert->key = (char *) malloc (sizeof(char) * (strlen(key) + 1));
    strcpy(toInsert->key, key);
    toInsert->value = value;
    toInsert->next = NULL;
    
    if(Hashmap->hashArr[keyIndex] == NULL) {
        Hashmap->hashArr[keyIndex] = toInsert;
        return;
    }
    
    if(hashSearch(Hashmap, key) != NULL) {
        HashNode * cpy = hashSearch(Hashmap, key);
        cpy->value = value;
        return;
    }
    
    HashNode * temp = Hashmap->hashArr[keyIndex];
    
    while(temp->next != NULL) {
        temp = temp->next;
    }
    temp->next = toInsert;
    return;
}

void * hashGet(HashMap * Hashmap, char * key) {
    HashNode * getResult = hashSearch(Hashmap, key);
    if(getResult == NULL) {
        return NULL;
    }
    return getResult->value;
}

ListNode * hashKeyValuePair(HashMap * Hashmap) {
    ListNode * return_value = NULL;
    for(int i = 0; i < SIZE; i++) {
        if(Hashmap->hashArr[i] == NULL) {
            continue;
        }
        HashNode * temp = Hashmap->hashArr[i];
        Pair * inst_pair = (Pair *) malloc(sizeof(Pair));
        while(temp != NULL) {
            inst_pair->key = temp->key;
            inst_pair->value = temp->value;
            append(&return_value, inst_pair);
            temp = temp->next;
        }
    }
    return return_value;
}

int hashSize(HashMap * Hashmap) {
    int count = 0;
    
    for(int i = 0; i < SIZE; i++) {
        if(Hashmap->hashArr[i] == NULL) {
            continue;
        } else {
            for(HashNode *temp = Hashmap->hashArr[i]; temp != NULL; temp = temp->next) {
                count++;
            }
        }
    }
    return count;
}

void hashDelete(HashMap * Hashmap, char * key) {
    int keyIndex = hashFunc(key);
    
    HashNode dummy;
    dummy.next = Hashmap->hashArr[keyIndex];
    HashNode * temp = &dummy;
    
    while(temp->next != NULL) {
        if(!strcmp(temp->next->key, key)) {
            temp->next = temp->next->next;
            Hashmap->hashArr[keyIndex] = dummy.next;
            return;
        }
        temp = temp->next;
    }
    printf("can't find the name.\n");
}

void hashClear(HashMap * Hashmap) {
    HashNode * temp;
    for(int i = 0; i < SIZE; i++) {
        temp = Hashmap->hashArr[i];
        HashNode * next;
        while (temp != NULL) {
            next = temp->next;
            free(temp);
            temp = next;
        }
        continue;
    }
}
