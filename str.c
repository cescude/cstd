#include "std.h"

str strFromC(char *cstr) {
  return (str){cstr, strlen(cstr)};
}

str strFromBuf(buf buf) {
  return (str){buf.ptr, buf.len};
}

str strFirstLine(str src) {
  return strTakeToChar(src, '\n');
}

str strTakeToChar(str src, char c) {
  str result = src;
  for (result.len = 0; result.len < src.len; result.len++) {
    if (src.ptr[result.len] == c) {
      break;
    }
  }
  return result;
}

