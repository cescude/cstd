#pragma once

typedef struct {
    int fd;
    buf_t buffer;
    iter_t it;
} reader_t;

/* No such thing as an unbuffered reader */
reader_t readInit(int fd, byte *buffer, size len);

/*
  These advance the current iterator, filling unused data from the
  file as needed.
*/
bool readToStr(reader_t *reader, str_t separator);
bool readToAnyChar(reader_t *reader, str_t chars);
bool readSkipChars(reader_t *reader, size count);

bool readToBytes(reader_t *reader, bytes_t separator);
bool readToAnyByte(reader_t *reader, bytes_t bytes);
bool readSkipBytes(reader_t *reader, size count);

/* These access the value of the current iterator */
str_t readStr(reader_t reader);
bytes_t readBytes(reader_t reader);
void *readStruct(reader_t reader, size sizeof_struct);

bool readWasTruncated(reader_t reader);

