#include "std.h"

printer printerFromFile(int fd, buf *buf) {
  return (printer){fd, buf};
}

_Bool printStr(printer p, str s) {
  return fdPrintStr(p.fd, p.buf, s);
}

_Bool printChar(printer p, char c) {
  return fdPrintChar(p.fd, p.buf, c);
}

_Bool printFlush(printer p) {
  return fdFlush(p.fd, p.buf);
}
