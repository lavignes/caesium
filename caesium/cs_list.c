#include "cs_list.h"

typedef struct CsListNode {

  void* data;
  struct CsListNode* next;
  struct CsListNode* prev;

} CsListNode;

CsList* cs_list_new() {
  CsList* list = cs_alloc_object(CsList);
  if (cs_unlikely(list == NULL))
    cs_exit(CS_REASON_NOMEM);
  list->length = 0;
  list->head = list->tail = NULL;
  return list;
}

void cs_list_free(CsList* list) {
  CsListNode* node = list->head;
  CsListNode* temp;
  while (node != NULL) {
    temp = node->next;
    cs_free_object(node);
    node = temp;
  }
  cs_free_object(list);
}

void cs_list_push_back(CsList* list, void* data) {
  list->length++;
  CsListNode* node = cs_alloc_object(CsListNode);
  if (cs_unlikely(node == NULL))
    cs_exit(CS_REASON_NOMEM);
  node->data = data;
  node->next = NULL;
  node->prev = list->tail;
  if (list->tail != NULL) {
    list->tail->next = node;
  }
  if(list->head == NULL)
    list->head = node;
  list->tail = node;
}

void* cs_list_pop_back(CsList* list) {
  void* data = NULL;
  CsListNode* node = list->tail;
  if (node != NULL) {
    list->length--;
    if (list->tail->prev != NULL) {
      list->tail = node->prev;
      list->tail->next = NULL;
    } else {
      list->head = list->tail = NULL;
    }
    data = node->data;
    cs_free_object(node);
  }
  return data;
}

void cs_list_push_front(CsList* list, void* data) {
  list->length++;
  CsListNode* node = cs_alloc_object(CsListNode);
  if (cs_unlikely(node == NULL))
    cs_exit(CS_REASON_NOMEM);
  node->data = data;
  node->prev = NULL;
  if (list->head != NULL) {
    node->next = list->head;
    list->head->prev = node;
  }
  list->head = node;
  if (list->tail == NULL)
    list->tail = node;
}

void* cs_list_pop_front(CsList* list) {
  CsListNode* node = list->head;
  void* data = NULL;
  if (list->head != NULL) {
    list->length--;
    if (list->head->next != NULL) {
      list->head = list->head->next;
      list->head->prev = NULL;
    } else {
      list->head= NULL;
      list->tail = NULL;
    }
    data = node->data;
    cs_free_object(node);
  }
  return data;
}

void* cs_list_peek_back(CsList* list) {
  if (cs_likely(list->tail))
    return list->tail->data;
  return NULL;
}

void* cs_list_peek_front(CsList* list) {
  if (cs_likely(list->head))
    return list->head->data;
  return NULL;
}

void cs_list_traverse(CsList* list, bool(*fn)(void*, void*), void* data) {
  CsListNode* node = list->head;
  while (node != NULL) {
    if (fn(node->data, data))
      break;
    node = node->next;
  }
}
