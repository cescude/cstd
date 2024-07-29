#pragma once

#include <fcntl.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

/*
  Assume the str_t struct *doesn't* own its memory, ie., it's really a
  slice into something else (so we can narrow ptr/len without causing
  a memory leak).

  Assume the str_t points to r/o memory, so no modifications should be
  done in these str* functions.
*/
typedef struct {
  char *ptr;
  ptrdiff_t len;
} str_t;

/*
  Assume that this structure is pointing at r/w data, that it doesn't own.
 */
typedef struct {
  char *ptr;
  ptrdiff_t len;
  ptrdiff_t cap;
} buf_t;

typedef struct {
  int fd;
  buf_t *buf;			/* NULL if unbuffered */
} print_t;

str_t strFromC(char *cstr);
str_t strFromBuf(buf_t buf);

/*
  Return a new str, bounded by '\n' (or an arbitrary *other*
  character).

  If src doesn't contain the searched for character, the original str
  is returned.
 */
str_t strFirstLine(str_t src);
str_t strTakeToChar(str_t src, char c);

uint64_t strHash_djb2(str_t src);

#define bufFromArray(x) (bufFromPtr(x, sizeof(x)))
buf_t bufFromPtr(char *data, ptrdiff_t data_size);

/*
  Basically just sets len to 0.
 */
void bufClear(buf_t *buf);

/*
  Shifts sz bytes off the front of buf.

  If before the buffer looks like...
  
    { [A,B,C,D,_], 4, 5 }

  ...and you call bufDrop(&b, 2), after it will look like...

    { [C,D,_,_,_], 2, 5 }
  
*/
void bufDrop(buf_t *buf, ptrdiff_t sz);

 /*
   Returns str_t w/ any data not appended, so if the returned str_t has
   len == 0, you know the full input was appended.
 */
str_t bufAppendStr(buf_t *buf, str_t str);

/*
  Fills data from fd into buf, starting at buf.len (ie., it appends
  file data to the end of the buffer).

  Returns 0 if the file is eof (or errors) and there's no data in buf.

  If buf's capacity and length are equal (ie., you're asking to put
  data with no free space), behavior is undefined (ie., I haven't
  decided what I want it to do yet).
 */
_Bool fdReadIntoBuf(int fd, buf_t *buf);
str_t fdMemMap(int fd, str_t *s);

_Bool fdFlush(int fd, buf_t *buf);
_Bool fdPrintStr(int fd, buf_t *buf, str_t s);
_Bool fdPrintChar(int fd, buf_t *buf, char c);

print_t printerFromFile(int fd, buf_t *buf);
_Bool printStr(print_t p, str_t s);
_Bool printChar(print_t p, char c);
_Bool printU64(print_t p, uint64_t n);
_Bool printFlush(print_t p); 	/* flush out any internal state */
