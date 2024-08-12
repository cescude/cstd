#include "std.h"

#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

size strLen(str_t s) {
  return utf8StrLen(s);
}

size strLenBytes(str_t s) {
  return s.end - s.beg;
}

bool strIsEmpty(str_t s) {
  return s.beg >= s.end;
}

bool strNonEmpty(str_t s) {
  return s.beg < s.end;
}

bool strEquals(str_t s, str_t t) {
  size s_len = strLenBytes(s);
  size t_len = strLenBytes(t);
  
  if (s_len != t_len) return 0;
  if (s_len == 0) return 1;	/* both are empty strings */
  if (s.beg == t.beg) return 1;	/* both point to the same memory */

  size min_sz = s_len < t_len ? s_len : t_len;
  return memcmp(s.beg, t.beg, min_sz) == 0;
}

bool strStartsWith(str_t s, str_t prefix) {
  size prefix_len = strLenBytes(prefix);
  if (strLenBytes(s) < prefix_len) return 0;
  
  s.end = s.beg + prefix_len;
  return strEquals(s, prefix);
}

str_t strDropChars(str_t s, size count) {
  return utf8DropChars(s, count);
}

str_t strDropBytes(str_t s, size count) {
  if (count < strLenBytes(s)) {
    s.beg += count;
  } else {
    s.beg = s.end;
  }
  return s;
}

utf8_char_t strFirstChar(str_t s) {
  return utf8FirstChar(s);
}

str_t strFirstLine(str_t src) {
  return strTakeToByte(src, '\n');
}

str_t strTakeToChar(str_t src, utf8_char_t c) {
  // ...
  abort();
  return (str_t){0};
}

str_t strTakeToByte(str_t src, char c) {
  str_t result = (str_t){src.beg, src.beg};
  
  for (; result.end < src.end; result.end++) {
    if (*result.end == c) {
      break;
    }
  }
  return result;
}

str_t strSkipByte(str_t src, char c) {
  while (src.beg < src.end && *src.beg == c) {
    src.beg++;
  }
  
  return src;
}

str_t strTrim(str_t src, str_t needles) {
  return strTrimRight(strTrimLeft(src, needles), needles);
}

str_t strTrimLeft(str_t src, str_t needles) {

  /* one character at a time, we move src.beg towards src.end */
  
  while (src.beg < src.end) {
    size char_width = utf8BytesNeeded(*src.beg);

    bool found_char = 0;

    str_t cursor = needles;
    while (cursor.beg < cursor.end - char_width) {
      if (memcmp(src.beg, cursor.beg, char_width) == 0) {
	found_char = 1;
	break;
      }

      cursor.beg += utf8BytesNeeded(*cursor.beg);
    }
    
    if (!found_char) {
      return src;
    }

    src.beg += char_width;
  }

  return src;
}

str_t strTrimRight(str_t src, str_t needles) {

  /* assume we've trimmed the whole string */
  str_t result = (str_t){src.beg, src.beg};
  
  while (src.beg < src.end) {
    size char_width = utf8BytesNeeded(*src.beg);

    bool found_char = 0;

    str_t cursor = needles;
    while (cursor.beg < cursor.end - char_width) {
      if (memcmp(src.beg, cursor.beg, char_width) == 0) {
	found_char = 1;
	break;
      }

      cursor.beg += utf8BytesNeeded(*cursor.beg);
    }
    
    src.beg += char_width;

    if (!found_char) {
      result.end = src.beg;
    }
  }

  return result;
}

str_t strTakeLineWrapped(str_t text, size cols) {
  if (strLen(text) <= cols) {
    return text;
  }

  str_t line = (str_t){text.beg, text.beg};

  size original_cols = cols;
  
  while (cols > 0 && text.beg < text.end) {
    if (*text.beg == ' ') {
      line.end = text.beg;
    } else if (*text.beg == '\n') {
      line.end = text.beg;
      cols = original_cols; 	/* a forced newline, we can start over */
      /* line.end = text.beg; */
      /* break; */
    }

    text.beg++;
    cols--;
  }
  
  if (strIsEmpty(line)) {
    /* a single word is more than cols wide, need to break it up! */
    line.end = text.beg;
  }

  return line;
}

uint64_t strHash_djb2(str_t src) {
  /* http://www.cse.yorku.ca/~oz/hash.html */
  uint64_t hash = 5381;

  size src_len = src.end - src.beg;
  for (size i=0; i<src_len; i++) {
    hash = 33 * hash ^ src.beg[i];
  }
  return hash;
}


// TODO tests
bool strMaybeParseInt(str_t s, int *result) {
  bool negative = 0;
  
  if (strIsEmpty(s)) return 0;

  if (s.beg[0] == '-') {
    negative = 1;

    s.beg++;

    /* can't have just "-", need at least "-N" */
    if (strIsEmpty(s)) return 0;
  }

  int tmp = 0;
  for (; s.beg < s.end; s.beg++) {

    if (*s.beg < '0' || *s.beg > '9') {
      return 0;
    }
    
    tmp *= 10;
    tmp += *s.beg - '0';

    if (tmp < 0) {
      return 0;			/* overflow :^( */
    }
  }

  *result = negative ? -tmp : tmp;
  return 1;
}

