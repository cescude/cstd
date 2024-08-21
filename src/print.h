#pragma once

typedef struct {
  int fd;
  buf_t *buf;                   /* Points to NULL if unbuffered */
} print_t;

#define printInit(fd, buf)	      (print_t){(fd), (buf)}
#define printInitUnbuffered(fd)	      (print_t){(fd), NULL}
#define printInitNoFile(buf)	      (print_t){0, (buf)}
#define printC(p, cstr)		      printStr((p), strC(cstr))

bool printFlush(print_t p);	    /* flush out any internal state */
bool printStr(print_t p, str_t s);
bool printStrF(print_t p, str_t s, format_t f);
bool printChar(print_t p, utf8_char_t c);
bool printCharF(print_t p, utf8_char_t c, format_t f);

/* Maybe want more specialized functions, but this is ok for now */
bool printUnsigned(print_t p, uint64_t n);
bool printUnsignedF(print_t p, uint64_t s, format_t f);
bool printNum(print_t p, int64_t n);
bool printNumF(print_t p, int64_t s, format_t f);

bool printBytes(print_t p, bytes_t bs);
bool printBytesF(print_t p, bytes_t bs, format_t f);

