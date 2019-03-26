//
//  ClientInfo.h
//  SocketServer
//
//  Created by Shen guo on 6/18/18.
//  Copyright © 2018 Shen guo. All rights reserved.
//

#ifndef ClientInfo_h
#define ClientInfo_h

//key is client's nickname, value is client's sockID
typedef struct ClientInfo {
    char * key;
    int value;
} ClientInfo;

/*
 ClientMap: 
    HashNode->key:(char *)nickname, HashNode->value:(ClientInfo *)->key:(char *)nickname
                                                    (ClientInfo *)->value:(int) sockID
 
 GroupMap:
    HashNode->key:(char *)groupname, HashNode->value:(ListNode *)->value:(char *)groupmember
 */
#endif /* ClientInfo_h */
