#include "../lib/config.h"
#include <event2/buffer.h>
#include <event2/bufferevent.h>
#include <event2/event.h>
#include <netinet/in.h>
#include <unistd.h>

void readcb(struct bufferevent *bev, void *arg) {
  char buf[1024];
  int n;

  struct evbuffer *input = bufferevent_get_input(bev);
  struct evbuffer *output = bufferevent_get_output(bev);

  printf("reading..\n");
  n = read(STDIN_FILENO, &buf, sizeof(buf));
  evbuffer_add(output, buf, n);
  // move sizeof(buf) Bytes from input to tail of buf..
  while ((n = evbuffer_remove(input, buf, sizeof(buf))) > 0) {
    fwrite(buf, 1, n, stdout);
  }
}

void writecb(struct bufferevent *bev, void *arg) {}

int main() {
  struct event_base *base;
  struct bufferevent *bev;

  base = event_base_new();
  struct sockaddr_in sin;
  socklen_t len;
  sin.sin_addr.s_addr = 0;
  sin.sin_family = AF_INET;
  sin.sin_port = htons(3477);
  len = sizeof(sin);

  bev = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE);
  bufferevent_setcb(bev, readcb, NULL, NULL, base);
  bufferevent_enable(bev, EV_READ | EV_WRITE);

  bufferevent_socket_connect(bev, (struct sockaddr *)&sin, len);

  event_base_dispatch(base);

  event_base_free(base);
}