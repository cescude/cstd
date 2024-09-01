#pragma once

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
bool fdOpenReadOnly(str_t filename, int *fd_out);
int fdClose(int fd);

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
