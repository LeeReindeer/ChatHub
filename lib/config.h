#ifndef __CONFIG_H__
#define __CONFIG_H__

#include "dbg.h"
#include <netinet/in.h>
#include <stdint.h>
#include <stdlib.h>
#define MAX_MSG_DATA 1024 * 2 // 2KB
#define MAX_BUFF 1024 * 4     // 4KB
#define MAX_CHARS 20

#define MAX_USER_ON 1024

#define SERVER_IP "128.199.163.149"
#define PORT 3477
#define DB_IP "127.0.0.1"
#define DB_PORT 6379

#define MESSAGE_SUCCESS "SUCCESS"
#define MESSAGE_FAILED "FAILED"

/*ERROR start*/
#define ERROR_NONE 0 // NO ERROR
#define ERROR_NORMAL -1
#define ERROR_USER_EXISTS -2
#define ERROR_USER_NOTEXISTS -3
#define ERROR_PASSWORD -4;
/*ERROR end*/

#define ONLINE 1
#define OFFLINE 0

// use in message senderName
#define CHATHUB_SERVER_NAME "ChatHub"

typedef long long LL;

extern const char *message_types[];

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

typedef struct _User {
  LL userId;  // auto increase
  int sockfd; // this user's sockfd with server
  char username[MAX_CHARS];
  char pass[MAX_CHARS];
  char avatar[MAX_MSG_DATA / 2];
  int isOnline; // send msg to isOnline user immediately
  int64_t registerTime;
  // update this when user logout or close app
  int64_t lastOnlineTime;
  //    char privateKey[MAX_MSG_DATA];
  //    char publicKey[MAX_MSG_DATA];
} User;

/*Message between server and client*/
typedef struct _Message {
  LL msgId;
  LL groupId;
  char senderName[MAX_CHARS];
  char msg[MAX_MSG_DATA];
  int64_t sendTime;
  MessageType type;
} Message;

User *new_user();
Message *new_messgae();
Message *user2msg(User *user);
User *msg2user(Message *msg);

int64_t currentTimeMillis();

#endif