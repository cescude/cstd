#include "std.h"

reader_t readInit(int fd, byte *buffer, size len) {
    return (reader_t){
        .fd = fd,
        .buffer = bufFromPtr(buffer, len),
        .it = (iter_t){0},
    };
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

    reader->it = it;
    reader->buffer = buf;

    if (iterDone(it)) {
        return 0;
    }

    return 1;
}
