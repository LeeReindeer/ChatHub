#include "dao.h"

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

void test_login(User *user) {
  int rc = login_user(user);
  printf("result: %d\n", rc);
  if (rc == 0) {
    printf("login success\n");
  }
}

void test_register(User *user) {
  user->userId = register_user(user);
  if (user->userId == -1) {
    printf("error\n");
    return;
  }
  printf("register succeed, your userId is: %lld\n", user->userId);
}

void test_logout(User *user) {
  int rc = logout_user(user);
  check(rc != -1, "error");
  check(rc != -3, "user not exsits");
  check(rc != -4, "already logout");
  log_d("rc: %d", rc);

error:
  return;
}

void test_pchat(User *user1, User *user2) {
  LL chatId = create_chatroom(user1->username, user2->username);
  if (chatId == -1) {
    log_d("can't create room");
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
  sprintf(format1, "%s %s %ld", "kwok", "0000", currentTimeMillis());
  strcpy(message1->msg, format1);
  printf("format: %s\n", message1->msg);

  // test server unpacket message
  User *user = msg2user(message);
  User *user1 = msg2user(message1);
  // printf("username: %s\n", user->username);
  // printf("pass: %s\n", user->pass);
  // printf("registerTime: %ld\n", user->registerTime);

  user->lastOnlineTime = currentTimeMillis();
  user1->lastOnlineTime = currentTimeMillis();
  // test_login(user);
  // test_login(user1);
  test_logout(user1);
  // test_logout(user1);
  // test_register(user);
  // test_register(user1);
  // test_pchat(user, user1);
  // leave_chatrooom(2, "leer");
  // strcpy(user->username, "XIAO HONG");
  // strcpy(user->pass, "1111");
  // user->registerTime = currentTimeMillis();

  free(user);
  free(user1);
  free(message);
  free(message1);
  // test_wrapper();
  close_db();
}
