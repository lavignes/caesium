#ifndef _CS_NURSERY_
#define _CS_NURSERY_

#include "cs_value.h"

#define CS_NURSERY_PAGE_SIZE 16384
#define CS_NURSERY_PAGE_MAX 496

typedef enum CsNurseryFlags {
  CS_NURSERY_UNUSED      = 0x00,
  // These flags are set on creation and are given a value equal to the
  // epoch of it's parent mutator. This prevents temporary values from
  // being freed during garbage collection.
  // Specifically, if a value's epoch is the same as that of it's mutator,
  // then it cannot possibly be freed
  CS_NURSERY_EPOCH       = 0x0F,


  CS_NURSERY_USED        = 0x10, 
  CS_NURSERY_MARK        = 0x20,
} CsNurseryFlags;

typedef struct CsNurseryPage {

  uint8_t bitmaps[CS_NURSERY_PAGE_MAX];
  uint64_t padding;
  uint64_t padding2;
  CsValueStruct values[CS_NURSERY_PAGE_MAX];

} CsNurseryPage;

CsNurseryPage* cs_nursery_new_page();

void cs_nursery_free_page(CsNurseryPage* page);

#endif /* _CS_NURSERY_ */
