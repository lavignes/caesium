#include <xxhash.h>

#include "cs_common.h"
#include "cs_hash.h"
#include "cs_unicode.h"

static const void* DUMMY = (void*) 0x4;

void hash_double(CsHash* hash) {
  CsPair** old_buckets = hash->buckets;
  size_t old_size = hash->size;
  size_t i;
  uint32_t pos;
  CsPair* pair;
  hash->size *= 2;
  hash->high_load = (hash->size >> 2) * 3;
  hash->buckets = malloc(sizeof(CsPair*) * hash->size);
  if (hash->buckets == NULL)
    cs_exit(CS_REASON_NOMEM);
  for (i = 0; i < old_size; i++) {
    pair = old_buckets[i];
    if (pair != NULL && pair != DUMMY) {
      pos = pair->hash % hash->size;
      while (hash->buckets[pos] != NULL) {
        pos = (pos == hash->size)? 0 : pos + 1;
      }
      hash->buckets[pos] = pair;
    }
  }
}

CsHash* cs_hash_new() {
  size_t i;
  CsHash* hash = cs_alloc_object(CsHash);
  if (hash == NULL)
    cs_exit(CS_REASON_NOMEM);
  hash->size = 8;
  hash->length = 0;
  hash->high_load = (hash->size >> 2) * 3;
  hash->buckets = malloc(sizeof(CsPair*) * hash->size);
  if (hash->buckets == NULL)
    cs_exit(CS_REASON_NOMEM);
  for (i = 0; i < hash->size; i++)
    hash->buckets[i] = NULL;
  return hash;
}

CsPair* cs_hash_insert(
  CsHash* hash,
  const char* key,
  size_t key_len,
  void* value)
{
  uint32_t h = XXH32(key, key_len, 0xdeadface);
  uint32_t i = h % hash->size;
  CsPair* pair = hash->buckets[i];
  while (pair != NULL) {
    if (pair != DUMMY && h == pair->hash && strcmp(key, pair->u8key) == 0) {
      pair->value = value;
      return pair;
    }
    i = (i == hash->size)? 0 : i + 1;
    pair = hash->buckets[i];
  }
  pair = cs_alloc_object(CsPair);
  if (pair == NULL)
    cs_exit(CS_REASON_NOMEM);
  pair->u8key = malloc(key_len);
  if (pair->u8key == NULL)
    cs_exit(CS_REASON_NOMEM);
  memcpy(pair->u8key, key, key_len);
  pair->key_len = key_len;
  pair->hash = h;
  pair->value = value;
  hash->buckets[i] = pair;
  hash->length++;
  if (hash->length >= hash->high_load) {
    hash_double(hash);
  }
  return pair;
}

CsPair* cs_hash_remove(CsHash* hash, const char* key, size_t key_len) {
  uint32_t h = XXH32(key, key_len, 0xdeadface);
  uint32_t i = h % hash->size;
  CsPair* pair = hash->buckets[i];
  while (pair != NULL) {
    if (pair != DUMMY && h == pair->hash && strcmp(key, pair->u8key) == 0) {
      hash->buckets[i] = (void*) DUMMY;
      return pair;
    }
    i = (i == hash->size)? 0 : i + 1;
    pair = hash->buckets[i];
  }
  return NULL;
}

CsPair* cs_hash_find(CsHash* hash, const char* key, size_t key_len) {
  uint32_t h = XXH32(key, key_len, 0xdeadface);
  uint32_t i = h % hash->size;
  CsPair* pair = hash->buckets[i];
  while (pair != NULL) {
    if (pair != DUMMY && h == pair->hash && strcmp(key, pair->u8key) == 0) {
      return pair;
    }
    i = (i == hash->size)? 0 : i + 1;
    pair = hash->buckets[i];
  }
  return NULL;
}

void cs_hash_traverse(CsHash* hash, bool (*fn)(CsPair*, void*), void* data) {

}

void cs_hash_free(CsHash* hash) {

}

void cs_pair_free(CsPair* pair) {
  cs_free_object(pair->u8key);
  cs_free_object(pair);
}