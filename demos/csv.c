#include "../src/std.h"

#include <stdlib.h>

#define MAX_COL_DEFNS 100
#define MAX_COLUMNS 1000
#define BUFFER_SIZE (1<<10)     /* Bytes used for the input buffer, needs to be able to hold a full row */
#define OUT_BUFFER_SIZE (1<<10)

typedef struct {
    str_t inp_sep;
    str_t inp_quot;
    
    str_t out_sep;
    str_t out_quot;

    bool colorize;      /* Print each column with a different color */
    bool print_nicely;  /* Don't use quotes in output formatting */
    bool run_tests;

    struct {
        int from, to;          /* 1-based index of columns to print */
    } col_defns[MAX_COL_DEFNS];

    size num_col_defns;
    
    size files_idx;   /* index into argc where the file list begins */
} config_t;

bool parseColumnDefinitions(config_t *conf, str_t columns);

config_t getConfig(int argc, char **argv) {

    /* Setup defaults */
    config_t result = {
        .inp_sep = strC(","),
        .inp_quot = strC("\""),
        .out_sep = strC(","),
        .out_quot = strC("\""),
        .colorize = 0,
        .print_nicely = 0,
        .run_tests = 0,
        .col_defns = {},
        .num_col_defns = 0,
        .files_idx = argc,
    };

    str_t columns = {0}; /* column patterns--we'll process these into usable definitions */
    bool show_help = 0;
    
    opt_t opts[] = {
        optStr(&columns, 'c', "columns", "PATTERN",
               "Comma separated list of columns to print (ex. \"-c1,2,3\", "
               "\"-c1-3\", \"-c1-5,3-10\". If omitted, prints just the "
               "column names so you can figure this out."
        ),
        optStr(&result.inp_sep, 's', "ifs", "FIELD_SEPARATOR", "Input field separator used by the csv file(s), defaults to ','."),
        optStr(&result.out_sep, 'F', "ofs", "FIELD_SEPARATOR", "Output field separator to use when processing csv file(s). Defaults to '\"'."),
        optStr(&result.inp_quot, 'q', "input-quote", "QUOTE", "Input character used by the csv file(s) for quoting columns. Defaults to '\"'."),
        optStr(&result.out_quot, 'Q', "output-quote", "QUOTE", "Output character used for quoting csv columns. Defaults to ','."),
        optBool(&result.colorize, 'C', "color", "Colorize the output"),
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

    if (!parseColumnDefinitions(&result, columns)) {
        goto fail;
    }

    return result;

fail:
    optPrintUsage(conf, 80);
    exit(99);
}

bool parseColumnDefinitions(config_t *result, str_t columns) {

    /*
      Now we need to convert the user-provided "columns" string into
      something we can use directly (ie., zero-based ranges).

      For example:

      csv -c 1,8,2-4,20-

      ...becomes...

      [ { .from=0, .to=1 },
        { .from=7, .to=8 },
        { .from=1, .to=4 },
        { .from=19, .to=MAX_COLUMNS } ]

      To do this we first split the pattern on commas, and then split
      on dashes.
    */
    
    iter_t it = iterFromStr(columns);
    while (iterTakeToStr(&it, strC(",")) && result->num_col_defns < MAX_COL_DEFNS) {
        str_t pattern = strTrimRight(iterStr(it), strC(","));

        /*
          Can be:
          1) a single number,
          2) two numbers separated by a dash (eg. 1-5)
          3) a single number followed by a dash (eg. 10-)
        */

        iter_t pit = iterFromStr(pattern);
        if (!iterTakeToStr(&pit, strC("-"))) {
            return false;
        }

        int from = 0;
        int to = 0;
        
        str_t num_str = strTrimRight(iterStr(pit), strC("-"));
        if (!strMaybeParseInt(num_str, &from)) {
            return false;
        }

        from -= 1;              /* User is 1 based, we're 0 based */

        if (iterLast(pit)) {
            /*
              Just a single number, eg. "10". Bump by one because the
              "from" isn't inclusive, it represents an exclusive
              boundary.
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
                return false;
            } else {
                /*
                  Two numbers, separated by a dash, eg. "10-15". We
                  don't need to adjust the 'to', since it's final
                  value is:

                  1) to = to-1; <- adjust because we're zero based
                  2) to = to+1; <- adjust because to is exclusive and we need to bump it
                */
                //to = to - 1 + 1;
            }
        }

        if (from >= 0 && to > from) {
            result->col_defns[result->num_col_defns].from = from;
            result->col_defns[result->num_col_defns].to = to;
            result->num_col_defns += 1;
        } else {
            return false;
        }
    }

    return true;
}

int runTests();
void processCsv(reader_t rdr, str_t *columns, config_t conf);

str_t columns[MAX_COLUMNS] = {0};
byte read_bytes[BUFFER_SIZE] = {0};
buf_t read_buf = bufFromC(read_bytes);

byte write_bytes[OUT_BUFFER_SIZE] = {0};
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

void processCsvHeader(reader_t rdr, str_t *columns, config_t conf);
void processCsvNormal(reader_t rdr, str_t *columns, config_t conf);

void processCsv(reader_t rdr, str_t *columns, config_t conf) {
    if (conf.num_col_defns > 0) {
        processCsvNormal(rdr, columns, conf);
    } else {
        processCsvHeader(rdr, columns, conf);
    }
}


size parseColumns(str_t line, str_t *columns, str_t iquot, str_t isep);
void printColumn(print_t out, str_t col, bool nice, str_t iquot, str_t oquot);

void processCsvHeader(reader_t rdr, str_t *columns, config_t conf) {
    if (readToStr(&rdr, strC("\n"))) {
        str_t line = strTrimRight(readStr(rdr), strC("\r\n"));
        if (strBytesLen(line) == rdr.buffer->cap) {
            die(strC("Need to resize the buffer I think"));
        }

        size num_columns = parseColumns(line, columns, conf.inp_quot, conf.inp_sep);

        if (conf.num_col_defns == 0) {
            for (size i=0; i<num_columns; i++) {
                printNum(out, i+1);
                printC(out, ") ");
                printColumn(out, columns[i], true, conf.inp_quot, conf.out_quot);
                printC(out, "\n");
            }
        }
    }

    printFlush(out);
}

void processCsvNormal(reader_t rdr, str_t *columns, config_t conf) {
    assert(conf.num_col_defns > 0, strC("Please don't call this without any column definitions!"));
    
    while (readToStr(&rdr, strC("\n"))) {
        str_t line = strTrimRight(readStr(rdr), strC("\r\n"));
        if (strBytesLen(line) == rdr.buffer->cap) {
            die(strC("Need to resize the buffer I think"));
        }

        size num_columns = parseColumns(line, columns, conf.inp_quot, conf.inp_sep);
        if (num_columns < 1) continue;

        printColumn(
            out,
            columns[conf.col_defns[0].from],
            conf.print_nicely,
            conf.inp_quot,
            conf.out_quot
        );

        for (size i=0; i<conf.num_col_defns; i++) {

            /* For each pattern of columns to print...print them out! */
            size from = conf.col_defns[i].from;
            size to = conf.col_defns[i].to;

            /*
              Ok, so we're adding i==0 so that the very first column
              is skipped, so we can avoid starting things off with an
              output separator.
            */
            for (size j = from + (i==0); j<to && j<num_columns; j++) {
                printStr(out, conf.out_sep);
                printColumn(out, columns[j], conf.print_nicely, conf.inp_quot, conf.out_quot);
            }
        }

        printStr(out, strC("\n"));
    }

    printFlush(out);
}

// TODO add some dang tests me
void printColumn(print_t out, str_t col, bool nice, str_t iquot, str_t oquot) {
    bool print_tail_quote = 0;
    
    if (strStartsWith(col, iquot)) {
        assert(strEndsWith(col, iquot), strC("String should start AND end with a quote!"));

        /* Drop the prefix & suffix */
        col.beg += strBytesLen(iquot);
        col.end -= strBytesLen(iquot);

        if (!nice) {
            printStr(out, oquot);
            print_tail_quote = true;
        }
    }

    str_t segment = (str_t){col.beg, col.beg};
    while (strNonEmpty(col)) {
        if (strStartsWith(col, iquot)) {
            printStr(out, segment);
            printStr(out, oquot);
            
            col.beg += strBytesLen(iquot);
            segment.beg = col.beg;
            segment.end = col.beg;

            if (nice && strStartsWith(col, iquot)) {
                /*
                  Two quotes in a row? That's just an escape! (ie.,
                  don't print it)
                */
                col.beg += strBytesLen(iquot);
                segment.beg = col.beg;
                segment.end = col.beg;
            }
        } else {
            strNextChar(&col);
            segment.end = col.beg;
        }
    }
    
    printStr(out, segment);

    if (print_tail_quote) {
        printStr(out, oquot);
    }
}

str_t takeUnQuotedColumn(str_t line, str_t sep);
str_t takeQuotedColumn(str_t line, str_t quot, str_t sep);

size parseColumns(str_t line, str_t *columns, str_t iquot, str_t isep) {
    int num_columns = 0;
    
    while (strNonEmpty(line) && num_columns < MAX_COLUMNS) {

        str_t c = strStartsWith(line, iquot)
            ? takeQuotedColumn(line, iquot, isep)
            : takeUnQuotedColumn(line, isep);

        if (strEndsWith(c, isep)) {
            columns[num_columns++] = (str_t){
                .beg = c.beg,
                .end = c.end - strBytesLen(isep),
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

void test_parseColumnDefinitions_shouldParseValidPatterns(test_t *t) {
    struct { str_t pattern; size idx, num_defns, from, to; } cases[] = {
        { strC("1"), 0, 1, 0, 1 },
        { strC("1-10"), 0, 1, 0, 10 },
        { strC("1-"), 0, 1, 0, MAX_COLUMNS },
        { strC("1-,5-20"), 1, 2, 4, 20 },
    };

    for (size i=0; i<countof(cases); i++) {
        config_t c = {0};

        assertTrue(t, parseColumnDefinitions(&c, cases[i].pattern), strC("Should parse columns"));
        assertTrue(t, cases[i].num_defns == c.num_col_defns, strC("Should parse the correct number"));
        assertTrue(t, c.col_defns[cases[i].idx].from == cases[i].from, strC("'from' should match"));
        assertTrue(t, c.col_defns[cases[i].idx].to == cases[i].to, strC("'to' should match"));
    }
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
        {"parseColumnDefinitions should parse valid patterns", test_parseColumnDefinitions_shouldParseValidPatterns},
        {"takeQuotedColumn should parse middle columns", test_takeQuotedColumn_shouldParseMiddleColumns},
        {"takeQuotedColumn should parse end columns", test_takeQuotedColumn_shouldParseEndColumns},
    };

    return (int)testRunner(tests, countof(tests), true);
}
