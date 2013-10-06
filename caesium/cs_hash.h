#ifndef _CS_HASH_H_
#define _CS_HASH_H_

#include <stdbool.h>

typedef struct CsPair {
  char* u8key;
  void* value;
  size_t key_len;
  uint32_t hash;
} CsPair;

typedef struct CsHash {
  size_t size;
  size_t length;
  size_t high_load;
  size_t low_load;
  CsPair** buckets;
} CsHash;

CsHash* cs_hash_new();

/**
 * Insert a value into a hash table
 * @param  hash    a hash table
 * @param  key     a null-terminated key. The key is copied
 * @param  key_len size of key excluding null terminator
 * @param  value   a value to pair to the key
 * @return         a reference to the key-value pair storing the value
 */
CsPair* cs_hash_insert(
  CsHash* hash,
  const char* key,
  size_t key_len,
  void* value);

CsPair* cs_hash_remove(CsHash* hash, const char* key, size_t key_len);

CsPair* cs_hash_find(CsHash* hash, const char* key, size_t key_len);

void cs_hash_traverse(CsHash* hash, bool (*fn)(CsPair*, void*), void* data);

void cs_hash_free(CsHash* hash);

void cs_pair_free(CsPair* pair);

#endif /* _CS_HASH_H_ */