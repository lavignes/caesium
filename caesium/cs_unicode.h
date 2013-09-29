#ifndef _CS_UNICODE_H_
#define _CS_UNICODE_H_

#include "cs_common.h"

typedef uint32_t CsUnichar;

/**
 * Compute string length in characters, excluding the null terminator.
 * @param  u8str pointer to utf8 string
 * @param  max   maximum number of bytes to examine. If `max` < 0, then the
 *               string must be null terminated. If `max` > 0, `max` bytes will
 *               be examined.
 * @return       the length of the string in characters
 */
size_t cs_utf8_strnlen(const char* u8str, ssize_t max);

/**
 * Like strncpy but using utf8 characters rather than bytes.
 * @param  dest    buffer to copy `n_chars` characters into
 * @param  src     a utf8 string
 * @param  n_chars number of characters to copy
 * @return         `dest`
 */
char* cs_utf8_strncpy(char* dest, const char* src, size_t n_chars);

/**
 * Find leftmost occurance of character in a utf8 string.
 * @param  u8str a null-terminated utf8 string to search
 * @param  max   maximum length of `u8str`
 * @param  c     unicode character to search for
 * @return       NULL if the character cant be found, otherwise, a pointer to
 *               the character
 */
char* cs_utf8_strchr(const char* u8str, ssize_t max, CsUnichar c);

/**
 * Convert a unicode character to a utf8 character
 * @param  c      a unicode character to convert
 * @param  buffer a buffer to fill with utf8 data, should be at least 6 bytes.
 *                If `buffer` is NULL, then just the number of bytes that would
 *                have been written will be returned.
 * @return        number of bytes written
 */
int cs_unichar_to_utf8(CsUnichar c, char* buffer);

/**
 * Copy a substring out of a utf8 string
 * @param  u8str a utf8 string
 * @param  start starting character offset
 * @param  end   ending character offset
 * @return       a newly allocated substring
 */
char* cs_utf8_substr(const char* u8str, long start, long end);

/**
 * Get character pointer at offset in a utf8 string
 * @param  u8str  a utf8 string
 * @param  offset character offset (can be negative for reverse offset)
 * @return        pointer to character at that offset
 */
char* cs_utf8_offset_to_pointer(const char* u8str, long offset);

/**
 * Given a character pointer, get its offset in a utf8 string
 * @param  u8str a utf8 string
 * @param  c     a pointer in the string
 * @return       the character offset
 */
long cs_utf8_pointer_to_offset(const char* u8str, const char* c);

/**
 * Find the next character in a utf8 string.
 * @param  u8str a utf8 string
 * @param  end   end of utf8 string or NULL if it is NULL-terminated
 * @return       a pointer to the next character or NULL
 */
char* cs_utf8_next(const char* u8str, const char* end);

#endif /* _CS_UNICODE_H_ */