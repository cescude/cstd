#include <stdlib.h>

#include "std.h"

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
