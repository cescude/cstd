#include "std.h"

void bufDrop(buf *buf, ptrdiff_t sz) {
  sz = sz > buf->len ? buf->len : sz;
  sz = sz < 0 ? 0 : sz;
  memmove(buf->ptr, buf->ptr + sz, buf->len - sz);
  buf->len -= sz;
}

str bufAppendStr(buf *bufp, str str) {
  buf b = *bufp;
  ptrdiff_t free = b.cap - b.len;
  ptrdiff_t sz = str.len < free ? str.len : free;
  
  memmove(b.ptr + b.len, str.ptr, sz);

  /* update buffer len to reflect new data */
  b.len += sz;
  *bufp = b;

  /* shift str position to reflect any uncopied data */
  str.ptr += sz;
  str.len -= sz;
  return str;
}
