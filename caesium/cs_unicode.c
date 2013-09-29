#include "cs_unicode.h"

#define UTF8_COMPUTE(c, mask, len)  \
  if (c < 128) {                    \
    len = 1;                        \
    mask = 0x7f;                    \
  } else if ((c & 0xe0) == 0xc0) {  \
    len = 2;                        \
    mask = 0x1f;                    \
  } else if ((c & 0xf0) == 0xe0) {  \
    len = 3;                        \
    mask = 0x0f;                    \
  } else if ((c & 0xf8) == 0xf0) {  \
    len = 4;                        \
    mask = 0x07;                    \
  } else if ((c & 0xfc) == 0xf8) {  \
    len = 5;                        \
    mask = 0x03;                    \
  } else if ((c & 0xfe) == 0xfc) {  \
    len = 6;                        \
    mask = 0x01;                    \
  } else len = -1;

#define UTF8_GET(output, u8str, count, mask, len) \
  (output) = (u8str)[0] & (mask);                 \
  for ((count) = 1; (count) < (len); ++(count)) { \
    if (((u8str)[(count)] & 0xc0) != 0x80) {      \
      (output) = -1;                              \
      break;                                      \
    }                                             \
    (output) <<= 6;                               \
    (output) |= ((u8str)[(count)] & 0x3f);        \
  }

static const char utf8_skip_data[256] = {
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,
  3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,5,5,5,5,6,6,1,1
};

#define utf8_next(ptr) ((ptr) + utf8_skip_data[*(const unsigned char*)(ptr)])

size_t cs_utf8_strnlen(const char* u8str, ssize_t max) {
  size_t len = 0;
  const char* start = u8str;
  cs_return_unless(u8str != NULL || max == 0, 0);
  /*If the user chose to read until null*/
  if (max < 0) {
    while (*u8str) {
      u8str = utf8_next(u8str);
      ++len;
    }
  } else {
    cs_return_if(max == 0 || *u8str == 0, 0);
    u8str = utf8_next(u8str);
    while (u8str - start < max && *u8str) {
      u8str = utf8_next(u8str);
      ++len;
    }
    if (u8str - start <= max)
      ++len;
  }
  return len;
}

char* cs_utf8_strncpy(char* dest, const char* src, size_t n_chars) {
  const char* c = src;
  /* This walks until a NULL character is found */
  while (n_chars && *c) {
    c = utf8_next(c);
    n_chars--;
  }
  strncpy(dest, src, c - src);
  dest[c - src] = 0;
  return dest;
}

static char* strnstr(const char* haystack, const char* needle,
  size_t haystack_len) {
  char c, sc;
  size_t len;
  if ((c = *needle++) != '\0') {
    len = strlen(needle);
    do {
      do {
        if (haystack_len-- < 1 || (sc = *haystack++) == 0)
          return NULL;
      } while (sc != c);
      if (len > haystack_len)
        return NULL;
    } while (strncmp(haystack, needle, len) != 0);
    haystack--;
  }
  return (char*) haystack;
}

char* cs_utf8_strchr(const char* u8str, ssize_t max, CsUnichar c) {
  char buf[8];
  buf[cs_unichar_to_utf8(c, buf)] = 0;
  return strnstr(u8str, buf, max);
}

int cs_unichar_to_utf8(CsUnichar c, char* buffer) {
  uint32_t len = 0;
  int first;
  int i;
  if (c < 0x80) {
    first = 0;
    len = 1;
  } else if (c < 0x800) {
    first = 0xc0;
    len = 2;
  } else if (c < 0x10000) {
    first = 0xe0;
    len = 3;
  } else if (c < 0x200000) {
    first = 0xf0;
    len = 4;
  } else if (c < 0x4000000) {
    first = 0xf8;
    len = 5;
  } else {
    first = 0xfc;
    len = 6;
  }
  if (buffer != NULL) {
    for (i = len - 1; i > 0; --i) {
      buffer[i] = (c & 0x3f) | 0x80;
      c >>= 6;
    }
    buffer[0] = c | first;
  }
  return len;
}

char* cs_utf8_substr(const char* u8str, long start, long end) {
  char *first, *last, *output;
  first = cs_utf8_offset_to_pointer(u8str, start);
  last = cs_utf8_offset_to_pointer(u8str, end - start);
  output = malloc(last - first + 1);
  cs_return_unless(output, NULL);
  memcpy(output, first, last - first);
  output[last - first] = 0;
  return output;
}

char* cs_utf8_offset_to_pointer(const char* u8str, long offset) {
  const char* start = u8str;
  if (offset > 0) {
    while (offset--) {
      start = utf8_next(start);
    }
  } else {
    const char* s;
    while (offset) {
      s = start;
      start += offset;
      while ((*start & 0xc0) == 0x80) {
        s--;
      }
      offset += cs_utf8_pointer_to_offset(start, s);
    }
  }
  return (char*) start;
}

long cs_utf8_pointer_to_offset(const char* u8str, const char* c) {
  const char* start = u8str;
  long offset = 0;
  if (c < u8str) {
    offset = -cs_utf8_pointer_to_offset(c, u8str);
  } else {
    while (start < c) {
      start = utf8_next(start);
      offset++;
    }
  }
  return offset;
}

char* cs_utf8_next(const char* u8str, const char* end) {
  if (*u8str) {
    if (end) {
      for (++u8str; u8str < end && (*u8str & 0xc0) == 0x80; ++u8str);
    } else {
      for (++u8str; (*u8str & 0xc0) == 0x80; ++u8str);
    }
  }
  return (u8str == end)? NULL : (char*) u8str;
}