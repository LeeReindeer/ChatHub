#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <event2/event.h>
#include <event2/listener.h>
#include <event2/util.h>

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#ifndef _WIN32
#include <netinet/in.h>
#ifdef _XOPEN_SOURCE_EXTENDED
#include <arpa/inet.h>
#endif
#include <sys/socket.h>
#endif

#include "../lib/config.h"
#include "../lib/dbg.h"
#include "../lib/message.h"
#include "dao.h"

/*
* main server
*/

// todo use hashtable instead
struct bufferevent
    *clients[1024]; /* use fd as index, get event at O(1) time.So the max only
                       user number is 1024.*/
void connectClient(struct bufferevent *bev);
void disconnectClient(struct bufferevent *bev);

/* dispatch messages to following func*/
void handleAllMessage(struct bufferevent *from, Message *message);
int handleRegister(struct bufferevent *this, Message *message);
int handleLogin(struct bufferevent *this, Message *message);
int handleLogout(struct bufferevent *this, Message *message);
int handleChat(struct bufferevent *from, Message *message);

void initDB();
void initEvent();

void handleAllMessage(struct bufferevent *from, Message *message) {
  int type = message->type;
  log_d("recv msg type: %s", message_types[type]);

  switch (type) {
  case LOGIN:
    handleLogin(from, message);
    break;
  case LOGOUT:
    handleLogout(from, message);
  case REGISTER:
    handleRegister(from, message);
    break;
  case TEXT:
  case IMAGE:
    handleChat(from, message);
    break;
  default:
    break;
  }
}

int handleRegister(struct bufferevent *this, Message *message) {
  User *user = msg2user(message);
  user->userId = register_user(user);
  int rc = user->userId;
  message->type = REGISTER;
  char id[MAX_MSG_DATA];
  sprintf(id, "%lld", user->userId);
  strcpy(message->msg, id);

  char *json = NULL;
  json = pack_message(message);

  bufferevent_write(this, json, MAX_BUFF);

  free(json);
  free(user);
  return rc;
}

int handleLogin(struct bufferevent *this, Message *message) {
  User *user = msg2user(message);
  user->userId = login_user(user);
  int rc = user->userId;
  message->type = LOGIN;
  char *json = NULL;
  char id[MAX_MSG_DATA];

  sprintf(id, "%lld", user->userId);
  strcpy(message->msg, id);
  json = pack_message(message);
  if (rc > 0) { // vaild user id
    // set user sockfd
    set_online(user, bufferevent_getfd(this));
  }
  bufferevent_write(this, json, MAX_BUFF);

  free(json);
  free(user);

  return rc;
}

int handleLogout(struct bufferevent *this, Message *message) {
  User *user = msg2user(message);
  user->userId = logout_user(user);
  int rc = user->userId;
  message->type = LOGOUT;
  char *json = NULL;
  char id[MAX_MSG_DATA];

  sprintf(id, "%lld", user->userId);
  strcpy(message->msg, id);
  json = pack_message(message);

  bufferevent_write(this, json, MAX_BUFF);

  free(json);
  free(user);

  return rc;
}

int handleChat(struct bufferevent *from, Message *message) {
  struct evbuffer *input;
  input = bufferevent_get_input(from);
  size_t size = 0;
  int *fds = get_fd_byid(message->groupId, &size);
  struct bufferevent **recv_bevs =
      (struct bufferevent **)calloc(size, sizeof(struct bufferevent *));

  for (size_t i = 0; i < size; i++) {
    recv_bevs[i] = clients[fds[i]];
    // send msg to all group memebers
    if (recv_bevs[i]) {
      evbuffer_add_buffer(bufferevent_get_output(recv_bevs[i]), input);
    } else {
      // set unread message
    }
    save_message(message->groupId, message);
  }

  free(fds);
  // struct bufferevent *receiver_bev = clients[this_fd]; // itself
  // if (receiver_bev)
  // { // "online" record in runtime
  //   log_d("write to client: %d\n", bufferevent_getfd(receiver_bev));
  //   log_d("Start handle message...");
  //   handleAllMessage(bev, message);
  //   // output = bufferevent_get_output(receiver_bev);
  //   // evbuffer_add_buffer(output, input);
  //   // bufferevent_flush(receiver_bev, EV_WRITE, BEV_NORMAL);
  // }
  // else
  // {
  //   // todo save only chat msg to db, and set unsent key
  //   bufferevent_write(bev, "error", 6);
  //   bufferevent_flush(bev, EV_WRITE, BEV_NORMAL);
  //   log_d("user not online, just save msg to db...");
  // }
  // get fd from redis("online" record on db)
  // evutil_socket_t fd =
  //     get_fd_byid(message->groupId); // this only for personal chat
  // struct bufferevent *receiver_bev = clients[fd];
}

void connectClient(struct bufferevent *bev) {
  clients[bufferevent_getfd(bev)] = bev;
}

void disconnectClient(struct bufferevent *bev) {
  clients[bufferevent_getfd(bev)] = NULL;
}

void readcb(struct bufferevent *bev, void *arg) {
  struct evbuffer *input, *output;
  char buf[MAX_BUFF];
  int n;

  input = bufferevent_get_input(bev);
  n = evbuffer_get_length(input);
  log_d("n: %d", n);
  if (n < MAX_BUFF) {
    log_d("drop small message");
    return;
  }

  while ((n = evbuffer_remove(input, buf, MAX_BUFF)) > 0) {
  }

  Message *message = unpack_message(buf);
  if (!strcmp(message->msg, "")) {
    log_d("drop empty message");
    return;
  }
  log_d("recv msg: %s", message->msg);

  // debug
  int this_fd = bufferevent_getfd(bev);
  log_d("read from fd: %d", this_fd);

  if (this_fd != -1) {
    handleAllMessage(bev, message);
  } else {
    // send error
  }
}
// void writecb(struct bufferevent *bev, void *arg) {}

void eventcb(struct bufferevent *bev, short events, void *arg) {
  if (events && (BEV_EVENT_EOF | BEV_EVENT_ERROR)) {
    struct event_base *base = arg;
    if (events & BEV_EVENT_EOF) {
      printf("server: client %d closed.\n", bufferevent_getfd(bev));
    } else if (events & BEV_EVENT_ERROR) {
      printf("Got an error on the connection: %s\n",
             strerror(errno)); /*XXX win32*/
      printf("Closing server...\n");
      bufferevent_free(bev);
      event_base_loopbreak(base);
    }
    disconnectClient(bev);
  }
}

void listener_cb(struct evconnlistener *listener, evutil_socket_t fd,
                 struct sockaddr *sa, int socklen, void *arg) {
  struct event_base *base = arg;
  struct bufferevent *bev;

  bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
  if (!bev) {
    printf("server: can't make bufferevent\n");
    event_base_loopbreak(base);
  }
  // set online user to redis here
  printf("server: new client: %d connected\n", fd);

  // record bufferevent
  connectClient(bev);

  bufferevent_setcb(bev, readcb, NULL, eventcb, base);
  bufferevent_enable(bev, EV_READ | EV_WRITE);
}

void listener_errorcb(struct evconnlistener *listener, void *arg) {
  struct event_base *base = arg;
  event_base_loopbreak(base);
}

static void signal_cb(evutil_socket_t sig, short events, void *user_data) {
  struct event_base *base = user_data;
  struct timeval delay = {2, 0};

  log_d("Caught an interrupt signal; exiting cleanly in two seconds");

  event_base_loopexit(base, &delay);
}

void initDB() { open_db(DB_IP, DB_PORT); }

void initEvent() {
  struct event_base *base;
  struct event *signal_event;
  struct evconnlistener *listener;
  struct sockaddr_in sin;

  base = event_base_new();
  check(base, "base error");

  memset(&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_addr.s_addr = 0;
  sin.sin_port = htons(PORT);

  // the sockfd in listener is default
  // nonblocking.To make bloking: LEV_OPT_LEAVE_SOCKETS_BLOCKING,
  listener = evconnlistener_new_bind(base, listener_cb, base,
                                     LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE,
                                     16, (struct sockaddr *)&sin, sizeof(sin));
  check(listener, "listen error");
  evconnlistener_set_error_cb(listener, listener_errorcb);

  signal_event = evsignal_new(base, SIGINT, signal_cb, (void *)base);
  check(signal_event && event_add(signal_event, NULL) >= 0,
        "Could not create/add a signal event!");

  event_base_dispatch(base);

error: // fallthrough
  event_base_free(base);
  evconnlistener_free(listener);
  close_db();
}

int main() {
  initDB();
  initEvent();
}