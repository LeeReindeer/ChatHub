#ifndef __DAO_H__
#define __DAO_H__

#include "../lib/config.h"
#include <hiredis/hiredis.h>

// int SET(char *key, char *value);
// char *GET(char *key);
// int HSET(char *key, char *field, char *value);
// char *HGET(char *key, char *field);

void open_db(char *host, int port);

void close_db();

/*user dao start*/
LL register_user(User *user);

LL login_user(User *user /*, char *auth*/);

LL logout_user(User *user);

LL set_online(User *user, int fd);
LL set_offline(User *user);

LL get_next_userid();

LL get_userid_by_name(char *username);
int is_user_exists(char *username);

int get_fd_byname(char *username);
int *get_fd_byid(LL chatroom_id, size_t *size);
int set_fd_byid(LL id);
/*user dao end*/

/*chatroom dao start*/
LL get_next_chatroomid();

LL create_chatroom(LL userId1, LL userId2);

// LL create_group(...);

LL del_chatrooom(LL chatroom_id);
LL leave_chatrooom(LL chatroom_id, char *username);

int is_user_exists_inchatroom(LL chatroom_id, char *username);
/*chatroom dao end*/

/*message dao start*/
LL get_next_msgid(LL chatroom_id);
LL save_message(LL chatroom_id, Message *messge);
Message **get_unread_msgs(LL chatroom_id, int64_t time);
/*message dao end*/

#endif // MACRO