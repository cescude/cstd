#include <stdlib.h>

#include "../src/std.h"

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
    byte buf_data[1024] = {0};
    buf_t buf = bufFromC(buf_data);
    print_t p = printInitNoFile(&buf);
  
    struct {str_t s; size len; size bytes;} examples[] = {
        { strC(phrase1), 19, 25 },
        { strC(phrase2), 4, 13 },
    };

    for (size i=0; i<countof(examples); i++) {
        bufClear(&buf);
        printStr(p, strC("char length for phrase <<"));
        printStr(p, examples[i].s);
        printStr(p, strC(">> expecting: "));
        printUnsigned(p, examples[i].len);
        assertTrue(t, strLen(examples[i].s) == examples[i].len, strFromBuf(buf));

        bufClear(&buf);
        printStr(p, strC("byte length for phrase <<"));
        printStr(p, examples[i].s);
        printStr(p, strC(">> expecting "));
        printUnsigned(p, examples[i].bytes);
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

void test_strDropChars_shouldDropChars(test_t *t) {
    struct { str_t str, expected; size count; } cases[] = {
        { strC("Hey string"), strC("Hey string"), 0 },
        { strC("Hey string"), strC("ring"), 6 },
        { strC("Hey string"), strC(""), 100 },
        { strC("𨉟呐㗂越"), strC("㗂越"), 2 },
    };

    byte data[64] = {0};
    buf_t buf = bufFromC(data);
    print_t p = printInitNoFile(&buf);
  
    for (size i=0; i<countof(cases); i++) {
        bufClear(&buf);
        printC(p, "Dropping ");
        printUnsigned(p, cases[i].count);
        printC(p, " characters from <<");
        printStr(p, cases[i].str);
        printC(p, ">>, expected <<");
        printStr(p, cases[i].expected); printC(p, ">>");
    
        assertTrue(t, strEquals(strDropChars(cases[i].str, cases[i].count),
                                cases[i].expected), strFromBuf(buf));
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

    opts_config_t config = optInit(opts, countof(opts));

    if (!optParse(config, argc, argv) || help) {
        optPrintUsage(config, argv[0], "Run unit tests on the string library.");
        return help ? 0 : 99;
    }
  
    test_defn_t tests[] = {
        {test_strLen_shouldReturnZero, "strLen should return zero"},
        {test_strLen_shouldCountUtf8Characters, "strLen should count utf8 characters"},
        {test_strIsEmpty_shouldFunctionAppropriately, "strIsEmpty should function appropriately"},
        {test_strEquals_shouldReturnTrue, "strEquals should return true on same bytes"},
        {test_strEquals_shouldReturnFalse, "strEquals should return false on different bytes"},
        {test_strEquals_shouldHandleOverlappingMemory, "strEquals should handle overlapping memory"},
        {test_strStartsWith_shouldReturn1WhenPrefixed, "strStartsWith should return 1 when prefixed"},
        {test_strDropChars_shouldDropChars, "strDropChars should drop the specified # of characters"},
        {test_strTrimLeft_basicFunctionality, "strTrimLeft basic functionality"},
        {test_strTrimRight_basicFunctionality, "strTrimRight Basic functionality"},
        {test_strTakeLineWrapped_shouldReturnFullTextForShortRows, "strTakeLineWrapped Should return full text for short rows"},
        {test_strTakeLineWrapped_shouldReturnRowsNoLongerThanCols, "strTakeLineWrapped Should return rows no longer than cols"},
        {test_strTakeLineWrapped_shouldBreakOnASpace, "strTakeLineWrapped Should break on a space"},
        {test_utf8CharEquals_shouldMatchSameCharacter, "utf8CharEquals Basic functionality"},
    };

    return (int)testRunner(tests, countof(tests), verbose);
}
