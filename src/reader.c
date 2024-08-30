#include "std.h"

reader_t readInit(int fd, buf_t *buf) {
    return (reader_t){
        .fd = fd,
        .it = (iter_t){0},
        .buffer = buf,
    };
}

bool readWasTruncated(reader_t reader) {
    return
        reader.buffer->ptr == reader.it.beg &&
        reader.buffer->len == (reader.it.end - reader.it.beg);
}

str_t readStr(reader_t reader) {
    return iterStr(reader.it);
}

bool readToStr(reader_t *reader, str_t separator) {
    int fd = reader->fd;
    buf_t buf = *reader->buffer;
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
    *reader->buffer = buf;

    if (iterDone(it)) {
        return 0;
    }

    return 1;
}

bool readToAnyChar(reader_t *reader, str_t chars) {
    int fd = reader->fd;
    buf_t buf = *reader->buffer;
    iter_t it = reader->it;

    bool has_token = iterTakeToAnyChar(&it, chars);

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

    reader->it = it;
    *reader->buffer = buf;

    if (iterDone(it)) {
        return 0;
    }

    return 1;
}

bool readToByte(reader_t *reader, byte b) {
    int fd = reader->fd;
    buf_t buf = *reader->buffer;
    iter_t it = reader->it;

    bool has_token = iterTakeToByte(&it, b);

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

    reader->it = it;
    *reader->buffer = buf;

    if (iterDone(it)) {
        return 0;
    }

    return 1;
}
