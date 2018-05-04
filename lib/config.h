#ifndef __CONFIG_H__
#define __CONFIG_H__

#include "dbg.h"
// #include "net.h"
#include <netinet/in.h>
#include <stdint.h>
#include <stdlib.h>
#define MAX_MSG_DATA 1024 * 2 // 2KB
#define MAX_BUFF 1024 * 8     // 8KB
#define MAX_CHARS 20

#define MAX_USER_ON 1024

#define SERVER_IP "128.199.163.149"
#define PORT 3477
#define DB_IP "127.0.0.1"
#define DB_PORT 6379

#define MESSAGE_SUCCESS "SUCCESS"
#define MESSAGE_FAILED "FAILED"

typedef long long LL;

typedef enum {
  TEXT = 0, // 0
  IMAGE,    // 1 image is also a string, string of image url

  LOGIN,    // 2
  REGISTER, // 3
  LOGOUT,   // 4

  LIST_CHAT,   // 5 if is a personal chat show isOnline or offline
  LIST_FRIEND, // 6 same as above

  ADD_FRIEND,          // 7
  ADD_FRIEND_TO_GROUP, // 8
  DEL_FRIEND,          // 9
  ADD_GROUP,           // 10 NEW!! create a group
  DEL_GROUP, // 11 delete group if you'r the own of the group, else just
             // leave

  SNY_MSG, // 12
  DEL_MSG, // 13

  EXIT // 14
} MessageType;

/*
typedef enum {
    LOGIN_R,
    LOGOUT_R,
    DESTROY_R, //remove this account from server
    REGISTER_R,
    SEND_R,   //send msg
    RECV_R,   //recv msg
} RequestType;*/

typedef struct _User {
  LL userId;  // AUTO increase
  int sockfd; // this user's sockfd with server
  char username[MAX_CHARS];
  char pass[MAX_CHARS];
  // char nickname[MAX_CHARS];
  char avatar[MAX_MSG_DATA / 2];

  int isOnline; // send msg to isOnline user immediately

  int64_t registerTime;
  // update this when user logout or close app,
  // uses this filed to check new message
  int64_t lastOnlineTime;
  //    char privateKey[MAX_MSG_DATA];
  //    char publicKey[MAX_MSG_DATA];
} User; // 64 B

/*Message between server and client*/
typedef struct _Message {
  // char msgId[MAX_CHARS];
  LL msgId;
  LL groupId; /* if message is personal chat groupId is receiver's
                            userId, else is chatRoom id*/
  char senderName[MAX_CHARS];
  char msg[MAX_MSG_DATA]; // 2048
  int64_t sendTime;
  MessageType type;
} Message;

User *new_user();
Message *new_messgae();
Message *user2msg(User *user);
User *msg2user(Message *msg);

int64_t currentTimeMillis();

#endif