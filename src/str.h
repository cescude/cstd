#pragma once

str_t strFromC(char *cstr);
bool strNextChar(str_t *s);

size strLen(str_t s);

bool strIsEmpty(str_t s);
bool strNonEmpty(str_t s);

bool strEquals(str_t s, str_t t);
bool strStartsWith(str_t s, str_t prefix);

str_t strTakeChars(str_t src, size count);
str_t strDropChars(str_t s, size count);

utf8_char_t strFirstChar(str_t s);

str_t strFirstLine(str_t src);

str_t strTakeToChar(str_t src, utf8_char_t c);

str_t strTrim(str_t haystack, str_t needles);
str_t strTrimLeft(str_t haystack, str_t needles);
str_t strTrimRight(str_t haystack, str_t needles);
str_t strTakeLineWrapped(str_t text, size cols);

uint64_t strHash_djb2(str_t src);

bool strMaybeParseInt(str_t s, int *result);
