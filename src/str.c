#include "std.h"

#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/mman.h>

str_t strFromC(char *s) {
  return s == NULL
    ? (str_t){0}
    : (str_t){s, s + strlen(s)};
}

bool strNextChar(str_t *s) {
  size char_width = utf8BytesNeeded(*s->beg);
  s->beg += char_width;
  /*
    This check isn't necessary for code that checks the return value,
    but does prevent us from going *wildly* out-of-bounds for
    wrong-code. /shrug
   */
  if (s->end < s->beg) {
      s->beg = s->end;
  }
  return s->beg < s->end;
}

inline size strLen(str_t s) {
    return utf8StrLen(s);
}

inline size strBytesLen(str_t s) {
    return s.end - s.beg;
}

inline bool strIsEmpty(str_t s) {
  return s.beg >= s.end;
}

inline bool strNonEmpty(str_t s) {
  return s.beg < s.end;
}

bool strEquals(str_t s, str_t t) {
  size s_len = s.end - s.beg;
  size t_len = t.end - t.beg;
  
  if (s_len != t_len) return 0;
  if (s_len == 0) return 1;	/* both are empty strings */
  if (s.beg == t.beg) return 1;	/* both point to the same memory */

  return bcmp(s.beg, t.beg, s_len) == 0;
}

bool strStartsWith(str_t s, str_t prefix) {
  if (prefix.beg == NULL) return 1;
  
  size prefix_len = prefix.end - prefix.beg;

  if (s.beg == NULL) return prefix_len == 0;
  if ((s.end - s.beg) < prefix_len) return 0;
  if (prefix_len == 1) return s.beg[0] == prefix.beg[0];

  s.end = s.beg + prefix_len;
  return strEquals(s, prefix);
}

bool strEndsWith(str_t s, str_t suffix) {
  if (suffix.beg == NULL) return 1;
  
  size suffix_len = suffix.end - suffix.beg;

  if (s.beg == NULL) return suffix_len == 0;
  if ((s.end - s.beg) < suffix_len) return 0;
  if (suffix_len == 1) return s.end[-1] == suffix.beg[0];
  
  s.beg = s.end - suffix_len;
  return strEquals(s, suffix);
}

str_t strTakeChars(str_t src, size count) {
  str_t result = src;
  
  do {
  } while (count-- > 0 && strNextChar(&src));
  
  result.end = src.beg;
  
  return result;
}

str_t strDropChars(str_t s, size count) {
  return utf8DropChars(s, count);
}

utf8_char_t strFirstChar(str_t s) {
  return utf8FirstChar(s);
}

str_t strFirstLine(str_t src) {
  return strTakeToChar(src, utf8CharFromC('\n'));
}

str_t strTakeToChar(str_t src, utf8_char_t c) {
  str_t cursor = src;

  do {
    if (utf8CharEquals(utf8FirstChar(cursor), c)) {
      break;
    }
  } while (strNextChar(&cursor));
  
  return (str_t){
    .beg = src.beg,
    .end = cursor.beg
  };
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

/*
  NOTE no tests yet.  TODO: modify the other take-to-* functions to
  behave like this & the iterators--ie., include the substr so you can
  tell if the search stopped because it a) found the substr, or b)
  found the end-of-string.
*/
str_t strTakeToStr(str_t src, str_t substr) {
    str_t cursor = src;
    do {
        if (strStartsWith(cursor, substr)) {
            return (str_t){
                .beg = src.beg,
                .end = cursor.beg + strBytesLen(substr),
            };
        }
    } while (strNextChar(&cursor));

    /* Couldn't find substr, so here's the whole thing */
    return src;
}

// TODO tests, also, unsure about this API and might remove...
str_t strDropToStr(str_t src, str_t substr) {
    str_t cursor = strTakeToStr(src, substr);
    return (str_t){
        .beg = cursor.end,
        .end = src.end,
    };
}

str_t strDropPrefix(str_t src, str_t prefix) {
    if (strStartsWith(src, prefix)) {
        src.beg += strBytesLen(prefix);
    }
    return src;
}

str_t strDropSuffix(str_t src, str_t suffix) {
    if (strEndsWith(src, suffix)) {
        src.end -= strBytesLen(suffix);
    }
    return src;
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
    while (cursor.beg <= cursor.end - char_width) {
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
    while (cursor.beg <= cursor.end - char_width) {
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

    for (size c = cols; c > 0 && strNonEmpty(text); c--, strNextChar(&text)) {
        if (*text.beg == ' ') {
            line.end = text.beg;
        } else if (*text.beg == '\n') {
            /* a forced newline, hard wrap at this point */
            line.end = text.beg + 1;
            break;
        }
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
  for (str_t i = s; strNonEmpty(i); strNextChar(&i)) {

    if (*i.beg < '0' || *i.beg > '9') {
      return 0;
    }
    
    tmp *= 10;
    tmp += *i.beg - '0';

    if (tmp < 0) {
      return 0;			/* overflow :^( */
    }
  }

  *result = negative ? -tmp : tmp;
  return 1;
}

/* str_iter_t strSplitIter(str_t s) { */
/*     return (str_iter_t){ */
/*         (str_t){ s.beg, s.beg }, */
/*         s.end */
/*     }; */
/* } */

/* bool strNextSplit(str_iter_t *s, str_t sep) { */
/*     str_t scanner = (str_t){ */
/*         s->str.end, */
/*         s->str.end + (sep.end - sep.beg) */
/*     }; */
    
/*     while (scanner.end < s->cap) { */
/*         if (strEquals(scanner, sep)) { */
/*             break; */
/*         } */
/*         size char_width = utf8BytesNeeded(*scanner.beg); */
/*         scanner.beg += char_width; */
/*         scanner.end += char_width; */
/*     } */

/*     s->str.beg = s->str.end; */
/*     s->str.end = scanner.end; */

/*     return s->str.beg < s->cap; */
/* } */
