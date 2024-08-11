#include <stdlib.h>

#include "std.h"

typedef union {
  uint32_t value;
  char bytes[4];
} packable_char_t;

utf8_char_t utf8CharFromC(char c) {
  packable_char_t ch = {
    .bytes = {c, 0, 0, 0},
  };
  return (utf8_char_t)ch.value;
}

// NOTE: write tests
str_t utf8DropChars(str_t s, ptrdiff_t count) {
  ptrdiff_t idx = 0;
  for (; idx < s.len && 0 < count; count--) {
    idx += utf8BytesNeeded(s.ptr[idx]);
  }

  s.len -= idx;
  s.ptr += idx;

  /* we don't ever want to adjust ptr beyond it's length, even when
     passed in an incorrectly aligned str */
  if (s.len < 0) {
    s.ptr += s.len;
    s.len = 0;
  }

  return s;
}

ptrdiff_t utf8CharLen(utf8_char_t ch) {
  packable_char_t p = { .value = ch };
  return utf8BytesNeeded(p.bytes[0]);
}

ptrdiff_t utf8StrLen(str_t s) {
  ptrdiff_t count = 0;
  for (ptrdiff_t i=0; i<s.len; count++) {
    i += utf8BytesNeeded(s.ptr[i]);
  }
  return count;
}


ptrdiff_t utf8BytesNeeded(char head) {
  if ((head & 0x80) == 0) {
    return 1;
  } else if ((head & 0xC0) == 0x80) {
    abort();	 /* head is in the middle of a multi-byte character */
  } else if ((head & 0xE0) == 0xC0) {
    return 2;
  } else if ((head & 0xF0) == 0xE0) {
    return 3;
  } else if ((head & 0xF8) == 0xF0) {
    return 4;
  }

  /* Unsure what we were passed here :grimace: */
  abort();
}

bool utf8NextChar(str_t s, ptrdiff_t *ch_idx, ptrdiff_t *idx) {
  ptrdiff_t i = *idx + utf8BytesNeeded(s.ptr[*idx]);

  if (i < s.len) {
    *idx = i;

    if (ch_idx != NULL) {
      *ch_idx += 1;
    }
    
    return 1;
  }

  /* there is NO next character! */
  return 0;
}

utf8_char_t utf8FirstChar(str_t s) {
  if (s.len < 1) {
    return 0;
  }
  
  packable_char_t result = {0};
  memmove(result.bytes, s.ptr, utf8BytesNeeded(s.ptr[0]));
  return result.value;
}

utf8_char_t utf8CharAt(str_t s, ptrdiff_t index) {
  s = utf8DropChars(s, index);
  return utf8FirstChar(s);
}

bool utf8CharEquals(utf8_char_t a, utf8_char_t b) {
  ptrdiff_t sz_a = utf8CharLen(a);
  ptrdiff_t sz_b = utf8CharLen(b);

  if (sz_a != sz_b) return 0;

  packable_char_t pa = { .value = a };
  packable_char_t pb = { .value = b };
  return memcmp(pa.bytes, pb.bytes, sz_a) == 0;
}

