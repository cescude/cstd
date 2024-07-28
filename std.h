#pragma once

#include <fcntl.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>

/*
  Assume the str struct *doesn't* own its memory, ie., it's really a
  slice into something else (so we can narrow ptr/len without causing
  a memory leak).

  Assume the str points to r/o memory, so no modifications should be
  done in these str* functions.
*/
typedef struct {
  char *ptr;
  ptrdiff_t len;
} str;

#define strFromC(x) ((str){x, strlen(x)})

/*
  Return a new str, bounded by '\n' (or an arbitrary *other*
  character).

  If str doesn't contain the searched for character, the original str
  is returned.
 */
str strTakeLine(str src);
str strTakeToChar(str src, char c);

/*
  Assume that this structure is pointing at r/w data.
 */
typedef struct {
  char *ptr;
  ptrdiff_t len;
  ptrdiff_t cap;
} buf;

str strFromBuf(buf buf);

/*
  Shifts sz bytes off the front of buf.
*/
void bufDropBytes(buf *buf, ptrdiff_t sz);

/*
  Returns 1 if more data might exist
 */
_Bool readToBufFromFile(buf *buf, int fd);

_Bool writeStrToFile(str s, int fd);
_Bool writeBufToFile(buf b, int fd);
_Bool writeCharToFile(char c, int fd);
