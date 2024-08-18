#include "std.h"

iter_t iterFromBytes(bytes_t bs) {
    return (iter_t){
        .beg = bs.beg,
        .end = bs.beg,
        .tail = bs.end,
    };
}

iter_t iterFromStr(str_t s) {
    return (iter_t){
        .beg = s.beg,
        .end = s.beg,
        .tail = s.end,
    };
}

iter_t iterFromBuf(buf_t b) {
    return (iter_t){
        .beg = b.ptr,
        .end = b.ptr,
        .tail = b.ptr + b.len,
    };
}

str_t iterStr(iter_t it) {
    if (iterDone(it)) return (str_t){0};
    return (str_t){it.beg, it.end};
}

bytes_t iterBytes(iter_t it) {
    return (bytes_t){it.beg, it.end};
}

bool iterLast(iter_t it) {
    /*
      On the last token (ie. the next "take" call will fail) if tail
      is null, but beg/end still point somewhere.
    */
    return it.tail == 0 && it.beg && it.end;
}

bool iterDone(iter_t it) {
    return it.tail == 0 && it.beg == 0 && it.end == 0;
}

bool iterTakeToChar(iter_t *it, utf8_char_t u) {
    if (iterLast(*it)) {
        *it = (iter_t){0};      /* completely clear out this iterator */
        return 0;
    }

    /* skip past our prior selection */
    it->beg = it->end;
    
    /* create a search string over the remainder of the iterator */
    str_t s = (str_t){it->end, it->tail};

    if (strNonEmpty(s)) {
        do {
            if (utf8CharEquals(utf8FirstChar(s), u)) {
                strNextChar(&s);
                it->end = s.beg;
                return 1;
            }
        } while (strNextChar(&s));
    }

    it->end = s.beg;
    it->tail = 0; /* we didn't find the character, so this is the last iteration */

    return 1;
}

bool iterTakeToAnyChar(iter_t *it, str_t needles) {
    return 0;
}

bool iterTakeToStr(iter_t *it, str_t sep) {
    if (iterLast(*it)) {
        *it = (iter_t){0};    /* Completely clear out this iterator */
        return 0;
    }

    /* skip past prior selection */
    it->beg = it->end;

    str_t s = (str_t){it->end, it->end + (sep.end - sep.beg)};

    /* Needs to be enough bytes in our iterator */
    if ((it->tail - s.beg) >= (sep.end - sep.beg)) {
        while (s.end <= it->tail) {
            if (strEquals(s, sep)) {
                it->end = s.end;
                return 1;
            }

            size char_width = utf8BytesNeeded(*s.beg);
            s.beg += char_width;
            s.end += char_width;
        }
    }

    it->end = it->tail;
    it->tail = 0; /* we didn't find sep, so this is the last iteration */

    return 1;
}

bool iterTakeToByte(iter_t *it, byte b) {
    return 0;
}

bool iterTakeToAnyByte(iter_t *it, bytes_t needles) {
    return 0;
}

bool iterTakeToBytes(iter_t *it, bytes_t bs) {
    return 0;
}
