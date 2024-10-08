#pragma once

typedef struct {
    iter_t it;
    print_t out;
} fmt_t;

fmt_t fmtInit(print_t out);
fmt_t fmtToBuffer(buf_t *buf);
fmt_t fmtToFile(int fd);

void fmtStart(fmt_t *fmt, char *pattern);
void fmtBool(fmt_t *fmt, bool b);
void fmtStr(fmt_t *fmt, str_t str);
void fmtChar(fmt_t *fmt, utf8_char_t c);
void fmtNum(fmt_t *fmt, int64_t n);
void fmtUnm(fmt_t *fmt, uint64_t u);
void fmtSkip(fmt_t *fmt);       /* Skip a gap */
