#include "std.h"

reader_t readInit(int fd, buf_t buf) {
    return (reader_t){
        .fd = fd,
        .it = (iter_t){0},
        .buffer = buf,
        .mmap = 0,   /* set to non-zero to indicate a mmap'ed file */
    };
}

#include <sys/mman.h>
#include <sys/stat.h>

#define _isMemMapped(r) ((r)->mmap > 0)

bool readMmap(reader_t *rdr) {
    struct stat st = {0};
    
    if (fstat(rdr->fd, &st) < 0) {
        goto cannot_map;
    }
    
    void *ptr = mmap(0, st.st_size, PROT_READ, MAP_PRIVATE, rdr->fd, 0);
    if (ptr == MAP_FAILED) {
        goto cannot_map;
    }

    rdr->buffer = (buf_t){
        .ptr = ptr,
        .len = st.st_size, /* Data is loaded(ish), so set len to capacity */
        .cap = st.st_size
    };
    rdr->mmap = 1;
    rdr->it = iterFromBuf(rdr->buffer); /* we already have everything */
    
    return true;

cannot_map:
    return false;
}

bool readMunmap(reader_t *rdr) {
    return _isMemMapped(rdr) && (munmap(rdr->buffer.ptr, rdr->buffer.cap) == 0);
}

bool readWasTruncated(reader_t reader) {
    return
        reader.buffer.ptr == reader.it.beg &&
        reader.buffer.len == (reader.it.end - reader.it.beg);
}

str_t readStr(reader_t reader) {
    return iterStr(reader.it);
}

bool readToStr(reader_t *reader, str_t separator) {
    int fd = reader->fd;
    buf_t buf = reader->buffer;
    iter_t it = reader->it;

    bool has_token = iterTakeToStr(&it, separator);

    /*
      Memory mapped files don't have the potential to read more data :^(
     */
    if (!_isMemMapped(reader)) {
        if (!has_token || iterLast(it)) {
            if (it.beg > buf.ptr) {
                bufDropTo(&buf, it.beg);
            } else {
                bufClear(&buf);
            }
            if (fdReadIntoBuf(fd, &buf)) {
                it = iterFromBuf(buf);

                /*
                  Try a second time. if the buffer still wasn't large
                  enough, we'll get a partial token.
                */
            
                iterTakeToStr(&it, separator);
            }
        }
    }

    reader->it = it;
    reader->buffer = buf;

    if (iterDone(it)) {
        return 0;
    }

    return 1;
}

bool readToAnyChar(reader_t *reader, str_t chars) {
    int fd = reader->fd;
    buf_t buf = reader->buffer;
    iter_t it = reader->it;

    bool has_token = iterTakeToAnyChar(&it, chars);

    /*
      Memory mapped files don't have the potential to read more data :^(
     */
    if (!_isMemMapped(reader)) {
        if (!has_token || iterLast(it)) {
            if (it.beg > buf.ptr) {
                bufDropTo(&buf, it.beg);
            } else {
                bufClear(&buf);
            }
            if (fdReadIntoBuf(fd, &buf)) {
                it = iterFromBuf(buf);

                /*
                  Try a second time. if the buffer still wasn't large
                  enough, we'll get a partial token.
                */
            
                iterTakeToAnyChar(&it, chars);
            }
        }
    }

    reader->it = it;
    reader->buffer = buf;

    if (iterDone(it)) {
        return 0;
    }

    return 1;
}

bool readToByte(reader_t *reader, byte b) {
    int fd = reader->fd;
    buf_t buf = reader->buffer;
    iter_t it = reader->it;

    bool has_token = iterTakeToByte(&it, b);

    /*
      Memory mapped files don't have the potential to read more data :^(
     */
    if (!_isMemMapped(reader)) {
        if (!has_token || iterLast(it)) {
            if (it.beg > buf.ptr) {
                bufDropTo(&buf, it.beg);
            } else {
                bufClear(&buf);
            }
            if (fdReadIntoBuf(fd, &buf)) {
                it = iterFromBuf(buf);

                /*
                  Try a second time. if the buffer still wasn't large
                  enough, we'll get a partial token.
                */
            
                iterTakeToByte(&it, b);
            }
        }
    }

    reader->it = it;
    reader->buffer = buf;

    if (iterDone(it)) {
        return 0;
    }

    return 1;
}
