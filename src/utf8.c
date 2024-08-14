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
str_t utf8DropChars(str_t s, size count) {
  for (; s.beg < s.end && 0 < count; count--) {
    s.beg += utf8BytesNeeded(*s.beg);
  }
  
  /*
    we don't ever want to adjust ptr beyond it's length, even when
    passed in an incorrectly aligned str.
  */
  
  if (s.end < s.beg) {
    s.beg = s.end;
  }

  return s;
}

size utf8BytesNeeded(char head) {
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

size utf8CharLen(utf8_char_t ch) {
  packable_char_t p = { .value = ch };
  return utf8BytesNeeded(p.bytes[0]);
}

size utf8StrLen(str_t s) {
  size count = 0;
  for (; s.beg < s.end; count++) {
    s.beg += utf8BytesNeeded(*s.beg);
  }
  return count;
}

utf8_char_t utf8FirstChar(str_t s) {
  if (s.beg >= s.end) {
    return 0;
  }

  size char_width = utf8BytesNeeded(*s.beg);
  if (char_width <= s.end - s.beg) {
    packable_char_t result = {0};
    memmove(result.bytes, s.beg, char_width);
    return result.value;
  }

  return 0;
}

utf8_char_t utf8CharAt(str_t s, size index) {
  s = utf8DropChars(s, index);
  return utf8FirstChar(s);
}

bool utf8CharEquals(utf8_char_t a, utf8_char_t b) {
  size sz_a = utf8CharLen(a);
  size sz_b = utf8CharLen(b);

  if (sz_a != sz_b) return 0;

  packable_char_t pa = { .value = a };
  packable_char_t pb = { .value = b };
  
  return memcmp(pa.bytes, pb.bytes, sz_a) == 0;
}

