#include "std.h"

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

bool fdReadIntoBuf(int fd, buf_t *bufp) {
    buf_t b = *bufp;
  
    while (b.len < b.cap) {
        ssize_t bytes_read = read(fd, b.ptr + b.len, b.cap - b.len);

        if (bytes_read == 0) {
            bool any_read = bufp->len != b.len;
            *bufp = b;
            return any_read;
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

bool fdOpenReadOnly(str_t filename, int *fd_out) {
    return fdOpen(filename, fd_out, O_RDONLY);
}

bool fdOpen(str_t filename, int *fd_out, int flags) {
    size fn_len = bytesLen(bytesFromStr(filename));
    if (fn_len >= PATH_MAX-1) {
        return ENAMETOOLONG;
    }
        
    char zpath[PATH_MAX] = {0};
    memcpy(zpath, filename.beg, fn_len);

    int fd = open(zpath, flags);
    if (fd < 0) {
        return 0;
    }

    *fd_out = fd;
    return 1;
}

bool fdClose(int fd) {
    return close(fd) == 0;
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

bool fdPrintBytes(int fd, buf_t *buf, bytes_t bs) {
  return fdPrintBytesF(fd, buf, bs, (format_t){0});
}

bool fdPrintBytesF(int fd, buf_t *buf, bytes_t bs, format_t f) {
  /* no formatting options yet... */
  if (bytesIsEmpty(bs)) return 1;
  
  size bs_len = bytesLen(bs);
  char data[4] = {0};

  int len = snprintf(data, sizeof(data), "%xd", bs.beg[0]);
  if (!fdPrintStrF(fd, buf, (str_t){data, &data[len]}, f)) {
    return 0;
  }
  
  for (size i=1; i<bs_len; i++) {
    len = snprintf(data, sizeof(data), " %xd", bs.beg[i]);
    if (!fdPrintStrF(fd, buf, (str_t){data, &data[len]}, f)) {
      return 0;
    }
  }

  return 1;
}
