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

/*
  Assume that this structure is pointing at r/w data.
 */
typedef struct {
  char *ptr;
  ptrdiff_t len;
  ptrdiff_t cap;
} buf;

str strFromC(char *cstr);
str strFromBuf(buf buf);

/*
  Return a new str, bounded by '\n' (or an arbitrary *other*
  character).

  If src doesn't contain the searched for character, the original str
  is returned.
 */
str strFirstLine(str src);
str strTakeToChar(str src, char c);

/*
  Shifts sz bytes off the front of buf.

  If before the buffer looks like...
  
    { [A,B,C,D,_], 4, 5 }

  ...and you call bufDrop(&b, 2), after it will look like...

    { [C,D,_,_,_], 2, 5 }
  
*/
void bufDrop(buf *buf, ptrdiff_t sz);

 /*
   Returns str w/ any data not appended, so if the returned str has
   len == 0, you know the full input was appended.
 */
str bufAppendStr(buf *buf, str str);

/*
  Fills data from fd into buf, starting at buf.len (ie., it appends
  file data to the end of the buffer).

  Returns 1 if more data might exist in the file, 0 if the squeeze has
  been squozen.

  If buf's capacity and length are equal (ie., you're asking to put
  data with no free space), behavior is undefined (ie., I haven't
  decided what I want it to do yet).
 */
_Bool fdFillBuf(int fd, buf *buf);
_Bool fdMemMap(int fd, str *s);

_Bool fdFlush(int fd, buf *buf);
_Bool fdPrintStr(int fd, buf *buf, str s);
_Bool fdPrintChar(int fd, buf *buf, char c);
