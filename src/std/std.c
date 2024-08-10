#include "std.h"

#include <string.h>
#include <sys/mman.h>

bool strEquals(str_t s, str_t t) {
  if (s.len != t.len) return 0;
  if (s.ptr == t.ptr) return 1;

  ptrdiff_t min_sz = s.len < t.len ? s.len : t.len;
  return memcmp(s.ptr, t.ptr, min_sz) == 0;
}

bool strStartsWith(str_t s, str_t prefix) {
  if (s.len < prefix.len) return 0;
  
  s.len = prefix.len;
  return strEquals(s, prefix);
}

// TODO tests
str_t strDropBytes(str_t s, ptrdiff_t count) {
  ptrdiff_t min_sz = count < s.len ? count : s.len;
  s.len -= min_sz;
  s.ptr += min_sz;
  return s;
}

str_t strDropChars(str_t s, ptrdiff_t count) {
  return utf8DropChars(s, count);
}

str_t strFirstLine(str_t src) {
  return strTakeToByte(src, '\n');
}

str_t strTakeToByte(str_t src, char c) {
  str_t result = src;
  for (result.len = 0; result.len < src.len; result.len++) {
    if (src.ptr[result.len] == c) {
      break;
    }
  }
  return result;
}

uint64_t strHash_djb2(str_t src) {
  /* http://www.cse.yorku.ca/~oz/hash.html */
  uint64_t hash = 5381;
  for (int i=0; i<src.len; i++) {
    hash = 33 * hash ^ src.ptr[i];
  }
  return hash;
}

// TODO tests
bool strMaybeParseInt(str_t s, int *result) {
  bool negative = 0;
  
  if (s.len == 0) return 0;

  if (s.ptr[0] == '-') {
    negative = 1;

    s.ptr++;
    s.len--;

    /* can't have just "-", need at least "-N" */
    if (s.len == 0) return 0;
  }

  int tmp = 0;
  for (ptrdiff_t i=0; i<s.len; i++) {

    if (s.ptr[i] < '0' || s.ptr[i] > '9') {
      return 0;
    }
    
    tmp *= 10;
    tmp += s.ptr[i] - '0';

    if (tmp < 0) {
      return 0;			/* overflow :^( */
    }
  }

  *result = negative ? -tmp : tmp;
  return 1;
}

void bufClear(buf_t *buf) {
  buf->len = 0;
}

void bufDrop(buf_t *buf, ptrdiff_t sz) {
  sz = sz > buf->len ? buf->len : sz;
  sz = sz < 0 ? 0 : sz;
  memmove(buf->ptr, buf->ptr + sz, buf->len - sz);
  buf->len -= sz;
}

str_t bufAppendStr(buf_t *bufp, str_t str) {
  buf_t b = *bufp;
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

_Bool fdReadIntoBuf(int fd, buf_t *bufp) {
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
  return (str_t){NULL, 0};
}

_Bool fdFlush(int fd, buf_t *bufp) {
  /* nothing to flush... */
  if (bufp == NULL || bufp->ptr == NULL || bufp->len == 0) {
    return 1;
  }
  
  _Bool result = 1;
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

  bufDrop(bufp, bufp->len - b.len); /* handle partial flush */
  return result;
}

_Bool fdPrintStr(int fd, buf_t *bufp, str_t s) {

  /* conditions for an unbuffered, direct write */
  if (bufp == NULL || bufp->ptr == NULL || bufp->cap == 0) {
    buf_t tmp = (buf_t){s.ptr, s.len, s.len};
    return fdFlush(fd, &tmp);
  }

  do {
    s = bufAppendStr(bufp, s);
    
    if (s.len == 0) {
      break;
    }

    if (!fdFlush(fd, bufp)) {
      return 0;
    }
  } while (1);

  /* all of s made it into buf */
  return 1;
}

_Bool fdPrintStrF(int fd, buf_t *buf, str_t s, format_t f) {
  _Bool result = 1;
  
  if (!f.right) {
    result = result && fdPrintStr(fd, buf, s);
  }
  
  for (ptrdiff_t i=0; i < f.width - s.len; i++) {
    result = result && fdPrintChar(fd, buf, ' ');
  }
  
  if (f.right) {
    result = result && fdPrintStr(fd, buf, s);
  }

  return result;
}

_Bool fdPrintChar(int fd, buf_t *buf, char c) {
  return fdPrintCharF(fd, buf, c, (format_t){0});
}

_Bool fdPrintCharF(int fd, buf_t *buf, char c, format_t f) {
  str_t s = (str_t){&c, 1};
  return fdPrintStrF(fd, buf, s, f);
}

_Bool fdPrintU64(int fd, buf_t *buf, uint64_t n) {
  return fdPrintU64F(fd, buf, n, (format_t){0});
}

_Bool fdPrintU64F(int fd, buf_t *buf, uint64_t n, format_t f) {
  char data[21] = {0};
  int len = snprintf(data, sizeof(data), "%lu", n);
  return fdPrintStrF(fd, buf, (str_t){data, len}, f);
}

_Bool printFlush(print_t p) {
  return fdFlush(p.fd, p.buf);
}

_Bool printStr(print_t p, str_t s) {
  return fdPrintStr(p.fd, p.buf, s);
}

_Bool printStrF(print_t p, str_t s, format_t f) {
  return fdPrintStrF(p.fd, p.buf, s, f);
}

_Bool printChar(print_t p, char c) {
  return fdPrintChar(p.fd, p.buf, c);
}

_Bool printCharF(print_t p, char c, format_t f) {
  return fdPrintCharF(p.fd, p.buf, c, f);
}

_Bool printU64(print_t p, uint64_t n) {
  return printU64F(p, n, (format_t){0});
}

_Bool printU64F(print_t p, uint64_t n, format_t f) {
  return fdPrintU64F(p.fd, p.buf, n, f);
}
