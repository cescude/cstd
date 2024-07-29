#include "std.h"

_Bool fdFillBuf(int fd, buf *bufp) {
  buf b = *bufp;
  
  while (b.len < b.cap) {
    ssize_t bytes_read = read(fd, b.ptr + b.len, b.cap - b.len);

    if (bytes_read == 0 && bufp->len == b.len) {
      /* no bytes to read (eof) AND none read AT ALL this go-round */
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

_Bool fdMemMap(int fd, str *s) {
  /* TODO */
  return 0;
}

_Bool fdFlush(int fd, buf *bufp) {
  _Bool result = 1;
  buf b = *bufp;
  
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

_Bool fdPrintStr(int fd, buf *bufp, str s) {

  /* conditions for an unbuffered, direct write */
  if (bufp == NULL || bufp->ptr == NULL || bufp->cap == 0) {
    buf tmp = (buf){s.ptr, s.len, s.len};
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

_Bool fdPrintChar(int fd, buf *buf, char c) {
  str s = (str){&c, 1};
  return fdPrintStr(fd, buf, s);
}
