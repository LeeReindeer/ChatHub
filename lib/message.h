#ifndef __MESSAGE_H__
#define __MESSAGE_H__

#include "../lib/config.h"

char *pack_message(Message *message);

Message *unpack_message(char *json);

#endif