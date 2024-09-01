#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

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
#include "test.h"
#include "fd.h"
#include "reader.h"
#include "print.h"
#include "fmt.h"
#include "opt.h"

void die(char *msg);
void assert(bool t, char *fail_msg);
void errlog(str_t msg);
