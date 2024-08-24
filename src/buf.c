#include "std.h"

#include <stdlib.h>

void bufClear(buf_t *buf) {
  buf->len = 0;
}

void bufDropTo(buf_t *buf, char *ptr) {
    if (ptr < buf->ptr) {
        return;
    }
    bufDropBytes(buf, ptr - buf->ptr);
}

void bufDropBytes(buf_t *buf, size sz) {
  sz = sz > buf->len ? buf->len : sz;
  sz = sz < 0 ? 0 : sz;
  memmove(buf->ptr, buf->ptr + sz, buf->len - sz);
  buf->len -= sz;
}

str_t bufAppendBytes(buf_t *bufp, str_t str) {
  size str_len = str.end - str.beg;
  
  buf_t b = *bufp;
  size free = b.cap - b.len;
  size sz = str_len < free ? str_len : free;
  
  memmove(b.ptr + b.len, str.beg, sz);

  /* update buffer len to reflect new data */
  b.len += sz;
  *bufp = b;

  /* shift str position to reflect any uncopied data */
  str.beg += sz;
  return str;
}

/* Treats str as utf8, won't copy partial characters */
str_t bufAppendStr(buf_t *bufp, str_t str) {
  size str_len = str.end - str.beg;
  
  buf_t b = *bufp;
  ptrdiff_t free = b.cap - b.len;
  ptrdiff_t sz = str_len < free ? str_len : free;

  if (sz < str_len) {
    /*
      Hit the buffer capacity, so we're copying a partial string. Need
      to ensure the final character is valid...
    */

    while ((str.beg[sz] & 0xC0) == 0x80) {
      sz--;
    }

    /* str.beg[sz] should now point to a valid utf8-char start */
  }
  
  memmove(b.ptr + b.len, str.beg, sz);

  /* update buffer len to reflect new data */
  b.len += sz;
  *bufp = b;

  /* shift str position to reflect any uncopied data */
  str.beg += sz;
  return str;
}

void *bufAllocCount(buf_t *buf, size sz, size align, size count) {
    void *out;
    if (bufAllocTest(buf, &out, sz, align, count)) {
        return out;
    } else {
        fprintf(stderr,
                "Failed allocation buf{cap=%lu}, sz=%lu, align=%lu, count=%lu\n",
                buf->cap, sz, align, count);
        fflush(stderr);
        abort();
    }
}

bool bufAllocTest(buf_t *buf, void **out, size sz, size align, size count) {
    char *buf_end = buf->ptr + buf->len;
    char *mem_start = buf_end;

    if ((uintptr_t)buf_end & (align-1)) {
        mem_start += align;
        mem_start = (char*)(((uintptr_t)mem_start) & ~((uintptr_t)align-1));
    }

    char *mem_end = mem_start + sz * count;

    if (mem_end > (buf->ptr + buf->cap)) {
        return false;
    }

    buf->len = mem_end - buf->ptr;
    *out = (void*)mem_start;
    return true;
}

