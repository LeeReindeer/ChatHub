#include "hashtable.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * @brief Hash table base on (positive) integer key, use simple mod hash
 * function,
 * and linear probing for collision.
 * provide inset, search, delete function.
 */

HashMap *hash_init(int cap) {
  HashMap *hm = malloc(sizeof(HashMap));
  hm->hashTable = calloc(cap, sizeof(HashNode *));
  hm->size = 0;
  hm->cap = cap;
  return hm;
}

void hash_destroy(HashMap *hm) {
  int i = 0;
  int cap = hm->cap;
  HashNode **ht = hm->hashTable;
  HashNode *hn;

  for (i = 0; i < cap; i++) {
    if ((hn = ht[i])) {
      free(hn);
    }
  }

  free(ht);
  free(hm);
}

HashNode *find(HashMap *hm, int key) {
  HashNode *node;
  HashNode **ht = hm->hashTable;
  int cap = hm->cap;
  int h = abs(key) % cap;

  while ((node = ht[h])) {
    if (node->key != key) {
      if (h < cap - 1) {
        h++; // h = (h + 1) mod cap;
      } else {
        h = 0;
      }
    } else {
      return node;
    }
  }

  return NULL;
}

ValueType hash_get(HashMap *hm, int key) {
  HashNode *node = find(hm, key);
  return node == NULL ? 0 : node->value;
}

void hash_set(HashMap *hm, int key, ValueType value) {
  if (hm->size >= hm->cap) {
    printf("[ERROR] HashTable overflow.\n");
    return;
  }
  HashNode **ht = hm->hashTable;
  HashNode *node;
  int cap = hm->cap;
  int h = abs(key) % cap; // sample hash function

  while (ht[h]) {
    if (ht[h]->key == key) { // update value
      ht[h]->value = value;
      return;
    }
    if (h < cap - 1) {
      h++;
    } else {
      h = 0;
    }
  }

  node = malloc(sizeof(HashNode));
  node->key = key;
  node->value = value;
  ht[h] = node;
  hm->size++;
}

ValueType hash_delete(HashMap *hm, int key) {
  // HashNode *node = find(hm, key);
  // if (node != NULL) {
  //   ValueType value = node->value;
  //   free(node);
  //   hm->size--;
  //   return value;
  // }
  HashNode *node;
  HashNode **ht = hm->hashTable;
  int cap = hm->cap;
  int h = abs(key) % cap;

  while ((node = ht[h])) {
    if (node->key != key) {
      if (h < cap - 1) {
        h++;
      } else {
        h = 0;
      }
    } else {
      ValueType value = node->value;
      free(node);
      ht[h] = NULL;
      hm->size--;
      return value;
    }
  }

  // return (ValueType)NULL;
  return 0;
}

/**
 * @brief  unit test
 */
/*
int main(int argc, char const *argv[]) {
  HashMap *map = hash_init(5);
  hash_set(map, 1, 'a');
  hash_set(map, 1, 'b'); // update test
  hash_set(map, 2, 'd');
  hash_set(map, 2, 'c');
  hash_set(map, 3, 'e');
  hash_set(map, 4, 'e');
  hash_set(map, 5, 'F');
  hash_set(map, 6, 'E'); // overflow test

  printf("size: %d\n", map->size);
  hash_delete(map, 2);
  printf("after delete, size: %d\n", map->size);

  printf("1: %c\n", hash_get(map, 1));
  printf("2: %c\n", hash_get(map, 2));
  hash_destroy(map);
  return 0;
}
*/