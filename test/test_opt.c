#include <stdlib.h>
#include <unistd.h>

#include "../src/std.h"

void test_optParse_shouldParseBooleans(test_t *t) {
    bool a = false;
    bool b = false;
    bool c = false;
    
    opt_t opts[] = {
        optBool(&a, 'a', "abool", 0),
        optBool(&b, 'b', "bbool", 0),
        optBool(&c, 'c', "cbool", 0),
    };

    opts_config_t conf = optInit(opts, countof(opts));

    char *case1[] = {"_", "-abc"};
    a = b = c = 0;
    assertTrue(t, optParse(conf, countof(case1), case1), strC("Short-args all smashed together"));
    assertTrue(t, a, strC("case1: a is set"));
    assertTrue(t, b, strC("case1: b is set"));
    assertTrue(t, c, strC("case1: c is set"));

    char *case2[] = {"_", "-c", "-a", "-b"};
    a = b = c = 0;
    assertTrue(t, optParse(conf, countof(case2), case2), strC("Short-args split"));
    assertTrue(t, a, strC("case2: a is set"));
    assertTrue(t, b, strC("case2: b is set"));
    assertTrue(t, c, strC("case2: c is set"));

    char *case3[] = {"_", "--abool", "--cbool", "--bbool"};
    a = b = c = 0;
    assertTrue(t, optParse(conf, countof(case3), case3), strC("Long-args split"));
    assertTrue(t, a, strC("case3: a is set"));
    assertTrue(t, b, strC("case3: b is set"));
    assertTrue(t, c, strC("case3: c is set"));
}

void test_optParse_shouldParseIntegers(test_t *t) {
    int a, b, c;
    
    opt_t opts[] = {
        optInt(&a, 'a', "aint", 0, 0),
        optInt(&b, 'b', "bint", 0, 0),
        optInt(&c, 'c', "cint", 0, 0),
    };

    opts_config_t conf = optInit(opts, countof(opts));

    char *case1[] = {"_", "-a=1", "-b2", "-c", "3"};
    a = b = c = 0;
    assertTrue(t, optParse(conf, countof(case1), case1), strC("Short int arguments"));
    assertTrue(t, a == 1, strC("case1: a is set"));
    assertTrue(t, b == 2, strC("case1: b is set"));
    assertTrue(t, c == 3, strC("case1: c is set"));

    char *case2[] = {"_", "--cint=3", "--aint", "1", "--bint", "2"};
    a = b = c = 0;
    assertTrue(t, optParse(conf, countof(case2), case2), strC("Long int arguments"));
    assertTrue(t, a == 1, strC("case2: a is set"));
    assertTrue(t, b == 2, strC("case2: b is set"));
    assertTrue(t, c == 3, strC("case2: c is set"));
}

void test_optParse_shouldParseStrings(test_t *t) {
    str_t a, b, c;
    
    opt_t opts[] = {
        optStr(&a, 'a', "astr", 0, 0),
        optStr(&b, 'b', "bstr", 0, 0),
        optStr(&c, 'c', "cstr", 0, 0),
    };

    opts_config_t conf = optInit(opts, countof(opts));

    char *case1[] = {"_", "-a=one", "-btwo", "-c", "three"};
    a = b = c = (str_t){0};
    assertTrue(t, optParse(conf, countof(case1), case1), strC("Short string arguments"));
    assertTrue(t, strEquals(a, strC("one")), strC("case1: a is set"));
    assertTrue(t, strEquals(b, strC("two")), strC("case1: b is set"));
    assertTrue(t, strEquals(c, strC("three")), strC("case1: c is set"));

    char *case2[] = {"_", "--cstr=three", "--astr", "one", "--bstr", "two"};
    a = b = c = (str_t){0};
    assertTrue(t, optParse(conf, countof(case2), case2), strC("Long string arguments"));
    assertTrue(t, strEquals(a, strC("one")), strC("case2: a is set"));
    assertTrue(t, strEquals(b, strC("two")), strC("case2: b is set"));
    assertTrue(t, strEquals(c, strC("three")), strC("case2: c is set"));
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
        optPrintUsage(config, argv[0], "Run unit tests on the opt library.");
        return help ? 0 : 99;
    }
  
    test_defn_t tests[] = {
        {test_optParse_shouldParseBooleans, "optParse should parse booleans"},
        {test_optParse_shouldParseIntegers, "optParse should parse intbers"},
        {test_optParse_shouldParseStrings, "optParse should parse strings"},
    };

    return (int)testRunner(tests, countof(tests), verbose);
}
