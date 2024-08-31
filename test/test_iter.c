#include <stdlib.h>

#include "../src/std.h"

#define nilstr (str_t){0}

void test_iterStr_shouldReturnTheCurrentSelection(test_t *t) {
    struct { str_t src, expected; size idx0, idx1; } cases[] = {
        { strC("full string"), strC(""), 0, 0 },
        { strC("full string"), strC("full"), 0, 4 },
    };

    for (size i=0; i<countof(cases); i++) {
        iter_t it = iterFromStr(cases[i].src);
        it.beg += cases[i].idx0;
        it.end += cases[i].idx1;

        assertTrue(t, strEquals(cases[i].expected, iterStr(it)), nilstr);
    }
}

void test_iterTakeToChar_shouldIterateOverAString(test_t *t) {
    struct { str_t src; utf8_char_t u; size expected_segments; } cases[] = {
        { strC(""), utf8CharFromC(' '), 1 },
        { strC("one"), utf8CharFromC(' '), 1 },
        { strC("one "), utf8CharFromC(' '), 2 },
        { strC("one two three"), utf8CharFromC(' '), 3 },
        { strC("one two three "), utf8CharFromC(' '), 4 },
        { strC("one😅two😅three"), utf8FirstChar(strC("😅")), 3 },
    };

    for (size i=0; i<countof(cases); i++) {
        iter_t it = iterFromStr(cases[i].src);

        size segments = 0;
        while (iterTakeToChar(&it, cases[i].u)) {
            segments++;
        }

        assertTrue(t, segments == cases[i].expected_segments, nilstr);
    }
}

void test_iterTakeToAnyChar_shouldIterateOverAString(test_t *t) {
    str_t stop_chars = strC(" \t😅");
    struct { str_t src; size expected_segments; } cases[] = {
        { strC(""), 1 },
        { strC("one"), 1 },
        { strC("one😅two three\tfour"), 4 },
        { strC("one two😅three\t"), 4 },
    };

    for (size i=0; i<countof(cases); i++) {
        iter_t it = iterFromStr(cases[i].src);

        size segments = 0;
        while (iterTakeToAnyChar(&it, stop_chars)) {
            segments++;
        }

        assertTrue(t, segments == cases[i].expected_segments, cases[i].src);
    }
}

void test_iterTakeToStr_shouldIterateOverAString(test_t *t) {
    struct { str_t src, needle; size expected_segments; } cases[] = {
        { strC(""), strC("ok"), 1 },
        { strC("one"), strC("ok"), 1 },
        { strC("oneok"), strC("ok"), 2 },
        { strC("one__two__three"), strC("__"), 3 },
        { strC("one__two__three__"), strC("__"), 4 },
    };

    for (size i=0; i<countof(cases); i++) {
        iter_t it = iterFromStr(cases[i].src);

        size segments = 0;
        while (iterTakeToStr(&it, cases[i].needle)) {
            segments++;
        }

        assertTrue(t, segments == cases[i].expected_segments, nilstr);
    }
}

void test_iterTakeToByte_shouldIterateOverAString(test_t *t) {
    struct { str_t src; byte needle; size expected_segments; } cases[] = {
        { strC(""), ';', 1 },
        { strC("one"), ';', 1 },
        { strC("one;"), ';', 2 },
        { strC("one_two_three"), '_', 3 },
        { strC("one_two_three_"), '_', 4 },
    };

    for (size i=0; i<countof(cases); i++) {
        iter_t it = iterFromStr(cases[i].src);

        size segments = 0;
        while (iterTakeToByte(&it, cases[i].needle)) {
            segments++;
        }

        assertTrue(t, segments == cases[i].expected_segments, nilstr);
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
        optPrintUsage(config, argv[0], "Run unit tests on the iter library.");
        return help ? 0 : 99;
    }
  
    test_defn_t tests[] = {
        {"iterStr should return the current selection", test_iterStr_shouldReturnTheCurrentSelection},
        {"iterTakeToChar should iterate over a string", test_iterTakeToChar_shouldIterateOverAString},
        {"iterTakeToAnyChar should iterate over a string", test_iterTakeToAnyChar_shouldIterateOverAString},
        {"iterTakeToStr should iterate over a string", test_iterTakeToStr_shouldIterateOverAString},
        {"iterTakeToByte should iterate over a string", test_iterTakeToByte_shouldIterateOverAString},
    };

    return (int)testRunner(tests, countof(tests), verbose);
}
