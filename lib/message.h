#ifndef __MESSAGE_H__
#define __MESSAGE_H__

#include "../lib/config.h"

char *pack_message(Message *message, size_t *json_size);

Message *unpack_message(char *json);

void print_message(Message *message);

#endif