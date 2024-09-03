#include "../src/std.h"

#include <stdlib.h>

typedef struct {
    bool print_line_number;
    bool show_help;
    size rest_idx;
} conf_t;

conf_t getConfiguration(int argc, char **argv) {
    conf_t result = {0};
    
    opt_t opts[] = {
        optBool(&result.print_line_number, 'n', "line-number",
                "Print the line number along with the data"),
        optBool(&result.show_help, 'h', "help",
                "Show this help message"),
        optRest(&result.rest_idx, "<file>", 0),
    };

    opts_config_t config = optInit(opts, countof(opts));

    if (!optParse(config, argc, argv) || result.show_help) {
        optPrintUsage(config, argv[0],
                      "Demo application that shows an example of using the reader interface "
                      "in this particular library.");
        exit(result.show_help ? 0 : 99);
    }

    return result;
}

void printFile(print_t out, str_t filename, conf_t cnf) {
    int fd = 0;                 /* default to stdin */
    if (strNonEmpty(filename)) {
        if (!fdOpenReadOnly(filename, &fd)) {
            return;
        }
    }

    char read_data[1<<10] = {0};
    reader_t rdr = readInit(fd, bufFromC(read_data));

    size n = 1;
    while (readToStr(&rdr, strC("\n"))) {
        str_t line = iterStr(rdr.it);

        if (iterLast(rdr.it) && strIsEmpty(line)) {
            continue;
        }
        
        if (cnf.print_line_number) {
            printNum(out, n);
            printC(out, " ");
        }
        
        printStr(out, line);    /* newline is included */

        if (!readWasTruncated(rdr)) {
            n++;
        }
    }

    fdClose(fd);

    printFlush(out);
}

int main(int argc, char **argv) {
    conf_t cnf = getConfiguration(argc, argv);

    char out_data[1<<10] = {0};
    buf_t out_buf = bufFromC(out_data);
    print_t out = printInit(1, &out_buf);

    if (cnf.rest_idx == argc) {
        // No files provided on the commandline, use stdin
        printFile(out, (str_t){0}, cnf);
    }

    for (size i=cnf.rest_idx; i<argc; i++) {
        printFile(out, strFromC(argv[i]), cnf);
    }
}
