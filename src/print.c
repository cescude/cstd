#include "std.h"

bool printFlush(print_t p) {
    return fdFlush(p.fd, p.buf);
}

bool printStr(print_t p, str_t s) {
    return fdPrintStr(p.fd, p.buf, s);
}

bool printStrF(print_t p, str_t s, format_t f) {
    return fdPrintStrF(p.fd, p.buf, s, f);
}

bool printChar(print_t p, utf8_char_t c) {
    return fdPrintChar(p.fd, p.buf, c);
}

bool printCharF(print_t p, utf8_char_t c, format_t f) {
    return fdPrintCharF(p.fd, p.buf, c, f);
}

bool printUnsigned(print_t p, uint64_t n) {
    return printUnsignedF(p, n, (format_t){0});
}

bool printUnsignedF(print_t p, uint64_t n, format_t f) {
    return fdPrintU64F(p.fd, p.buf, n, f);
}

bool printNum(print_t p, int64_t n) {
    return printNumF(p, n, (format_t){0});
}

bool printNumF(print_t p, int64_t n, format_t f) {
    return fdPrintI64F(p.fd, p.buf, n, f);
}

bool printBytes(print_t p, bytes_t bs) {
    return printBytesF(p, bs, (format_t){0});
}

bool printBytesF(print_t p, bytes_t bs, format_t f) {
    return fdPrintBytesF(p.fd, p.buf, bs, (format_t){0});
}
