#ifndef _CS_LIST_H_
#define _CS_LIST_H_

#include "cs_common.h"

typedef struct CsList {

  size_t length;
  struct CsListNode* head;
  struct CsListNode* tail;

} CsList;

CsList* cs_list_new();

void cs_list_free(CsList* list);

void cs_list_push_back(CsList* list, void* data);

void* cs_list_pop_back(CsList* list);

void cs_list_push_front(CsList* list, void* data);

void* cs_list_pop_front(CsList* list);

void* cs_list_peek_back(CsList* list);

void* cs_list_peek_front(CsList* list);

void cs_list_traverse(CsList* list, bool(*fn)(void*, void*), void* data);

#endif /* _CS_LIST_H_ */