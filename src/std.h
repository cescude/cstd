#pragma once

#include <fcntl.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

typedef ptrdiff_t size;

#define countof(x) (size)(sizeof(x)/sizeof(x[0]))

/*
  Assume the str_t struct *doesn't* own its memory, ie., it's really a
  slice into something else (so we can narrow without causing a memory
  leak).

  Assume the str_t points to r/o memory, so no modifications should be
  done in these str* functions.
*/
typedef struct {
  char *beg;
  char *end;
} str_t;

/* For working with utf8 characters */
typedef uint32_t utf8_char_t;

typedef char byte;

typedef struct {
  byte *beg;
  byte *end;
} bytes_t;

typedef struct {
  int width;
  bool right;
} format_t;

#define strC(cstr)                    (str_t){cstr, &cstr[sizeof(cstr)-1]}
#define strFromBuf(buf)		      (str_t){(buf).ptr, (buf).ptr + (buf).len}
#define bytesC(cbytes)		      (bytes_t){cbytes, &(cbytes)[sizeof(cbytes)]}
#define bytesFromBuf(buf)	      (bytes_t){(buf).ptr, (buf).ptr + (buf).len}

#include "str.h"
#include "bytes.h"
#include "buf.h"
#include "iter.h"
#include "utf8.h"
#include "opt.h"
#include "test.h"
#include "reader.h"
#include "print.h"

/*
  Fills data from fd into buf, starting at buf.len (ie., it appends
  file data to the end of the buffer).

  Returns 0 if the file is eof (or errors) and there's no data in buf.

  If buf's capacity and length are equal (ie., you're asking to put
  data with no free space), behavior is undefined (ie., I haven't
  decided what I want it to do yet).
*/
bool fdReadIntoBuf(int fd, buf_t *buf);
str_t fdMemMap(int fd);

bool fdOpen(str_t filename, int *fd_out, int flags);
int fdCLose(int fd);

bool fdFlush(int fd, buf_t *buf);
bool fdPrintStr(int fd, buf_t *buf, str_t s);
bool fdPrintStrF(int fd, buf_t *buf, str_t s, format_t f);
bool fdPrintChar(int fd, buf_t *buf, utf8_char_t c);
bool fdPrintCharF(int fd, buf_t *buf, utf8_char_t c, format_t f);
bool fdPrintU64(int fd, buf_t *buf, uint64_t n);
bool fdPrintU64F(int fd, buf_t *buf, uint64_t s, format_t f);
bool fdPrintI64(int fd, buf_t *buf, int64_t n);
bool fdPrintI64F(int fd, buf_t *buf, int64_t s, format_t f);
bool fdPrintBytes(int fd, buf_t *buf, bytes_t bs);
bool fdPrintBytesF(int fd, buf_t *buf, bytes_t bs, format_t f);
