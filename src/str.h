#pragma once

str_t strFromC(char *cstr);
bool strNextChar(str_t *s);

size strLen(str_t s);
size strBytesLen(str_t s);

bool strIsEmpty(str_t s);
bool strNonEmpty(str_t s);

bool strEquals(str_t s, str_t t);
bool strStartsWith(str_t s, str_t prefix);
bool strEndsWith(str_t s, str_t suffix);

str_t strTakeChars(str_t src, size count);
str_t strDropChars(str_t s, size count);

utf8_char_t strFirstChar(str_t s);

str_t strFirstLine(str_t src);

str_t strTakeToChar(str_t src, utf8_char_t c);
str_t strTakeToByte(str_t src, byte b);
str_t strTakeToStr(str_t src, str_t substr);
str_t strDropToStr(str_t src, str_t substr);

str_t strDropPrefix(str_t src, str_t prefix);
str_t strDropSuffix(str_t src, str_t suffix);

str_t strTrim(str_t haystack, str_t needles);
str_t strTrimLeft(str_t haystack, str_t needles);
str_t strTrimRight(str_t haystack, str_t needles);
str_t strTakeLineWrapped(str_t text, size cols);

uint64_t strHash_djb2(str_t src);

bool strMaybeParseInt(str_t s, int *result);

