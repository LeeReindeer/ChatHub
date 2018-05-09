#include "dao.h"
#include <hiredis/hiredis.h>

/*compiler(as lib): gcc -c -o dao.o dao.c -lhiredis*/
/*compiler(has main): gcc dao.c ../lib/config.o -o dao -lhiredis*/
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
  log_d("REDIS OPEN");
}

void close_db() {
  if (c) {
    redisFree(c);
  }
  log_d("REDIS CLOSE");
}

/**
 * @brief  save user's info(username, pass, refisterTime).Use next_user_id
 * generate next user's id.
 * @param  *user: user to register, in this time the userId is 0.
 * @retval succeed->userId, failed->error code
 */
LL register_user(User *user) {
  redisReply *reply;

  // check username exists
  int exists = is_user_exists(user->username);
  check(exists != ERROR_NORMAL, "DB: error");
  log_d("exists: %d", exists);
  if (exists) {
    log_w("%s: register username exists.", TAG);
    return ERROR_USER_EXISTS;
  }

  // generate userId
  user->userId = get_next_userid();
  log_d("userID: %lld", user->userId);

  // store user info
  reply = redisCommand(
      c, "HMSET user:%lld username %s pass %s registerTime %ld", user->userId,
      user->username, user->pass, currentTimeMillis());
  check(reply->type != REDIS_REPLY_ERROR, "DB: error");
  freeReplyObject(reply);

  // set user hash table
  reply = redisCommand(c, "HSET users %s %lld", user->username, user->userId);
  check(reply->type != REDIS_REPLY_ERROR, "DB: error");

  return user->userId;

error:
  return ERROR_NORMAL;
}

/**
 * @brief check username and pass in db, set user online and update
 * lastOnlineTime.Need call set_user_online at server
 * @param  *user: in this time, user's userId also can be 0,
 *  it will find the userId by username, if not exists return
 * ERROR_USER_NOTEXISTS
 * @retval succeed->userId
 */
LL login_user(User *user /*, char *auth*/) {
  redisReply *reply;
  int exists = is_user_exists(user->username);
  check(exists != ERROR_NORMAL, "DB: error");
  if (!exists) {
    return ERROR_USER_NOTEXISTS;
  }

  if (!user->userId) {
    reply = redisCommand(c, "HGET users %s", user->username);
    check(reply->type != REDIS_REPLY_ERROR, "DB: error");

    user->userId = atoll(reply->str);
  }

  reply = redisCommand(c, "HMGET user:%lld username pass", user->userId);
  check(reply->type != REDIS_REPLY_ERROR, "DB: error");
  check(reply->elements == 2, "DB: get error");

  // check pass and username match
  if (!strcmp(user->username, reply->element[0]->str) &&
      !strcmp(user->pass, reply->element[1]->str)) {
    return user->userId;
  }

  return ERROR_NORMAL;

error:
  return ERROR_NORMAL;
}

/**
 * @brief  set user offline, also update lastOnlineTime
 * @retval succeed->userId
 */
LL logout_user(User *user) { return set_offline(user); }

LL get_next_userid() {
  redisReply *reply = redisCommand(c, "INCR next_user_id");
  check(reply->type != REDIS_REPLY_ERROR, "DB: error");
  int rc = reply->integer;
  freeReplyObject(reply);
  return rc;

error:
  return ERROR_NORMAL;
}

/**
 * @brief  call this function after loging, update user info(lastOnlineTime, fd,
 * online status)
 */
LL set_online(User *user, int fd) {
  redisReply *reply = redisCommand(
      c, "HMSET user:%lld isOnline %d sockfd %d lastOnlineTime %ld",
      user->userId, 1, fd, currentTimeMillis());
  check(reply->type != REDIS_REPLY_ERROR, "DB: error");

  return user->userId;
error:
  return ERROR_NORMAL;
}

LL set_offline(User *user) {
  redisReply *reply = redisCommand(
      c, "HMSET user:%lld isOnline %d sockfd %d lastOnlineTime %ld",
      user->userId, 0, -1, currentTimeMillis());
  check(reply->type != REDIS_REPLY_ERROR, "DB: error");
  freeReplyObject(reply);
  return user->userId;
error:
  return ERROR_NORMAL;
}

/**
 * @brief  get user id by username
 * @retval susseed->userId
 */
LL get_userid_by_name(char *username) {
  redisReply *reply = redisCommand(c, "HGET users %s", username);
  check(reply->type != REDIS_REPLY_ERROR, "DB: error");

  char id_str[MAX_MSG_DATA];
  strcpy(id_str, reply->str);
  LL id = atoll(id_str);

  freeReplyObject(reply);

  return id;

error:
  return ERROR_NORMAL;
}

/**
 * @param  *username: username to check
 * @retval  1 if username exists, 0 if username not exists.
 */
int is_user_exists(char *username) {
  redisReply *reply = redisCommand(c, "HEXISTS users %s", username);
  if (reply->type == REDIS_REPLY_ERROR) {
    return ERROR_NORMAL;
  }
  int rc = (int)reply->integer;
  freeReplyObject(reply);

  return rc;
}

/*ab*/
int get_fd_byname(char *username) { return -1; }
/**
 * @brief find all valid sockfds(online) in this chatroom by the chatroom id.
 * @param  id: chatroom id
 * @param  size: receiver to get size of sockfds
 * @retval pointer of all sockfds
 */
int *get_fd_byid(LL chatroom_id, size_t *size) {
  *size = 0;
  redisReply *reply = redisCommand(c, "SMEMBERS chatroom:%lld", chatroom_id);
  check(reply->type != REDIS_REPLY_ERROR, "DB: error");

  int *fds = calloc(reply->elements, sizeof(int));
  check_mem(fds);

  for (int i = 0; i < reply->elements; i++) {
    int fd = atoi(reply->element[i]->str);
    log_d("fd: %d", fd);
    if (fd > 0) {
      fds[(*size)++] = fd;
    }
  }

  freeReplyObject(reply);

  return fds;

error:
  return NULL;
}

int set_fd_byid(LL id) { return -1; }
/*ab*/

/*chatroom dao start*/
LL get_next_chatroomid() {
  redisReply *reply = redisCommand(c, "INCR next_chatroom_id");
  check(reply->type != REDIS_REPLY_ERROR, "DB: error");
  int rc = reply->integer;
  freeReplyObject(reply);
  return rc;

error:
  return ERROR_NORMAL;
}
/**
 * @brief  this is a spical function for personal char, and group is similar.
 * put two username in a set named like chatroom:1.
 * @param  *username1: the first user in chatroom
 * @param  *username2: the second user in chatroom
 * @retval succeed->chatroom id, else error code
 */
LL create_chatroom(LL userId1, LL userId2) {

  redisReply *reply;

  int chatroom_id = get_next_chatroomid();
  check(chatroom_id != ERROR_NORMAL, "can't create chatroom");

  reply = redisCommand(c, "SADD chatroom:%lld %lld %lld", chatroom_id, userId1,
                       userId2);
  check(reply->type != REDIS_REPLY_ERROR, "DB: error");
  freeReplyObject(reply);
  return chatroom_id;
error:
  return ERROR_NORMAL;
}

// LL create_group(...);

/**
 * @brief  delete chatroom and all messages.
 *
 **/
LL del_chatrooom(LL chatroom_id) {
  redisReply *reply;

  reply = redisCommand(c, "KEYS chatroom:%lld*", chatroom_id);
  check(reply->type != REDIS_REPLY_ERROR, "DB: error");

  long nums = reply->elements;
  log_d("need deelte %ld keys", nums);
  char **keys = (char **)malloc(nums * sizeof(char *));
  for (int i = 0; i < nums; i++) {
    keys[i] = malloc(100 * sizeof(char));
  }
  check_mem(keys);
  // copy keys needed to delete
  for (int i = 0; i < nums; i++) {
    // keys[i] = reply->element[i]->str;
    strcpy(keys[i], reply->element[i]->str);
    // log_d("%d) %s", i + 1, keys[i]);
  }
  freeReplyObject(reply);

  for (int i = 0; i < nums; i++) {
    reply = redisCommand(c, "DEL %s", keys[i]);
    if (reply->type == REDIS_REPLY_ERROR) {
      continue;
    }
    // log_d("deelte: %s->%lld", keys[i], reply->integer);
    freeReplyObject(reply);
  }
  free(keys);
  return chatroom_id;

error:
  return ERROR_NORMAL;
}

/**
 * @brief  remove the given username in the set of chatroom, it will not delete
 * the chatroom and mesages in this chatroom.
 * @param  chatroom_id: chatroom id intent to leave
 * @param  *username: username intent to remove in chatroom
 * @retval success->chatroom id
 */
LL leave_chatrooom(LL chatroom_id, char *username) {
  redisReply *reply;
  reply = redisCommand(c, "SREM chatroom:%lld %s ", chatroom_id, username);
  check(reply->type != REDIS_REPLY_ERROR, "DB: error");
  freeReplyObject(reply);

  return chatroom_id;
error:
  return ERROR_NORMAL;
}

int is_user_exists_inchatroom(LL chatroom_id, char *username) {
  redisReply *reply;

  reply = redisCommand(c, "SISMEMBER chatroom:%lld %s", chatroom_id, username);
  check(reply->type != REDIS_REPLY_ERROR, "DB: error");
  int rc = (int)reply->integer;
  freeReplyObject(reply);

  return rc;
error:
  return ERROR_NORMAL;
}
/*chatroom dao end*/

/*message dao start*/
LL get_next_msgid(LL chatroom_id) {
  redisReply *reply =
      redisCommand(c, "INCR chatroom:%lld:next_msg_id", chatroom_id);
  check(reply->type != REDIS_REPLY_ERROR, "DB: error");

  int rc = reply->integer;
  freeReplyObject(reply);

  return rc;

error:
  return ERROR_NORMAL;
}
/**
 * @brief  save message in a chatroom hashs, message is only distinct in this
 * chatroom, every message's hash key is like: chatroom:1:msgid.And use
 * chatroom:1:next_msg_id to generate next msg id.
 * @param  chatroom_id: room id is used as part of hash key to store message
 * @param  *messge: message to save
 * @retval succeed->messageId, failed->wait to save again
 */
LL save_message(LL chatroom_id, Message *message) {
  redisReply *reply;

  int msg_id = get_next_msgid(chatroom_id);
  check(msg_id != ERROR_NORMAL, "can't get next msg id");
  if (message->sendTime <= 0) {
    message->sendTime = currentTimeMillis();
  }
  reply = redisCommand(
      c, "HMSET chatroom:%lld:%lld msg %s senderName %s sendTime %ld type %d",
      chatroom_id, msg_id, message->msg, message->senderName, message->sendTime,
      message->type);
  check(reply->type != REDIS_REPLY_ERROR, "DB: error");
  freeReplyObject(reply);

  return msg_id;

error:
  return ERROR_NORMAL;
}
// todo
Message **get_unread_msgs(LL chatroom_id, int64_t time) { return NULL; }
/*message dao end*/

// void test_register(User *user) {
//   user->userId = register_user(user);
//   log_d("id: %lld", user->userId);
//   if (user->userId == ERROR_USER_EXISTS) {
//     printf("ussername exists.\n");
//     return;
//   }
//   printf("register succeed, your userId is: %lld\n", user->userId);
// }

// int main() {
//   open_db(DB_IP, DB_PORT);

//   Message *message2 = new_messgae();
//   char format2[MAX_MSG_DATA] = {0};
//   sprintf(format2, "%s %s %ld", "simon", "1111", currentTimeMillis());
//   strcpy(message2->msg, format2);
//   printf("format: %s\n", message2->msg);

//   // test server unpacket message
//   User *user2 = msg2user(message2);
//   user2->lastOnlineTime = currentTimeMillis();

//   test_register(user2);

//   free(message2);
//   free(user2);

//   close_db();
// }