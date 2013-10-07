#include "cs_array.h"

CsArray* cs_array_new() {
  size_t i = 0;
  CsArray* arr = cs_alloc_object(CsArray);
  if (arr == NULL)
    cs_exit(CS_REASON_NOMEM);
  arr->size = 8;
  arr->length = 0;
  arr->buckets = malloc(sizeof(void*) * arr->size);
  if (arr->buckets == NULL)
    cs_exit(CS_REASON_NOMEM);
  for (i = 0; i < arr->size; i++)
    arr->buckets[i] = NULL;
  return arr;
}

void cs_array_free(CsArray* arr) {
  cs_free_object(arr->buckets);
  cs_free_object(arr);
}

bool cs_array_find(CsArray* arr, long pos, void* data) {
  long i = (pos < 0)? ((long) arr->length) + pos : pos;
  cs_return_if(i >= (long) arr->length || i < 0, false);
  data = arr->buckets + i;
  return true;
}

void cs_array_append(CsArray* arr, void* data) {

}

void cs_array_prepend(CsArray* arr, void* data) {

}

bool cs_array_insert(CsArray* arr, long pos, void* data) {
  return true;
}

bool cs_array_remove(CsArray* arr, long pos, void* data) {
  return true;
}

void cs_array_traverse(CsArray* arr, bool (*fn)(void*, void*), void* data) {

}