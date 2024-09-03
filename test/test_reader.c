#include <stdlib.h>
#include <unistd.h>

#include "../src/std.h"

#define nilstr (str_t){0}

void initPipe(int *writer, int *reader) {
    int fildes[2] = {0};
    if (pipe(fildes) < 0) {
        die("Unable to setup test");
    }
    *reader = fildes[0];
    *writer = fildes[1];
}

void test_readToStr_shouldHandleBuffering(test_t *t) {
    struct { str_t src, sep; size expected_segments; } cases[] = {
        { strC("one two three four five six seven eight nine ten eleven twelve"), strC(" "), 12 },
        { strC("one😅😅two😅😅three"), strC("😅😅"), 3 },
        { strC("small"), strC("anything!"), 1 },
    };
    
    for (size i=0; i<countof(cases); i++) {
        int w = 0, r = 0;
        initPipe(&w, &r);
        
        print_t prn = printInitUnbuffered(w);
        printStr(prn, cases[i].src);
        close(w);

        char read_data[16] = {0};
        buf_t read_buf = bufFromC(read_data);
        reader_t rdr = readInit(r, read_buf);
    
        size segments = 0;
        while (readToStr(&rdr, cases[i].sep)) {
            segments++;
        }

        close(r);

        assertTrue(t, segments == cases[i].expected_segments, cases[i].src);
    }
}

void test_readToStr_shouldHandleTruncatedScans(test_t *t) {
    struct { str_t src, sep; size truncated_idx; } cases[] = {
        { strC("one two 1234567890ABCDEFGHIJKL four"), strC(" "), 3 },
    };

    for (size i=0; i<countof(cases); i++) {
        int w = 0, r = 0;
        initPipe(&w, &r);

        print_t prn = printInitUnbuffered(w);
        printStr(prn, cases[i].src);
        close(w);

        char read_data[16] = {0};
        buf_t read_buf = bufFromC(read_data);
        reader_t rdr = readInit(r, read_buf);

        size segments = 0;
        while (readToStr(&rdr, cases[i].sep)) {
            segments++;
            bool should_be_truncated = segments == cases[i].truncated_idx;
            assertTrue(t, readWasTruncated(rdr) == should_be_truncated, cases[i].src);
        }

        close(r);
    }
}

void test_readToAnyChar_shouldHandleBuffering(test_t *t) {
    struct { str_t src, chars; size expected_segments; } cases[] = {
        { strC("one two|three/four/five six|seven eight|nine|ten eleven/twelve"), strC(" |/"), 12 },
        { strC("one😅two/three😅four"), strC("😅/"), 4 },
        { strC("small"), strC("! \t😅"), 1 },
    };
    
    for (size i=0; i<countof(cases); i++) {
        int w = 0, r = 0;
        initPipe(&w, &r);
        
        print_t prn = printInitUnbuffered(w);
        printStr(prn, cases[i].src);
        close(w);

        char read_data[16] = {0};
        reader_t rdr = readInit(r, bufFromC(read_data));
    
        size segments = 0;
        while (readToAnyChar(&rdr, cases[i].chars)) {
            segments++;
        }

        close(r);

        assertTrue(t, segments == cases[i].expected_segments, cases[i].src);
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
        optPrintUsage(config, argv[0], "Run unit tests on the reader library.");
        return help ? 0 : 99;
    }
  
    test_defn_t tests[] = {
        {test_readToStr_shouldHandleBuffering, "readToStr should handle buffering"},
        {test_readToStr_shouldHandleTruncatedScans, "readToStr should handle truncated scans"},
        {test_readToAnyChar_shouldHandleBuffering, "readToAnyChar should handle buffering"},
    };

    return (int)testRunner(tests, countof(tests), verbose);
}
