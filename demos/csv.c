#include "../src/std.h"

#include <stdlib.h>

#define MAX_COL_DEFNS 100
#define MAX_COLUMNS 1000
#define BUFFER_SIZE (1<<12)     /* Bytes used for the input buffer, needs to be able to hold a full row */

typedef struct {
    str_t inp_sep;
    str_t inp_quot;
    
    str_t out_sep;
    str_t out_quot;

    bool print_nicely;          /* Don't use quotes in output formatting */
    bool run_tests;

    struct {
        int from, to;          /* 1-based index of columns to print */
    } col_defns[MAX_COL_DEFNS];

    size num_col_defns;
    
    size files_idx;    /* index into argc where the file list begins */
} config_t;

config_t getConfig(int argc, char **argv) {

    /* Setup defaults */
    config_t result = {
        .inp_sep = strC(","),
        .inp_quot = strC("\""),
        .out_sep = strC(","),
        .out_quot = strC("\""),
        .print_nicely = 0,
        .run_tests = 0,
        .col_defns = {},
        .num_col_defns = 0,
        .files_idx = argc,
    };

    str_t columns = {0}; /* column patterns--we'll process these into usable definitions */
    bool show_help = 0;
    
    opt_t opts[] = {
        optStr(&columns, 'c', "columns", "PATTERN", "Comma separated list of columns to print (ex. \"-c1,2,3\", \"-c1-3\", \"-c1-5,3-10\""),
        optStr(&result.inp_sep, 's', "ifs", "FIELD_SEPARATOR", "Input field separator used by the csv file(s), defaults to ','."),
        optStr(&result.inp_quot, 'q', "input-quote", "QUOTE", "Input character used by the csv file(s) for quoting columns. Defaults to '\"'."),
        optStr(&result.out_sep, 'F', "ofs", "FIELD_SEPARATOR", "Output field separator to use when processing csv file(s). Defaults to '\"'."),
        optStr(&result.out_quot, 'Q', "output-quote", "QUOTE", "Output character used for quoting csv columns. Defaults to ','."),
        optBool(&result.print_nicely, 'n', "nice", "Output csv data directly, without quoting or column separators."),
        optBool(&result.run_tests, 0, "run-tests", "Runs unit tests on the program"),
        optBool(&show_help, 'h', "help", "Show this help."),
        optRest(&result.files_idx),
    };

    opts_config_t conf = optInit(
        opts, countof(opts),
        "Usage: csv [OPTIONS] [FILE...]\n"
        "\n"
        "Process one or more CSV files according to the provided options. If no "
        "options are specified, print the column list.");


    if (!optParse(conf, argc, argv)) {
        goto fail;
    } else if (show_help) {
        optPrintUsage(conf, 80);
        exit(0);
    }

    /*
      Now we need to convert the user-provided "columns" string into
      something we can use directly.

      For example:

        csv -c 1,8,2-4,20-

      ...becomes...

        [ { .from=1, .to=2 },
          { .from=8, .to=9 },
          { .from=2, .to=5 },
          { .from=20, .to=MAX_COLUMNS } ]

      To do this we first split the pattern on commas, and then split
      on dashes.
    */
    
    iter_t it = iterFromStr(columns);
    while (iterTakeToStr(&it, strC(","))) {
        str_t pattern = strTrimRight(iterStr(it), strC(","));

        /*
          Can be:
          1) a single number,
          2) two numbers separated by a dash (eg. 1-5)
          3) a single number followed by a dash (eg. 10-)
        */

        iter_t pit = iterFromStr(pattern);
        if (!iterTakeToStr(&pit, strC("-"))) {
            goto fail;
        }

        int from = 0;
        int to = 0;
        
        str_t num_str = strTrimRight(iterStr(pit), strC("-"));
        if (!strMaybeParseInt(num_str, &from)) {
            goto fail;
        }

        if (iterLast(pit)) {
            /*
              Just a single number, eg. "10". So we're printing a
              range from->to+1
            */
            to = from + 1;
        } else {
            num_str = iterStrRest(pit); /* grab everything after the dash */
            if (strIsEmpty(num_str)) {
                /*
                  A single number followed by a dash, eg. "10-". So
                  we're printing a range, from to->rest of columns.
                 */
                to = MAX_COLUMNS;
            } else if (!strMaybeParseInt(num_str, &to)) {
                goto fail;
            } else {
                /*
                  Two numbers, separated by a dash, eg. "10-15". Since
                  the input pattern is "inclusive" (ie., the above
                  prints 10,11,12,13,14,15), we need to bump `to`
                  since internally we represent the range exclusively
                  (ie., we print up-to-but-not-including the column
                  identified by "to").
                */
                to += 1;
            }
        }

        result.col_defns[result.num_col_defns].from = from - 1;
        result.col_defns[result.num_col_defns].to = to - 1;
        result.num_col_defns += 1;
    }

    return result;

fail:
    optPrintUsage(conf, 80);
    exit(99);
}

int runTests();
void processCsv(reader_t rdr, str_t *columns, config_t conf);

str_t columns[MAX_COLUMNS] = {0};
byte read_bytes[BUFFER_SIZE] = {0};
buf_t read_buf = bufFromC(read_bytes);

byte write_bytes[1<<10] = {0};
buf_t write_buf = bufFromC(write_bytes);
print_t out = printInit(1, &write_buf);

int main(int argc, char **argv) {
    config_t conf = getConfig(argc, argv);
    if (conf.run_tests) {
        exit(runTests() > 0);
    }
    
    for (size i=conf.files_idx; i<argc; i++) {
        int fd;
        if (fdOpen(strFromC(argv[i]), &fd, O_RDONLY)) {
            reader_t rdr = readInit(fd, &read_buf);
            processCsv(rdr, columns, conf);
            close(fd);
        }
    }

    if (conf.files_idx == argc) {
        reader_t rdr = readInit(0, &read_buf);
        processCsv(rdr, columns, conf);
    }
    
    return 0;
}

size parseColumns(str_t line, str_t *columns, config_t conf);
void printNiceColumn(print_t out, str_t c, str_t sep);

void processCsv(reader_t rdr, str_t *columns, config_t conf) {
    while (readToStr(&rdr, strC("\n"))) {
        str_t line = strTrimRight(readStr(rdr), strC("\r\n"));
        if (strBytesLen(line) == rdr.buffer->cap) {
            die(strC("Need to resize the buffer I think"));
        }

        size num_columns = parseColumns(line, columns, conf);

        /*
          If the user hasn't specified any columns to print, well,
          print the column names instead!
         */
        if (conf.num_col_defns == 0) {
            for (size i=0; i<num_columns; i++) {
                printNum(out, i+1);
                printC(out, ") ");
                printNiceColumn(out, columns[i], conf.inp_quot);
                printC(out, "\n");
            }
            
            break;
        }

        bool first = 1;
        for (size i=0; i<conf.num_col_defns; i++) {

            /* For each pattern of columns to print...print them out! */
            
            for (size j=conf.col_defns[i].from; j<conf.col_defns[i].to && j<num_columns; j++) {
                if (first) {
                    first = 0;
                } else {
                    printStr(out, conf.out_sep);
                }
                
                if (conf.print_nicely) {
                    printNiceColumn(out, columns[j], conf.inp_quot);
                } else {
                    printStr(out, columns[j]);
                }
            }
        }

        printStr(out, strC("\n"));
    }

    printFlush(out);
}

void printNiceColumn(print_t out, str_t c, str_t quot) {
    if (strStartsWith(c, quot)) {
        assert(strEndsWith(c, quot), strC("String should start AND end with a quote!"));

        /* Drop the prefix & suffix */
        c.beg += strBytesLen(quot);
        c.end -= strBytesLen(quot);
        
        iter_t it = iterFromStr(c);
        while (iterTakeToStr(&it, quot)) {
            str_t segment = iterStr(it);
            printStr(out, segment);

            if (strStartsWith(iterStrRest(it), quot)) {
                /* Oh, we have two quotes in a row? Skip the second! */
                assert(
                    iterTakeToStr(&it, quot), strC("Should have more data to parse!")
                );
            }
        }
    } else {
        printStr(out, c);
    }
}

str_t takeUnQuotedColumn(str_t line, str_t sep);
str_t takeQuotedColumn(str_t line, str_t quot, str_t sep);

size parseColumns(str_t line, str_t *columns, config_t conf) {
    int num_columns = 0;
    
    while (strNonEmpty(line)) {

        str_t c = strStartsWith(line, conf.inp_quot)
            ? takeQuotedColumn(line, conf.inp_quot, conf.inp_sep)
            : takeUnQuotedColumn(line, conf.inp_sep);

        if (strEndsWith(c, conf.inp_sep)) {
            columns[num_columns++] = (str_t){
                .beg = c.beg,
                .end = c.end - strBytesLen(conf.inp_sep),
            };
        } else {
            columns[num_columns++] = c;
        }

        line.beg = c.end;
    }
    
    return num_columns;
}

/*
  Returns the column followed by sep (or not, if last column). For example:

    one,two,three

  The first two columns will include the trailing comma, the last will
  not.
*/
str_t takeUnQuotedColumn(str_t line, str_t sep) {
    return strTakeToStr(line, sep);
}

str_t takeQuotedColumn(str_t line, str_t quot, str_t sep) {
    assert(strStartsWith(line, quot), strC("Scanned line doesn't start with a quote"));
    
    str_t cursor = (str_t){
        .beg = line.beg + strBytesLen(quot),
        .end = line.end
    };
    
    while (1) {
        str_t substr = strTakeToStr(cursor, quot);
        if (substr.end == line.end) {
            return line;
        }

        cursor.beg = substr.end;

        if (strStartsWith(cursor, quot)) {
            /* Two quotes in a row, that's an escape! */
            cursor.beg += strBytesLen(quot);
        } else if (strStartsWith(cursor, sep)) {
            return (str_t){
                .beg = line.beg,
                .end = cursor.beg + strBytesLen(sep),
            };
        }
    }

    assert(false, strC("Unreachable"));
}

void test_takeQuotedColumn_shouldParseMiddleColumns(test_t *t) {
    str_t pipe = strC("|");
    str_t comma = strC(",");
    
    struct { str_t line, q, s, expected; } cases[] = {
        { strC("|one|,tw"), pipe, comma, strC("|one|,") },
        { strC("|on|one|,tw"), pipe, comma, strC("|on|one|,") },
        { strC("|on||one|,tw"), pipe, comma, strC("|on||one|,") },
        { strC("|one,two,three|,tw"), pipe, comma, strC("|one,two,three|,") },
        { strC("|one||,|two,three|,tw"), pipe, comma, strC("|one||,|two,three|,") },
    };

    for (size i=0; i<countof(cases); i++) {
        str_t c = takeQuotedColumn(cases[i].line, cases[i].q, cases[i].s);
        assertTrue(t, strEquals(c, cases[i].expected), cases[i].line);
    }
}

void test_takeQuotedColumn_shouldParseEndColumns(test_t *t) {
    str_t pipe = strC("|");
    str_t comma = strC(",");
    
    struct { str_t line, q, s, expected; } cases[] = {
        { strC("|one|\n"), pipe, comma, strC("|one|\n") },
        { strC("|on|one|\n"), pipe, comma, strC("|on|one|\n") },
        { strC("|on||one|\n"), pipe, comma, strC("|on||one|\n") },
        { strC("|one,two,three|\n"), pipe, comma, strC("|one,two,three|\n") },
        { strC("|one||,|two,three|\n"), pipe, comma, strC("|one||,|two,three|\n") },
    };

    for (size i=0; i<countof(cases); i++) {
        str_t c = takeQuotedColumn(cases[i].line, cases[i].q, cases[i].s);
        assertTrue(t, strEquals(c, cases[i].expected), cases[i].line);
    }
}

int runTests() {
    test_defn_t tests[] = {
        {"takeQuotedColumn should parse middle columns", test_takeQuotedColumn_shouldParseMiddleColumns},
        {"takeQuotedColumn should parse end columns", test_takeQuotedColumn_shouldParseEndColumns},
    };

    return (int)testRunner(tests, countof(tests), true);
}
