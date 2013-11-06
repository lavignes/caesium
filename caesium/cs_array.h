#ifndef _CS_ARRAY_H_
#define _CS_ARRAY_H_

#include "cs_common.h"

typedef struct CsArray {
  size_t size;
  size_t length;
  void** buckets;
} CsArray;

CsArray* cs_array_new();

void cs_array_free(CsArray* arr);

/**
 * Retreive element in array
 * @param  arr  an array
 * @param  pos  a index in the array. The index can be negative.
 * @param  data output location of data
 * @return      true if pos is valid, false if illegal access
 */
bool cs_array_find(CsArray* arr, long pos, void** data);

bool cs_array_set(CsArray* arr, long pos, void* data);

bool cs_array_insert(CsArray* arr, long pos, void* data);

CsArray* cs_array_copy(CsArray* arr);

bool cs_array_remove(CsArray* arr, long pos, void* data);

void cs_array_traverse(CsArray* arr, bool (*fn)(void*, void*), void* data);

#endif /* _CS_ARRAY_H_ */
