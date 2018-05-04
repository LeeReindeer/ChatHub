#include "config.h"
#include <sys/time.h>

int64_t currentTimeMillis() {
  struct timeval time;
  gettimeofday(&time, NULL);
  int64_t s1 = (int64_t)(time.tv_sec) * 1000;
  int64_t s2 = (time.tv_usec / 1000);
  return s1 + s2;
}

User *new_user() {
  User *user = malloc(sizeof(User));
  memset(user, 0, sizeof(User));
  return user;
}

Message *new_messgae() {
  Message *m = malloc(sizeof(Message));
  memset(m, 0, sizeof(Message));
  return m;
}

Message *user2msg(User *user) {}

User *msg2user(Message *msg) {
  User *user = new_user();

  // unpaket message
  char username[MAX_CHARS] = {0}, pass[MAX_CHARS] = {0};
  int64_t registerTime;
  sscanf(msg->msg, "%s %s %ld", username, pass, &registerTime);

  strcpy(user->username, username);
  strcpy(user->pass, pass);
  user->registerTime = registerTime;

  return user;
}

// use for client
Message *register_message(char *username, char *pass, int64_t time) {
  Message *message = new_messgae();
  char format[MAX_MSG_DATA];
  sprintf(format, "%s %s %ld", "XIAOHONG", "1111", currentTimeMillis());
  strcpy(message->msg, format);
  message->type = REGISTER;
  strncpy(message->senderName, username, MAX_CHARS);
  message->sendTime = time;
}