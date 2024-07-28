#include "std.h"

str strTakeLine(str src) {
  return strTakeToChar(src, '\n');
}

str strTakeToChar(str src, char c) {
  str result = src;
  for (result.len = 0; result.len < src.len; result.len++) {
    if (src.ptr[result.len] == c) {
      break;
    }
  }
  return result;
}

str strFromBuf(buf buf) {
  return (str){buf.ptr, buf.len};
}

_Bool readToBufFromFile(buf *buf_p, int fd) {
  buf b = *buf_p;
  
  while (b.len < b.cap) {
    ssize_t bytes_read = read(fd, b.ptr + b.len, b.cap - b.len);

    if (bytes_read == 0 && buf_p->len == b.len) {
      return 0;	/* no bytes to read (eof) AND none read this go-round */
    } else if (bytes_read == 0) {
      break; /* ok, we got eof, however some data was read so allow the caller to process */
    }

    if (bytes_read < 0) {
      return 0;			/* error */
    }

    b.len += bytes_read;
  }

  *buf_p = b;
  return 1;
}

void bufDropBytes(buf *buf, ptrdiff_t sz) {
  sz = sz > buf->len ? buf->len : sz;
  sz = sz < 0 ? 0 : sz;
  memmove(buf->ptr, buf->ptr + sz, buf->len - sz);
  buf->len -= sz;
}

_Bool writeStrToFile(str s, int fd) {
  while (s.len > 0) {
    ssize_t sz = write(fd, s.ptr, s.len);

    if (sz < 0) {
      return 0;
    }

    s.ptr += sz;
    s.len -= sz;
  }

  return 1;
}

_Bool writeBufToFile(buf b, int fd) {
  return writeStrToFile(strFromBuf(b), fd);
}

_Bool writeCharToFile(char c, int fd) {
  str s = (str){&c, 1};
  return writeStrToFile(s, fd);
}
