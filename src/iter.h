#pragma once

typedef struct {
    byte *beg;
    byte *end;
    byte *tail;
} iter_t;

bool iterLast(iter_t it);
bool iterDone(iter_t it);

iter_t iterFromBytes(bytes_t bs);
iter_t iterFromStr(str_t s);
iter_t iterFromBuf(buf_t b);

str_t iterStr(iter_t it);
bytes_t iterBytes(iter_t it);

#define iterStruct(it, type) (type*)_iterStruct(it, sizeof(type))
void *_iterStruct(iter_t it, size size_of_struct);

bool iterTakeToChar(iter_t *it, utf8_char_t u);
bool iterTakeToAnyChar(iter_t *it, str_t needles);
bool iterTakeToStr(iter_t *it, str_t sep);

bool iterTakeToByte(iter_t *it, byte b);
bool iterTakeToAnyByte(iter_t *it, bytes_t needles);
bool iterTakeToBytes(iter_t *it, bytes_t bs);
