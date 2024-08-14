#pragma once

bytes_t bytesFromStr(str_t s); // TODO: make the opposite for strings

bool bytesIntoStruct(bytes_t b, void *ptr, size ptr_sz);
bytes_t bytesFromStruct(void *ptr, size ptr_sz);

size bytesLen(bytes_t b);

bool bytesIsEmpty(bytes_t b);
bool bytesNonEmpty(bytes_t b);

bool bytesEquals(bytes_t a, bytes_t b);

bytes_t bytesDrop(bytes_t b, size count);
bytes_t bytesTakeToByte(bytes_t b, byte v);
bytes_t bytesSkipByte(bytes_t b, byte v);
