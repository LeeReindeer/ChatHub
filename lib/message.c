#include "../lib/config.h"
#include "../lib/parson.h"

/**
 * @brief  packet struct Message in json format.The caller must free
 * serialied json string.
 * @param  *message: message to format.
 * @retval json string.
 */
char *pack_message(Message *message, size_t *json_size) {
  JSON_Value *root_value = json_value_init_object();
  JSON_Object *root_object = json_value_get_object(root_value);
  char *serialied_string = NULL;

  json_object_set_number(root_object, "msgId", message->msgId);
  json_object_set_number(root_object, "groupId", message->groupId);
  json_object_set_string(root_object, "senderName", message->senderName);
  json_object_set_number(root_object, "sendTime", message->sendTime);
  json_object_set_number(root_object, "type", message->type);
  json_object_set_string(root_object, "msg", message->msg);

  serialied_string = json_serialize_to_string_pretty(root_value);
  if (json_size) {
    *json_size = json_serialization_size_pretty(root_value);
  }
  // json_free_serialized_string(serialied_string);
  json_value_free(root_value);

  return serialied_string;
}

/**
 * @brief Persistence the json string to Message
 * @param  *json: json string
 * @retval Message struct
 */
Message *unpack_message(char *json) {

  Message *message = new_messgae();
  JSON_Value *root_value = json_parse_string(json);
  JSON_Object *root_object = json_value_get_object(root_value);

  message->msgId = (LL)json_object_get_number(root_object, "msgId");
  message->groupId = (LL)json_object_get_number(root_object, "groupId");
  message->sendTime = (int64_t)json_object_get_number(root_object, "sendTime");
  message->type =
      (MessageType)((int)json_object_get_number(root_object, "type"));

  strncpy(message->msg, json_object_get_string(root_object, "msg"),
          MAX_MSG_DATA);
  strncpy(message->senderName,
          json_object_get_string(root_object, "senderName"), MAX_CHARS);

  json_value_free(root_value);

  return message;
}

void print_message(Message *message) {
  char *pack = pack_message(message, NULL);
  printf("%s\n", pack);
  free(pack);
}