#include "std.h"

#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

/* static print_t POUT = printerFromFile(1, NULL); */
/* static print_t PERR = printerFromFile(2, NULL); */

void bufClear(buf_t *buf) {
  buf->len = 0;
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

bool fdReadIntoBuf(int fd, buf_t *bufp) {
  buf_t b = *bufp;
  
  while (b.len < b.cap) {
    ssize_t bytes_read = read(fd, b.ptr + b.len, b.cap - b.len);

    if (bytes_read == 0 && b.len == 0) {
      /* no bytes to read (eof) AND nothing for the caller to process */
      return 0;
    } else if (bytes_read == 0) {
      /*
	ok, we got eof, however some data was read so we're going to
	indicate success here, and let the next invocation fail.
      */
      break;
    }

    if (bytes_read < 0) {
      return 0;			/* error */
    }

    b.len += bytes_read;
  }

  *bufp = b;
  return 1;
}

str_t fdMemMap(int fd) {
  /* TODO */
  abort();
  return (str_t){NULL, 0};
}

bool fdFlush(int fd, buf_t *bufp) {
  /* nothing to flush... */
  if (bufp == NULL || bufp->ptr == NULL || bufp->len == 0) {
    return 1;
  }

  if (fd == 0) {
    /*
      Not attached to an output handle, so we're just using these
      functions for their formatting. Not sure how much I like this
      solution, it's kind of hacky...
    */
    return 0;
  }
  
  bool result = 1;
  buf_t b = *bufp;
  
  while (b.len > 0) {
    ssize_t sz = write(fd, b.ptr, b.len);
    
    if (sz < 0) {
      result = 0;
      break;
    }

    b.ptr += sz;
    b.len -= sz;
  }

  bufDropBytes(bufp, bufp->len - b.len); /* handle partial flush */
  return result;
}

bool fdPrintStr(int fd, buf_t *bufp, str_t s) {

  /* conditions for an unbuffered, direct write */
  if (bufp == NULL || bufp->ptr == NULL || bufp->cap == 0) {
    buf_t tmp = (buf_t){
      s.beg,
      s.end - s.beg,
      s.end - s.beg
    };
    return fdFlush(fd, &tmp);
  }

  do {
    s = bufAppendStr(bufp, s);
    
    if (strIsEmpty(s)) {
      break;
    }

    if (!fdFlush(fd, bufp)) {
      return 0;
    }
  } while (1);

  /* all of s made it into buf */
  return 1;
}

bool fdPrintStrF(int fd, buf_t *buf, str_t s, format_t f) {
  bool result = 1;
  
  if (!f.right) {
    result = result && fdPrintStr(fd, buf, s);
  }

  size s_len = s.end - s.beg;
  for (size i=0; i < f.width - s_len; i++) {
    result = result && fdPrintStr(fd, buf, strC(" "));
  }
  
  if (f.right) {
    result = result && fdPrintStr(fd, buf, s);
  }

  return result;
}

bool fdPrintChar(int fd, buf_t *buf, utf8_char_t c) {
  return fdPrintCharF(fd, buf, c, (format_t){0});
}

bool fdPrintCharF(int fd, buf_t *buf, utf8_char_t c, format_t f) {
  str_t s = (str_t){
    .beg = (char*)(&c),
    .end = (char*)(&c) + utf8CharLen(c),
  };
  return fdPrintStrF(fd, buf, s, f);
}

bool fdPrintU64(int fd, buf_t *buf, uint64_t n) {
  return fdPrintU64F(fd, buf, n, (format_t){0});
}

bool fdPrintU64F(int fd, buf_t *buf, uint64_t n, format_t f) {
  char data[21] = {0};
  int len = snprintf(data, sizeof(data), "%lu", n);
  return fdPrintStrF(fd, buf, (str_t){data, data + len}, f);
}

bool fdPrintI64(int fd, buf_t *buf, int64_t n) {
  return fdPrintI64F(fd, buf, n, (format_t){0});
}

bool fdPrintI64F(int fd, buf_t *buf, int64_t n, format_t f) {
  char data[21] = {0};
  int len = snprintf(data, sizeof(data), "%ld", n);
  return fdPrintStrF(fd, buf, (str_t){data, data + len}, f);
}

bool printFlush(print_t p) {
  return fdFlush(p.fd, p.buf);
}

bool printStr(print_t p, str_t s) {
  return fdPrintStr(p.fd, p.buf, s);
}

bool printStrF(print_t p, str_t s, format_t f) {
  return fdPrintStrF(p.fd, p.buf, s, f);
}

bool printChar(print_t p, utf8_char_t c) {
  return fdPrintChar(p.fd, p.buf, c);
}

bool printCharF(print_t p, utf8_char_t c, format_t f) {
  return fdPrintCharF(p.fd, p.buf, c, f);
}

bool printU64(print_t p, uint64_t n) {
  return printU64F(p, n, (format_t){0});
}

bool printU64F(print_t p, uint64_t n, format_t f) {
  return fdPrintU64F(p.fd, p.buf, n, f);
}

bool printI64(print_t p, int64_t n) {
  return printI64F(p, n, (format_t){0});
}

bool printI64F(print_t p, int64_t n, format_t f) {
  return fdPrintI64F(p.fd, p.buf, n, f);
}
