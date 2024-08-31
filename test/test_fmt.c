#include <stdlib.h>
#include <unistd.h>

#include "../src/std.h"

void test_fmtNew(test_t *t) {
    char data[100] = {0};
    buf_t buf = bufFromC(data);
    fmt_t fmt = fmtToBuffer(&buf);

    fmtNew(&fmt, "We'll print {} a few times, perhaps {} times!\n");
    fmtStr(&fmt, strC("this"));
    fmtNum(&fmt, 99);

    assertTrue(
        t,
        strEquals(strFromBuf(buf), strC("We'll print this a few times, perhaps 99 times!\n")),
        strC("Should format/print strings and numbers")
    );
}

void test_shouldReplaceHoles(test_t *t) {
    char data[100] = {0};
    buf_t buf = bufFromC(data);
    fmt_t fmt = fmtToBuffer(&buf);

    struct { char *pat; str_t toks, expected; } cases[] = {
        { "one={}, two={}, three={}", strC("1,2,3"), strC("one=1, two=2, three=3") },
        { "one={}, two={}, three={}", strC("1,TWO,3"), strC("one=1, two=TWO, three=3") },
        { "{}{}{}", strC("THREE,two,1111"), strC("THREEtwo1111") },
        {
            "first {} looks like {} with {} sections labelled {}!",
            strC("segment,this,100,that"),
            strC("first segment looks like this with 100 sections labelled that!")
        },
        {
            "Is it {} to {} in {}?",
            strC("safe,embed {} signs,here"),
            strC("Is it safe to embed {} signs in here?")
        },
    };

    for (size i=0; i<countof(cases); i++) {
        bufClear(&buf);
        
        fmtNew(&fmt, cases[i].pat);

        iter_t it = iterFromStr(cases[i].toks);
        while (iterTakeToByte(&it, ',')) {

            str_t tok = strDropSuffix(iterStr(it), strC(","));
            int num = 0;
            
            if (strMaybeParseInt(tok, &num)) {
                fmtNum(&fmt, num);
            } else {
                fmtStr(&fmt, tok);
            }
        }

        assertTrue(t, strEquals(strFromBuf(buf), cases[i].expected), strC("Output matches pattern"));
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
        optPrintUsage(config, argv[0], "Run unit tests on the fmt library.");
        return help ? 0 : 99;
    }
  
    test_defn_t tests[] = {
        {"fmtNew basic usage", test_fmtNew},
        {"Formatting should replace holes", test_shouldReplaceHoles},
    };

    return (int)testRunner(tests, countof(tests), verbose);
}
