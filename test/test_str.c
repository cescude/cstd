#include <stdlib.h>

#include "../src/std.h"

typedef struct {
  size passed, failed;
} test_t;

typedef struct {
  char *name;
  void (*fn)(test_t *);
} test_defn_t;

print_t tout = printerFromFile(2, NULL);

void testFail(test_t *t, str_t desc) {
  t->failed++;
  printStr(tout, strC("FAILED: Case #"));
  printU64(tout, t->failed + t->passed);
  if (strNonEmpty(desc)) {
    printStr(tout, strC(", "));
    printStr(tout, desc);
  }
  printStr(tout, strC("\n"));
}

void testPass(test_t *t) {
  t->passed++;
}

#define assertTrue(t, bln, desc) if (bln) testPass(t); else testFail(t, desc)
#define assertFalse(t, bln, desc) assertTrue(t, !(bln), desc)

size runtests(test_defn_t *tests, size num_tests, bool verbose) {
  size total_passed = 0;
  size total_failed = 0;
  
  for (size i=0; i<num_tests; i++) {
    test_t t = {0};

    if (verbose) {
      printStr(tout, strC("["));
      printU64F(tout, i+1, (format_t){.right = 1, .width=4});
      printStr(tout, strC("] "));
      printStr(tout, strFromC(tests[i].name));
      printStr(tout, strC("\n"));
    }
    
    tests[i].fn(&t);

    if (verbose) {
      printStr(tout, strC("    passed="));
      printU64(tout, t.passed);
      printStr(tout, strC(" failed="));
      printU64(tout, t.failed);
      printStr(tout, strC("\n"));
    }

    total_passed += t.passed;
    total_failed += t.failed;
  }

  if (verbose) {
    printStr(tout, strC("\n"));
  }

  printStr(tout, strC("Results: passed="));
  printU64(tout, total_passed);
  printStr(tout, strC(" failed="));
  printU64(tout, total_failed);
  printStr(tout, strC("\n"));

  return total_failed;
}

/* taken from wikipedia */
char phrase1[] = "Mình nói tiếng Việt";
char phrase2[] = "𨉟呐㗂越";

#define nilstr (str_t){0}

void test_strLen_shouldReturnZero(test_t *t) {
  str_t cases[] = {
    strC(""), (str_t){0}
  };

  for (size i=0; i<countof(cases); i++) {
    assertTrue(t, strLen(cases[i]) == 0, nilstr);
    assertTrue(t, bytesLen(bytesFromStr(cases[i])) == 0, nilstr);
  }
}

void test_strLen_shouldCountUtf8Characters(test_t *t) {
  byte scratch[1024] = {0};
  buf_t buf = bufFromArray(scratch);
  
  struct {str_t s; size len; size bytes;} examples[] = {
    { strC(phrase1), 19, 25 },
    { strC(phrase2), 4, 13 },
  };

  for (size i=0; i<countof(examples); i++) {
    bufClear(&buf);
    bufAppendStr(&buf, strC("char length for phrase: "));
    bufAppendStr(&buf, examples[i].s);
    assertTrue(t, strLen(examples[i].s) == examples[i].len, strFromBuf(buf));

    bufClear(&buf);
    bufAppendStr(&buf, strC("byte length for phrase: "));
    bufAppendStr(&buf, examples[i].s);
    assertTrue(t, bytesLen(bytesFromStr(examples[i].s)) == examples[i].bytes, strFromBuf(buf));
  }  
}

void test_strIsEmpty_shouldFunctionAppropriately(test_t *t) {
  str_t empty[] = { (str_t){0}, strC("") };
  for (size i=0; i<countof(empty); i++) {
    assertTrue(t, strIsEmpty(empty[i]), nilstr);
    assertTrue(t, !strNonEmpty(empty[i]), nilstr);
  }

  str_t non_empty[] = {strC(" "), strC("one"), strC(phrase1), strC(phrase2)};
  for (size i=0; i<countof(non_empty); i++) {
    assertTrue(t, strNonEmpty(non_empty[i]), nilstr);
    assertTrue(t, !strIsEmpty(non_empty[i]), nilstr);
  }
}

void test_strEquals_shouldReturnTrue(test_t *t) {
  struct { str_t a, b; } cases[] = {
    { strC("one"), strC("one") },
    { strC(""), (str_t){0} },
    { strC(phrase1), strC(phrase1) },
  };

  for (size i=0; i<countof(cases); i++) {
    assertTrue(t, strEquals(cases[i].a, cases[i].b), nilstr);
    assertTrue(t, strEquals(cases[i].b, cases[i].a), nilstr);
  }
}

void test_strEquals_shouldReturnFalse(test_t *t) {
  struct { str_t a, b; } cases[] = {
    { strC("one"), strC("eno") },
    { strC("a"), (str_t){0} },
    { strC(phrase1), strC(phrase2) },
  };

  for (size i=0; i<countof(cases); i++) {
    assertTrue(t, !strEquals(cases[i].a, cases[i].b), nilstr);
    assertTrue(t, !strEquals(cases[i].b, cases[i].a), nilstr);
  }
}

void test_strEquals_shouldHandleOverlappingMemory(test_t *t) {
  char buffer[] = "aabbaabbaabb";
  str_t s0 = (str_t){buffer, buffer+8}; /* first aabbaabb */
  str_t s1 = (str_t){buffer+4, buffer+12}; /* second aabbaabb */
  assertTrue(t, strEquals(s0, s1), nilstr);
  assertTrue(t, strEquals(s1, s0), nilstr);

  str_t s2 = (str_t){buffer+1, buffer+9}; /* abbaabba */
  assertFalse(t, strEquals(s0, s2), nilstr);
  assertFalse(t, strEquals(s1, s2), nilstr);
}

void test_strStartsWith_shouldReturn1WhenPrefixed(test_t *t) {
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
    assertTrue(t, strStartsWith(cases[i].str, cases[i].prefix), nilstr);
  }

  struct { str_t str, prefix; } neg_cases[] = {
    { (str_t){0}, strC("a") },
    { strC(""), strC("a") },
    { strC("any"), strC("anything") },
    { strC("something"), strC("soem") },
    { strC("Mình nói"), strC("Mình nói tiếng Việt") }
  };
	  
  for (size i=0; i<countof(neg_cases); i++) {
    assertTrue(t, !strStartsWith(neg_cases[i].str,
				 neg_cases[i].prefix), nilstr);
  }
}

void test_strTrimLeft_basicFunctionality(test_t *t) {
  struct { str_t src, chars, result; } cases[] = {
    { strC("one"), strC(""), strC("one") }, /* nothing to trim here */
    { strC("one"), strC(" "), strC("one") },
    { strC("   \t one"), strC(" \t"), strC("one") },
    { strC("abcbabcdefabc"), strC("abc"), strC("defabc") }
  };

  for (size i=0; i<countof(cases); i++) {
    assertTrue(t,
	       strEquals(strTrimLeft(cases[i].src, cases[i].chars),
			 cases[i].result),
	       nilstr);
  }
}

void test_strTrimRight_basicFunctionality(test_t *t) {
  struct { str_t src, chars, result; } cases[] = {
    { strC("one"), strC(""), strC("one") }, /* nothing to trim here */
    { strC("one"), strC(" "), strC("one") },
    { strC("one  \t  "), strC(" \t"), strC("one") },
    { strC("abcbabcdefabcba"), strC("abc"), strC("abcbabcdef") }
  };

  for (size i=0; i<countof(cases); i++) {
    assertTrue(t,
	       strEquals(strTrimRight(cases[i].src, cases[i].chars),
			 cases[i].result),
	       nilstr);
  }
}

void test_strTakeLineWrapped_shouldReturnFullTextForShortRows(test_t *t) {
  struct { str_t src; size cols; } cases[] = {
    { strC("wrap"), 80 },
    { strC("wrap?"), 5 }
  };

  for (size i=0; i<countof(cases); i++) {
    assertTrue(t,
	       strEquals(cases[i].src,
			 strTakeLineWrapped(cases[i].src, cases[i].cols)),
	       nilstr);
  }
}

void test_strTakeLineWrapped_shouldReturnRowsNoLongerThanCols(test_t *t) {
  struct { str_t src; size cols; } cases[] = {
    { strC("wrap wrapwrap ok"), 10 },
    { strC("wrap?"), 5 },
    { strC("wrap!"), 3 }    /* hard split in the middle of the word */
  };

  for (size i=0; i<countof(cases); i++) {
    str_t result = strTakeLineWrapped(cases[i].src, cases[i].cols);
    assertTrue(t, strLen(result) <= cases[i].cols, nilstr);
  }
}

void test_strTakeLineWrapped_shouldBreakOnASpace(test_t *t) {
  str_t para = strC("Here's an amazing piece of writing that spans "
		    "a fair amount of text yielding many opportunities "
		    "for wrapping.");

  /* not looking for edge cases (ie. why 10...len-10) */
  for (size i=10; i<strLen(para) - 10; i++) {
    str_t result = strTakeLineWrapped(para, i);
    assertTrue(t, *result.end == ' ', nilstr);
  }
}

void test_utf8CharEquals_shouldMatchSameCharacter(test_t *t) {
  struct { utf8_char_t a, b; } cases[] = {
    { utf8CharFromC('v'), utf8CharFromC('v') },
    { utf8FirstChar(strC("verbose")), utf8CharFromC('v') },
    { utf8FirstChar(strC("😅")), utf8FirstChar(strDropChars(strC(" 😅"), 1)) },
  };

  for (size i=0; i<countof(cases); i++) {
    assertTrue(t, utf8CharEquals(cases[i].a, cases[i].b), nilstr);
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
  
  test_defn_t tests[] = {
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
