#include "../src/std/std.h"

#include <stdlib.h>

typedef struct {
  char *name;
  void (*fn)(size *passed, size *failed);
} test_t;

#define assertTrue(bln) assertFalse(!(bln))

#define assertFalse(bln) {						\
    if (bln) {								\
      fprintf(stderr, "%s -- failed at line %d\n", __FUNCTION__, __LINE__); \
      (*failed)++;							\
    } else {								\
      (*passed)++;							\
    }									\
  }

size runtests(test_t *tests, size num_tests, bool verbose) {
  size total_passed = 0;
  size total_failed = 0;
  
  for (size i=0; i<num_tests; i++) {
    size num_passed = 0;
    size num_failed = 0;

    if (verbose) {
      fprintf(stderr, "[%04lu]\t%s\n", i+1, tests[i].name);
      fflush(stderr);
    }
    
    tests[i].fn(&num_passed, &num_failed);

    if (verbose) {
      fprintf(stderr, "\tpassed=%4lu failed=%4lu\n", num_passed, num_failed);
    }

    total_passed += num_passed;
    total_failed += num_failed;
  }

  if (verbose) {
    fprintf(stderr, "\nResults:\tpassed= %lu failed= %lu\n", total_passed, total_failed);
  } else {
    fprintf(stderr, "Results: passed=%lu failed=%lu\n", total_passed, total_failed);
  }

  return total_failed;
}

/* taken from wikipedia */
char phrase1[] = "Mình nói tiếng Việt";
char phrase2[] = "𨉟呐㗂越";

void test_strLen_shouldReturnZero(size *passed, size *failed) {
  str_t cases[] = {
    strC(""), (str_t){0}
  };

  for (size i=0; i<countof(cases); i++) {
    assertTrue(strLen(cases[i]) == 0);
    assertTrue(bytesLen(bytesFromStr(cases[i])) == 0);
  }
}

void test_strLen_shouldCountUtf8Characters(size *passed, size *failed) {
  struct {str_t s; size len; size bytes;} examples[] = {
    { strC(phrase1), 19, 25 },
    { strC(phrase2), 4, 13 },
  };

  for (size i=0; i<countof(examples); i++) {
    assertTrue(strLen(examples[i].s) == examples[i].len);
    assertTrue(bytesLen(bytesFromStr(examples[i].s)) == examples[i].bytes);
  }  
}

void test_strIsEmpty_shouldFunctionAppropriately(size *passed, size *failed) {
  str_t empty[] = { (str_t){0}, strC("") };
  for (size i=0; i<countof(empty); i++) {
    assertTrue(strIsEmpty(empty[i]));
    assertTrue(!strNonEmpty(empty[i]));
  }

  str_t non_empty[] = {strC(" "), strC("one"), strC(phrase1), strC(phrase2)};
  for (size i=0; i<countof(non_empty); i++) {
    assertTrue(strNonEmpty(non_empty[i]));
    assertTrue(!strIsEmpty(non_empty[i]));
  }
}

void test_strEquals_shouldReturnTrue(size *passed, size *failed) {
  struct { str_t a, b; } cases[] = {
    { strC("one"), strC("one") },
    { strC(""), (str_t){0} },
    { strC(phrase1), strC(phrase1) },
  };

  for (size i=0; i<countof(cases); i++) {
    assertTrue(strEquals(cases[i].a, cases[i].b));
    assertTrue(strEquals(cases[i].b, cases[i].a));
  }
}

void test_strEquals_shouldReturnFalse(size *passed, size *failed) {
  struct { str_t a, b; } cases[] = {
    { strC("one"), strC("eno") },
    { strC("a"), (str_t){0} },
    { strC(phrase1), strC(phrase2) },
  };

  for (size i=0; i<countof(cases); i++) {
    assertTrue(!strEquals(cases[i].a, cases[i].b));
    assertTrue(!strEquals(cases[i].b, cases[i].a));
  }
}

void test_strEquals_shouldHandleOverlappingMemory(size *passed, size *failed) {
  char buffer[] = "aabbaabbaabb";
  str_t s0 = (str_t){buffer, buffer+8}; /* first aabbaabb */
  str_t s1 = (str_t){buffer+4, buffer+12}; /* second aabbaabb */
  assertTrue(strEquals(s0, s1));
  assertTrue(strEquals(s1, s0));

  str_t s2 = (str_t){buffer+1, buffer+9}; /* abbaabba */
  assertFalse(strEquals(s0, s2));
  assertFalse(strEquals(s1, s2));
}

void test_strStartsWith_shouldReturn1WhenPrefixed(size *passed, size *failed) {
  struct { str_t str, prefix; } cases[] = {
    { strC(""), strC("") },
    { (str_t){0}, strC("") },
    { strC("anything"), (str_t){0} },
    { strC("anything"), strC("") },
    { strC("anything"), strC("any") },
    { strC(phrase1), strC(phrase1) },
    { strC("Mình nói tiếng Việt"), strC("Mình nói") }
  };

  for (size i=0; i<countof(cases); i++) {
    assertTrue(strStartsWith(cases[i].str, cases[i].prefix));
  }

  struct { str_t str, prefix; } neg_cases[] = {
    { (str_t){0}, strC("a") },
    { strC(""), strC("a") },
    { strC("any"), strC("anything") },
    { strC("something"), strC("soem") },
    { strC("Mình nói"), strC("Mình nói tiếng Việt") }
  };
	  
  for (size i=0; i<countof(neg_cases); i++) {
    assertTrue(!strStartsWith(neg_cases[i].str,
			      neg_cases[i].prefix));
  }
}

void test_strTrimLeft_basicFunctionality(size *passed, size *failed) {
  struct { str_t src, chars, result; } cases[] = {
    { strC("one"), strC(""), strC("one") }, /* nothing to trim here */
    { strC("one"), strC(" "), strC("one") },
    { strC("   \t one"), strC(" \t"), strC("one") },
    { strC("abcbabcdefabc"), strC("abc"), strC("defabc") }
  };

  for (size i=0; i<countof(cases); i++) {
    assertTrue(strEquals(strTrimLeft(cases[i].src, cases[i].chars), cases[i].result));
  }
}

void test_strTrimRight_basicFunctionality(size *passed, size *failed) {
  struct { str_t src, chars, result; } cases[] = {
    { strC("one"), strC(""), strC("one") }, /* nothing to trim here */
    { strC("one"), strC(" "), strC("one") },
    { strC("one  \t  "), strC(" \t"), strC("one") },
    { strC("abcbabcdefabcba"), strC("abc"), strC("abcbabcdef") }
  };

  for (size i=0; i<countof(cases); i++) {
    assertTrue(strEquals(strTrimRight(cases[i].src, cases[i].chars), cases[i].result));
  }
}

void test_strTakeLineWrapped_shouldReturnFullTextForShortRows(size *passed, size *failed) {
  struct { str_t src; size cols; } cases[] = {
    { strC("wrap"), 80 },
    { strC("wrap?"), 5 }
  };

  for (size i=0; i<countof(cases); i++) {
    assertTrue(strEquals(cases[i].src, strTakeLineWrapped(cases[i].src, cases[i].cols)));
  }
}

void test_strTakeLineWrapped_shouldReturnRowsNoLongerThanCols(size *passed, size *failed) {
  struct { str_t src; size cols; } cases[] = {
    { strC("wrap wrapwrap ok"), 10 },
    { strC("wrap?"), 5 },
    { strC("wrap!"), 3 }    /* hard split in the middle of the word */
  };

  for (size i=0; i<countof(cases); i++) {
    str_t result = strTakeLineWrapped(cases[i].src, cases[i].cols);
    assertTrue(strLen(result) <= cases[i].cols);
  }
}

void test_strTakeLineWrapped_shouldBreakOnASpace(size *passed, size *failed) {
  str_t para = strC("Here's an amazing piece of writing that spans "
		    "a fair amount of text yielding many opportunities "
		    "for wrapping.");

  /* not looking for edge cases (ie. why 10...len-10) */
  for (size i=10; i<strLen(para) - 10; i++) {
    str_t result = strTakeLineWrapped(para, i);
    assertTrue(*result.end == ' ');
  }
}

void test_utf8CharEquals_shouldMatchSameCharacter(size *passed, size *failed) {
  struct { utf8_char_t a, b; } cases[] = {
    { utf8CharFromC('v'), utf8CharFromC('v') },
    { utf8FirstChar(strC("verbose")), utf8CharFromC('v') },
    { utf8FirstChar(strC("😅")), utf8FirstChar(strDropChars(strC(" 😅"), 1)) },
  };

  for (size i=0; i<countof(cases); i++) {
    assertTrue(utf8CharEquals(cases[i].a, cases[i].b));
  }
}

int main(int argc, char **argv) {
  bool verbose = false;
  bool help = false;
  opt_t opts[] = {
    optBool(&verbose, 'v', "verbose", "Be louder"),
    optBool(&help, 'h', "help", "Show this help information"),
  };

  opts_config_t config = optInit(opts, countof(opts),
				 "USAGE: ./test_str [OPTIONS]\n\n"
				 "Run unit tests on the string library.\n\n");

  if (!optParse(config, argc, argv) || help) {
    optPrintUsage(config, 80);
    return help ? 0 : 99;
  }
  
  test_t tests[] = {
    {"strLen should return zero", test_strLen_shouldReturnZero},
    {"strLen should count utf8 characters", test_strLen_shouldCountUtf8Characters},
    {"strIsEmpty should function appropriately", test_strIsEmpty_shouldFunctionAppropriately},
    {"strEquals should return true on same bytes", test_strEquals_shouldReturnTrue},
    {"strEquals should return false on different bytes", test_strEquals_shouldReturnFalse},
    {"strEquals should handle overlapping memory", test_strEquals_shouldHandleOverlappingMemory},
    {"strStartsWith should return 1 when prefixed", test_strStartsWith_shouldReturn1WhenPrefixed},
    {"strTrimLeft basic functionality", test_strTrimLeft_basicFunctionality},
    {"strTrimRight Basic functionality", test_strTrimRight_basicFunctionality},
    {"strTakeLineWrapped Should return full text for short rows", test_strTakeLineWrapped_shouldReturnFullTextForShortRows},
    {"strTakeLineWrapped Should return rows no longer than cols", test_strTakeLineWrapped_shouldReturnRowsNoLongerThanCols},
    {"strTakeLineWrapped Should break on a space", test_strTakeLineWrapped_shouldBreakOnASpace},
    {"utf8CharEquals Basic functionality", test_utf8CharEquals_shouldMatchSameCharacter},
  };

  return (int)runtests(tests, countof(tests), verbose);
}
