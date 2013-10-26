#ifndef _CS_NURSERY_
#define _CS_NURSERY_

#include "cs_value.h"

#define CS_NURSERY_PAGE_SIZE 16384
#define CS_NURSERY_PAGE_MAX 496

typedef enum CsNurseryFlags {
  CS_NURSERY_UNUSED      = 0x00,
  CS_NURSERY_POINTER     = 0x01, 
  CS_NURSERY_EPOCH       = 0x02,
  CS_NURSERY_MARK        = 0x04,
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
