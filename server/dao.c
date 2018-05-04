#include "dao.h"
#include <hiredis/hiredis.h>

int SET(char *key, char *value);
char *GET(char *key);
int HSET(char *key, char *field, char *value);
char *HGET(char *key, char *field);

static redisContext *c;
static char *TAG = "REDIS";

void open_db(char *host, int port) {
  c = redisConnect(host, port);
  if (c == NULL || c->err) {
    if (c) {
      redisFree(c);
    } else {
      log_d("%s: can't allocate redis context!", TAG);
    }
    exit(1);
  }
}

void close_db() {
  if (c) {
    redisFree(c);
  }
}

/**
 * @brief  save user's info(username, pass, refisterTime).Use next_user_id
 * generate next user's id.
 * @param  *user: user to register, in this time the userId is 0.
 * @retval succeed->userId, failed->error code
 */
LL register_user(User *user);

/**
 * @brief set user online and update lastOnlineTime
 * @param  *user: in this time, user's userId also can be 0,
 *  it will find the userId by username, if not exists return
 * ERROR_USER_NOTEXISTS
 * @retval succeed->userId
 */
int login_user(User *user /*, char *auth*/);

/**
 * @brief  set user offline, also update lastOnlineTime
 * @retval succeed->userId
 */
int logout_user(User *user);

int is_user_exists(redisReply *reply, char *username);

/*ab*/
int get_fd_byname(char *username);
int get_fd_byid(LL id);
int set_fd_byid(LL id);
/*ab*/

/*chatroom dao start*/

/**
 * @brief  this is a spical function for personal char, and group is similar.
 * put two username in a set named like chatroom:1.
 * @param  *username1: the first user in chatroom
 * @param  *username2: the second user in chatroom
 * @retval succeed->chatroom id, else error code
 */
LL create_chatroom(char *username1, char *username2);

// LL create_group(...);

LL del_chatrooom(LL chatroom_id);

/**
 * @brief  remove the given username in the set of chatroom, it will not delete
 * the chatroom and mesages in this chatroom.
 * @param  chatroom_id: chatroom id intent to leave
 * @param  *username: username intent to remove in chatroom
 * @retval success->chatroom id
 */
LL leave_chatrooom(LL chatroom_id, char *username);

int is_user_exists_inchatroom(redisReply *reply, LL chatroom_id,
                              char *username);
/*chatroom dao end*/

/*message dao start*/
/**
 * @brief  save message in a chatroom hashs, message is only distinct in this
 * chatroom, every message's hash key is like: chatroom:1:msgid.And use
 * chatroom:1:next_msg_id to generate next msg id.
 * @param  chatroom_id: room id is used as part of hash key to store message
 * @param  *messge: message to save
 * @retval succeed->messageId, failed->wait to save again
 */
LL save_message(LL chatroom_id, Message *messge);
Message **get_unread_msgs(LL chatroom_id, int64_t time);
/*message dao end*/