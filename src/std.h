#pragma once

#include <fcntl.h>
#include <stdbool.h>
#include <stddef.h>
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

typedef struct {
  int width;
  bool right;
} format_t;

#define strC(cstr)                    (str_t){cstr, &cstr[sizeof(cstr)-1]}
#define strFromBuf(buf)		      (str_t){(buf).ptr, (buf).ptr + (buf).len}
#define bytesC(cbytes)		      (bytes_t){cbytes, &(cbytes)[sizeof(cbytes)]}
#define bytesFromBuf(buf)	      (bytes_t){(buf).ptr, (buf).ptr + (buf).len}
#define bufFromPtr(ptr, sz)	      (buf_t){ptr, 0, sz}
#define bufFromArray(arr)	      (buf_t){arr, 0, sizeof(arr)}
#define printerFromFile(fd, buf)      (print_t){fd, buf}

str_t strFromC(char *cstr);
bool strNextChar(str_t *s);

size strLen(str_t s);

bool strIsEmpty(str_t s);
bool strNonEmpty(str_t s);

bool strEquals(str_t s, str_t t);
bool strStartsWith(str_t s, str_t prefix);

str_t strAppend(buf_t *b, str_t s);

str_t strDropChars(str_t s, size count);

utf8_char_t strFirstChar(str_t s);

str_t strFirstLine(str_t src);

str_t strTakeToChar(str_t src, utf8_char_t c);

str_t strTrim(str_t haystack, str_t needles);
str_t strTrimLeft(str_t haystack, str_t needles);
str_t strTrimRight(str_t haystack, str_t needles);
str_t strTakeLineWrapped(str_t text, size cols);

uint64_t strHash_djb2(str_t src);

bool strMaybeParseInt(str_t s, int *result);

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
void bufDropBytes(buf_t *buf, size sz);

/*
  Returns str w/ any data not appended, so if the returned str has
  len == 0, you know the full input is in buf.

  The second version is utf8 aware and won't copy a partial character
  into buf.
*/
str_t bufAppendBytes(buf_t *buf, str_t str); // TODO: move to bytes 
str_t bufAppendStr(buf_t *buf, str_t str);

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

bool fdFlush(int fd, buf_t *buf);
bool fdPrintStr(int fd, buf_t *buf, str_t s);
bool fdPrintStrF(int fd, buf_t *buf, str_t s, format_t f);
bool fdPrintChar(int fd, buf_t *buf, utf8_char_t c);
bool fdPrintCharF(int fd, buf_t *buf, utf8_char_t c, format_t f);
bool fdPrintU64(int fd, buf_t *buf, uint64_t n);
bool fdPrintU64F(int fd, buf_t *buf, uint64_t s, format_t f);
bool fdPrintI64(int fd, buf_t *buf, int64_t n);
bool fdPrintI64F(int fd, buf_t *buf, int64_t s, format_t f);

bool printFlush(print_t p); 	/* flush out any internal state */
bool printStr(print_t p, str_t s);
bool printStrF(print_t p, str_t s, format_t f);
bool printChar(print_t p, utf8_char_t c);
bool printCharF(print_t p, utf8_char_t c, format_t f);
bool printU64(print_t p, uint64_t n);
bool printU64F(print_t p, uint64_t s, format_t f);
bool printI64(print_t p, int64_t n);
bool printI64F(print_t p, int64_t s, format_t f);

/* typedef struct { */
/*   int fd; */
/*   buf_t buffer; */
/*   str_t cursor; */
/* } reader_t; */

/* #define readerFromC(fd, x) (reader_t){fd, bufFromArray(x), (str_t){x, x}} */

/* reader_t readerFromBuf(int fd, buf_t b); /\* No such thing as an unbuffered reader *\/ */
/* str_t readerTakeLine(reader_t *r); */
/* str_t readerPeekLine(reader_t r); */


#include "bytes.h"
#include "utf8.h"
#include "opt.h"

