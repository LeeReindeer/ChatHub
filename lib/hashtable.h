#include <event2/bufferevent.h>
#ifndef __HASHTABLE_H__
#define __HASHTABLE_H__

typedef struct bufferevent *ValueType;

typedef struct HashNode {
  int key;
  ValueType value;
} HashNode;

typedef struct HashMap {
  struct HashNode **hashTable;
  int cap;
  int size;
} HashMap;

HashMap *hash_init(int cap);
void hash_destroy(HashMap *hm);

ValueType hash_get(HashMap *hm, int key);
void hash_set(HashMap *hm, int key, ValueType val);
ValueType hash_delete(HashMap *hm, int key);

#endif // __HASHTABLE_H__
