//
//  HashTableFuction.h
//  HashTable
//
//  Created by Shen guo on 5/22/18.
//  Copyright © 2018 Shen guo. All rights reserved.
//

#ifndef HashTableFuction_h
#define HashTableFuction_h

#include <stdio.h>
#include <string.h>
#include "LinkedListFunction.h"
#include "ClientInfo.h"

#define SIZE 10

//key is client's nickname, value is ClientInfo(when clientmap) or listnode(when groupmap)
typedef struct HashNode {
    char * key;
    void * value;
    struct HashNode * next;
} HashNode;

typedef struct Pair {
    char * key;
    void * value;
} Pair;

typedef struct HashMap {
    HashNode * hashArr[SIZE];
} HashMap;

int hashFunc(char * key);

void hashSet(HashMap* Hashmap, char * key, void * value);

HashNode * hashSearch(HashMap * Hashmap, char * key);

void * hashGet(HashMap* Hashmap, char * key);

ListNode * hashKeyValuePair(HashMap * Hashmap);

int hashSize(HashMap* Hashmap);

void hashDelete(HashMap* Hashmap, char * key);

void hashClear(HashMap* Hashmap);

#endif /* HashTableFuction_h */
