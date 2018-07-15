#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <event2/event.h>
#include <event2/util.h>

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#ifndef _WIN32
// For inet_addr
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#endif

#include "../lib/config.h"
#include "../lib/dbg.h"
#include "../lib/message.h"

typedef struct {
  struct bufferevent *bev;
  User *user;
  int is_login;
} Client;

static Client client = {0};

Message *user_msg(char *username, char *pass, MessageType type) {
  Message *message = new_messgae();
  char format[MAX_MSG_DATA];
  sprintf(format, "%s %s %ld", username, pass, currentTimeMillis());
  strncpy(message->msg, format, MAX_MSG_DATA);
  strncpy(message->senderName, username, MAX_CHARS);
  // 1 for login, 0 for register
  message->type = type;
  message->sendTime = currentTimeMillis();

  return message;
}

Message *chat_msg(char *sender_name, char *msg, LL group_id) {
  Message *message = new_messgae();
  strcpy(message->msg, msg);
  strncpy(message->senderName, sender_name, MAX_CHARS);
  // 1 for login, 0 for register
  message->type = TEXT;
  message->sendTime = currentTimeMillis();
  message->groupId = group_id;

  return message;
}

char *json_msg(Message *msg, size_t *size) { return pack_message(msg, size); }

void print_help() {
  printf("Commands:\n1->Login\n2->Register\n[chatroom id]:message\n");
}

void signal_cb(evutil_socket_t sig, short events, void *arg) {
  struct event_base *base = arg;
  struct timeval delay = {1, 0};

  log_d("Caught an interrupt signal; exiting cleanly in one seconds");

  event_base_loopexit(base, &delay);
}

void stdin_cb(int fd, short events, void *arg) {
  char buf[MAX_BUFF] = {0};

  int chatroom_id;
  size_t json_size;
  char *json_str;
  char msg[MAX_MSG_DATA] = {0};
  Message *message;

  int n = read(fd, buf, MAX_BUFF);
  buf[strcspn(buf, "\n")] = 0; // eat "\n"
  if (client.is_login) {
    sscanf(buf, "%d:%s", &chatroom_id, msg);
    message = chat_msg(client.user->username, msg, chatroom_id);
    json_str = json_msg(message, &json_size);

    bufferevent_write(client.bev, json_str, json_size);

    // clear
    free(message);
    free(json_str);
  } else {
    int op = atoi(buf);
    char username[MAX_CHARS] = {0};
    char pass[MAX_CHARS] = {0};
    char *password;

    switch (op) {
    case 1:
      printf("Username(less than 20 words):\n");
      n = read(fd, username, MAX_CHARS);
      username[strcspn(username, "\n")] = 0;
      password = getpass("Password:\n");
      strncpy(pass, password, MAX_CHARS);
      username[strcspn(pass, "\n")] = 0;
      message = user_msg(username, pass, LOGIN);
      json_str = json_msg(message, &json_size);

      // set client user
      strncpy(client.user->username, username, MAX_CHARS);
      strncpy(client.user->pass, pass, MAX_CHARS);

      bufferevent_write(client.bev, json_str, json_size);

      free(message);
      free(json_str);
      break;
    default:
      printf("Input error\n");
      print_help();
      break;
    }
  }
}

void event_cb(struct bufferevent *bev, short events, void *arg) {
  if (events & BEV_EVENT_CONNECTED) {
    log_d("Connetced to server");
    print_help();
  } else if (events && (BEV_EVENT_EOF | BEV_EVENT_ERROR)) {
    struct event_base *base = arg;
    if (events & BEV_EVENT_EOF) {
      log_w("Server closed");
    } else if (events & BEV_EVENT_ERROR) {
      log_e("Got an error on the connection: %s\n", strerror(errno));
    }
    // exit
    event_base_loopbreak(base);
  }
}

void read_cb(struct bufferevent *bev, void *arg) {
  // bufferevent_disable(bev, EV_WRITE);
  char buf[MAX_BUFF] = {0};
  int n = bufferevent_read(bev, buf, MAX_BUFF);
  Message *message = unpack_message(buf);
  LL user_id;
  // LL chatroom_id;
  // LL msg_id;
  char sender_name[MAX_CHARS] = {0};

  switch (message->type) {
  case LOGIN:
    if ((user_id = atoll(message->msg)) > 0) { // get use rid
      client.user->userId = user_id;
      client.is_login = 1;
      printf("Welcome!!!\nYou're login as %s\n", client.user->username);
    } else {
      printf("Login error!\n");
      memset(client.user->username, 0, MAX_CHARS);
      memset(client.user->pass, 0, MAX_CHARS);
    }
    break;
  case TEXT:
    printf("%ld %lld:%s:%s\n", message->sendTime, message->groupId,
           message->senderName, message->msg);
    break;
  default:
    break;
  }
  // log_d("%s", buf);
  free(message);
  // bufferevent_enable(bev, EV_WRITE);
}

int main(int argc, char const *argv[]) {

  struct event_base *base;
  struct event *ev_stdin;
  struct event *ev_signal;
  struct bufferevent *bev;
  struct sockaddr_in sin;

  if (argc != 3) {
    fprintf(stderr, "Usage: %s <IP> <port>\n", argv[0]);
    goto error;
  }

  base = event_base_new();
  check(base, "can't create base event");

  bev = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE);
  check(bev, "can't create bufferevent");

  memset(&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = inet_addr(argv[1]);
  sin.sin_port = htons(atoi(argv[2]));

  // connect
  int rt =
      bufferevent_socket_connect(bev, (struct sockaddr *)&sin, sizeof(sin));
  check(rt >= 0, "connect error");

  // init client
  client.bev = bev;
  client.user = new_user();
  client.is_login = 0;

  bufferevent_setcb(bev, read_cb, NULL, event_cb, (void *)base);
  bufferevent_enable(bev, EV_READ | EV_WRITE);

  ev_stdin =
      event_new(base, fileno(stdin), EV_READ | EV_PERSIST, stdin_cb, &client);
  event_add(ev_stdin, NULL);

  ev_signal = evsignal_new(base, SIGINT, signal_cb, (void *)base);
  check(ev_signal && event_add(ev_signal, NULL) >= 0,
        "Could not create/add a signal event!");
  event_base_dispatch(base);

  bufferevent_free(bev);
  event_free(ev_stdin);
  event_free(ev_signal);
  event_base_free(base);
  return 1;
error: // fall
  exit(1);
  return 1;
}
