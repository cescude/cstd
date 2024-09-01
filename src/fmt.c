#include "std.h"

fmt_t fmtInit(print_t out) {
    return (fmt_t){
        .it = {0},
        .out = out,
    };
}

fmt_t fmtToBuffer(buf_t *buf) {
    return (fmt_t){
        .it = {0},
        .out = printInitNoFile(buf),
    };
}

fmt_t fmtToFile(int fd) {
    return (fmt_t){
        .it = {0},
        .out = printInitUnbuffered(fd),
    };
}

static str_t sep = strC("{}");

static inline void _advance(print_t out, iter_t *it) {
    if (iterTakeToStr(it, sep)) {
        printStr(out, strDropSuffix(iterStr(*it), sep));
    }
}

void fmtStart(fmt_t *fmt, char *pattern) {
    iter_t it = iterFromStr(strFromC(pattern));
    _advance(fmt->out, &it);
    fmt->it = it;
}

void fmtStr(fmt_t *fmt, str_t str) {
    printStr(fmt->out, str);
    _advance(fmt->out, &fmt->it);
}

void fmtChar(fmt_t *fmt, utf8_char_t c) {
    printChar(fmt->out, c);
    _advance(fmt->out, &fmt->it);
}

void fmtNum(fmt_t *fmt, int64_t n) {
    printNum(fmt->out, n);
    _advance(fmt->out, &fmt->it);
}

void fmtUnm(fmt_t *fmt, uint64_t u) {
    printUnsigned(fmt->out, u);
    _advance(fmt->out, &fmt->it);
}
