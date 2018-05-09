#include "config.h"
#include <sys/time.h>

const char *message_types[] = {
    "TEXT",        "IMAGE",      "LOGIN",
    "REGISTER",    "LOGOUT",     "LIST_CHAT",
    "LIST_FRIEND", "ADD_FRIEND", "ADD_FRIEND_TO_GROUP",
    "DEL_FRIEND",  "ADD_GROUP",  "DEL_GROUP",
    "SNY_MSG",     "DEL_MSG",    "EXIT"};

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

Message *user2msg(User *user) {
  // check tregister time and username
  if (!user->registerTime || !strcmp("", user->username)) {
    return NULL;
  }
  Message *message = new_messgae();
  char format[MAX_MSG_DATA] = {0};
  sprintf(format, "%s %s %ld", user->username, user->pass, user->registerTime);
  strcpy(message->msg, format);
  return message;
}

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