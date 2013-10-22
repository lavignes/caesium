#include "cs_mutator.h"
#include "cs_nursery.h"

CsNurseryPage* cs_nursery_new_page() {
  int i;
  CsNurseryPage* page = NULL;
  if (cs_unlikely(posix_memalign((void**) &page,
      sizeof(CsNurseryPage), sizeof(CsNurseryPage))))
    cs_exit(CS_REASON_NOMEM);
  // zero-out the page
  page->nvalues = 0;
  for (i = 0; i < CS_NURSERY_PAGE_MAX; i++) {
    page->bitmaps[i] = CS_NURSERY_UNUSED;
  }
  return page;
}

void cs_nursery_free_page(CsNurseryPage* page) {
  int i;
  for (i = 0; i < CS_NURSERY_PAGE_MAX; i++) {
    if (page->bitmaps[i] & CS_NURSERY_USED)
      cs_value_cleanup(page->values + i);
  }
  free(page);
}