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

    bool out_raw;          /* Don't use quotes in output formatting */

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
        .out_raw = 0,
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
        optBool(&result.out_raw, 'r', "raw", "Output csv data directly, without quoting or column separators."),
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

        result.col_defns[result.num_col_defns].from = from;
        result.col_defns[result.num_col_defns].to = to;
        result.num_col_defns += 1;
    }

    return result;

fail:
    optPrintUsage(conf, 80);
    exit(99);
}

int main(int argc, char **argv) {
    config_t conf = getConfig(argc, argv);
    print_t p = printInitUnbuffered(1);
    for (size i=0; i<conf.num_col_defns; i++) {
        printC(p, "idx=");
        printNum(p, i);
        printC(p, ", from=");
        printNum(p, conf.col_defns[i].from);
        printC(p, ", to=");
        printNum(p, conf.col_defns[i].to);
        printC(p, "\n");
    }

    for (size i=conf.files_idx; i<argc; i++) {
        printC(p, "file: ");
        printStr(p, strFromC(argv[i]));
        printC(p, "\n");
    }

    if (conf.files_idx == argc) {
        printC(p, "file: <stdin>\n");
    }
    
    return 0;
}
