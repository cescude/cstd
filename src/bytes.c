#include <strings.h>
#include <string.h>

#include "std.h"

bytes_t bytesFromStr(str_t s) {
  return (bytes_t){s.beg, s.end};
}

bool bytesIntoStruct(bytes_t b, void *ptr, size ptr_sz) {
  if (bytesLen(b) < ptr_sz) return 0;
  memmove(ptr, b.beg, ptr_sz);
  return 1;
}

bytes_t bytesFromStruct(void *ptr, size ptr_sz) {
  return (bytes_t){(byte*)ptr, ((byte*)ptr) + ptr_sz};
}

size bytesLen(bytes_t b) {
  return b.end - b.beg;
}

bool bytesIsEmpty(bytes_t b) {
  return b.end <= b.beg;
}

bool bytesNonEmpty(bytes_t b) {
  return b.beg < b.end;
}

bool bytesEquals(bytes_t a, bytes_t b) {
  size a_len = bytesLen(a);
  size b_len = bytesLen(b);

  if (a_len != b_len) return 0;
  if (a_len == 0) return 1;	/* both are empty strings */
  if (a.beg == b.beg) return 1;	/* both point to the same memory */
  
  return bcmp(a.beg, b.beg, a_len) == 0;
}

bytes_t bytesDrop(bytes_t b, size count) {
  b.beg += count;
  if (b.beg > b.end) {
    b.beg = b.end;
  }
  return b;
}

bytes_t bytesTakeToByte(bytes_t b, byte v) {
  for (; b.beg < b.end && *b.beg != v; b.beg++);
  return b;
}

bytes_t bytesSkipByte(bytes_t b, byte v) {
  for (; b.beg < b.end && *b.beg == v; b.beg++);
  return b;
}
