#include "dao.h"
#include <assert.h>
#include <unistd.h>

/*compiler:  gcc test_dao.c ../bin/dao.o ../bin/config.o -o test -lhiredis*/
// void test_reply() {
//   redisReply *reply;

//   reply = redisCommand(c, "HGET user:5 registerTime");
//   printf("str: %s\n", reply->str);
//   printf("type: %d\n", reply->type);
//   printf("integer: %lld\n", reply->integer);
//   printf("len: %d\n", reply->len);

//   int64_t t = atol(reply->str);
//   printf("registerTime: %ld\n", t);
//   freeReplyObject(reply);

//   reply = redisCommand(c, "GET next_user_id");
//   printf("str: %s\n", reply->str);
//   printf("type: %d\n", reply->type);
//   printf("integer: %lld\n", reply->integer);
//   printf("len: %d\n", reply->len);
//   freeReplyObject(reply);
// }

/**
void test_wrapper() {
  char *id = GET("next_user_id");
  printf("%s\n", id);

  int rc = SET("TEST", "1");
  check(rc != -1, "SET error");

  char *s = HGET("user:5", "username");
  printf("username: %s\n", s);

  rc = HSET("user:5", "fd", "1");
  check(rc != -1, "HSET error");

  free(id);
  free(s);
  return;

error:
  free(id);
  free(s);
  close_db();
}
**/

void test_login(User *user) {
  int rc = login_user(user);
  printf("result: %d\n", rc);
  if (rc) {
    // test online
    set_online(user, user->userId);
    printf("login success\n");
  }
}

void test_register(User *user) {
  user->userId = register_user(user);
  log_d("id: %lld", user->userId);
  if (user->userId == ERROR_USER_EXISTS) {
    printf("ussername exists.\n");
    return;
  }
  printf("register succeed, your userId is: %lld\n", user->userId);
}

void test_logout(User *user) {
  int rc = logout_user(user);
  check(rc != ERROR_NORMAL, "error");
  log_d("logout success.rc: %d", rc);

error:
  return;
}

void test_pchat(User *user1, User *user2) {
  assert(user1->userId);
  assert(user2->userId);
  LL chatId = create_chatroom(user1->userId, user2->userId);
  if (chatId == -1) {
    return;
  }
  printf("Enter chatroom[%lld]\n", chatId);

  Message *msg1 = new_messgae();
  msg1->groupId = chatId;
  msg1->sendTime = currentTimeMillis();
  strcpy(msg1->senderName, user1->username);
  strcpy(msg1->msg, "I love you, kwok?");
  LL id1 = save_message(chatId, msg1);
  if (id1 == -1) {
    printf("send msg1 error\n");
  } else {
    printf("msg1 sent, id is %lld\n", id1);
  }

  sleep(1);

  Message *msg2 = new_messgae();
  msg2->groupId = chatId;
  msg2->sendTime = currentTimeMillis();
  strcpy(msg2->senderName, user2->username);
  strcpy(msg2->msg, "hahaha)");
  LL id2 = save_message(chatId, msg2);
  if (id2 == -1) {
    printf("send msg1 error\n");
  } else {
    printf("msg1 sent, id is %lld\n", id2);
  }
  free(msg1);
  free(msg2);
}

void test_get_userid(char *username) {
  LL id = get_userid_by_name(username);
  if (id <= 0) {
    printf("error\n");
    return;
  }
  printf("userId: %lld\n", id);
}

void test_get_fds(LL chatroom_id) {
  size_t size;
  int *fds = get_fd_byid(chatroom_id, &size);
  log_d("size: %ld", size);

  if (fds == NULL || size == 0) {
    printf("nobody online\n");
    return;
  }

  for (size_t i = 0; i < size; i++) {
    printf("send message to %d.\n", fds[i]);
  }

  free(fds);
}

int main() {
  open_db(DB_IP, DB_PORT);

  // test client packet message
  Message *message = new_messgae();
  char format[MAX_MSG_DATA] = {0};
  sprintf(format, "%s %s %ld", "leer", "0000", currentTimeMillis());
  strcpy(message->msg, format);
  printf("format: %s\n", message->msg);

  Message *message1 = new_messgae();
  char format1[MAX_MSG_DATA] = {0};
  sprintf(format1, "%s %s %ld", "kwok", "1111", currentTimeMillis());
  strcpy(message1->msg, format1);
  printf("format: %s\n", message1->msg);

  Message *message2 = new_messgae();
  char format2[MAX_MSG_DATA] = {0};
  sprintf(format2, "%s %s %ld", "dong", "1111", currentTimeMillis());
  strcpy(message2->msg, format2);
  printf("format: %s\n", message2->msg);

  // test server unpacket message
  User *user = msg2user(message);
  User *user1 = msg2user(message1);
  User *user2 = msg2user(message2);
  // printf("username: %s\n", user->username);
  // printf("pass: %s\n", user->pass);
  // printf("registerTime: %ld\n", user->registerTime);

  user->lastOnlineTime = currentTimeMillis();
  user1->lastOnlineTime = currentTimeMillis();
  user2->lastOnlineTime = currentTimeMillis();

  // test_get_userid(user->username);//pass
  // del_chatrooom(4); // pass
  // test_login(user); // pass
  // test_login(user1);

  // test_get_fds(5);//pass
  // test_logout(user);//pass
  // test_logout(user1);

  // test_register(user2);//pass
  // test_register(user1);

  // test_pchat(user, user1); // pass
  // leave_chatrooom(2, "leer");

  // strcpy(user->username, "XIAO HONG");
  // strcpy(user->pass, "1111");
  // user->registerTime = currentTimeMillis();

  free(user);
  free(user1);
  free(user2);
  free(message);
  free(message1);
  free(message2);
  // test_wrapper();
  close_db();
}
