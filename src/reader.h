#pragma once

typedef struct {
    int fd;
    iter_t it;
    buf_t buffer;
} reader_t;

/* No such thing as an unbuffered reader */
reader_t readInit(int fd, buf_t buf);

/*
  TODO:
  
  Would be nice if this could detect that `fd` is mmap'able & do that
  instead. However, would need to make *buffer NOT a pointer, which I
  guess we should do regardless?
*/
reader_t readFromFile(str_t filename, buf_t buf);
reader_t readFromFileHandle(int fd, buf_t buf);

/*
  These advance the current iterator, filling unused data from the
  file as needed.
*/
bool readToStr(reader_t *reader, str_t separator);
bool readToAnyChar(reader_t *reader, str_t chars);
bool readSkipChars(reader_t *reader, size count);

bool readToByte(reader_t *reader, byte b);
bool readToBytes(reader_t *reader, bytes_t separator);
bool readToAnyByte(reader_t *reader, bytes_t bytes);
bool readSkipBytes(reader_t *reader, size count);

/* These access the value of the current iterator */

str_t readStr(reader_t reader);
/* bytes_t readBytes(reader_t reader); */
/* void *readStruct(reader_t reader, size sizeof_struct); */

bool readWasTruncated(reader_t reader);

