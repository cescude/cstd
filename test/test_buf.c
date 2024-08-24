#include <stdlib.h>

#include "../src/std.h"

void test_bufAlloc_shouldAllocateWithoutClobbering(test_t *t) {
    char data[128] = {0};
    buf_t buf = bufFromC(data);

    uint8_t *a;
    uint16_t *b;
    uint32_t *c;
    uint64_t *d;

    a = bufAlloc(&buf, uint8_t);
    b = bufAlloc(&buf, uint16_t);
    c = bufAlloc(&buf, uint32_t);
    d = bufAlloc(&buf, uint64_t);

    *a = 1;
    *b = 2;
    *c = 3;
    *d = 4;

    assertTrue(t, *a == 1, strC("a is assigned correctly"));
    assertTrue(t, *b == 2, strC("b is assigned correctly"));
    assertTrue(t, *c == 3, strC("c is assigned correctly"));
    assertTrue(t, *d == 4, strC("d is assigned correctly"));

    bufClear(&buf);

    d = bufAlloc(&buf, uint64_t);
    c = bufAlloc(&buf, uint32_t);
    b = bufAlloc(&buf, uint16_t);
    a = bufAlloc(&buf, uint8_t);

    *d = 4;
    *c = 3;
    *b = 2;
    *a = 1;

    assertTrue(t, *a == 1, strC("a is assigned correctly (part 2)"));
    assertTrue(t, *b == 2, strC("b is assigned correctly (part 2)"));
    assertTrue(t, *c == 3, strC("c is assigned correctly (part 2)"));
    assertTrue(t, *d == 4, strC("d is assigned correctly (part 2)"));
}

void test_bufAlloc_shouldAlignPtrs(test_t *t) {
    char data[128] = {0};
    buf_t buf = bufFromC(data);

    for (size i=0; i<32; i++) {
        bufClear(&buf);
        /* char *chars = */ bufAllocN(&buf, char, i);
        uint32_t *test_int = bufAlloc(&buf, uint32_t);

        assertTrue(t, (uintptr_t)test_int % _Alignof(uint32_t) == 0, strC("test_int is aligned properly"));
    }
}

int main(int argc, char **argv) {
    bool verbose = false;
    bool help = false;
    opt_t opts[] = {
        optBool(&verbose, 'v', "verbose", "Be louder"),
        optBool(&help, 'h', "help", "Show this help information"),
    };

    opts_config_t config = optInit(
        opts, countof(opts),
        "USAGE: ./test_buf [OPTIONS]\n\n"
        "Run unit tests on the buf library.\n\n"
        );

    if (!optParse(config, argc, argv) || help) {
        optPrintUsage(config, 80);
        return help ? 0 : 99;
    }
  
    test_defn_t tests[] = {
        {"bufAlloc should allocate into different parts of memory", test_bufAlloc_shouldAllocateWithoutClobbering},
        {"bufAlloc should align pointers", test_bufAlloc_shouldAlignPtrs},
    };

    return (int)testRunner(tests, countof(tests), verbose);
}
