#include <stdlib.h>
#include <fcntl.h>

#include "../src/std.h"

int main(int argc, char **argv) {
    bool verbose = 0;
    int energy = 5;
    str_t filename = strC("");
    bool show_text = 0;
    str_t text = strC("");
    int columns = 80;
    bool show_help = 0;
    str_t name = strC("");
    str_t nickname = strC("");
    ptrdiff_t rest_idx;

    opt_t opts[] = {
        optBool(&verbose, 'v', "verbose",
                "Be extra loud or something when performing operations "
                "here, except it really doesn't do a whole lot in this "
                "particular example program."),
        optInt(&energy, 'N', "energy", "ENERGY", "Energy count"),
        optStr(&filename, 'f', "filename", "FILE",
               "Filename to do something to. Note that there's "
               "some notes about this option that make it necessary "
               "to show a bit more here."),
        optBool(&show_text, 's', NULL, "Short no long option for completeness."),
        optInt(&columns, 'c', "columns", "NUM_COLUMNS",
               "Number of columns to use when printing the --text option."),
        optStr(&text, 0, "text", "TEXT", "Text to word-wrap."),
        optBool(&show_help, 'h', "help", "Display this help."),
        optArg(&name, "<name>", "Name to pass to whatever."),
        optArg(&nickname, "<nick>", "Nickname to pass to whatever."),
        optRest(&rest_idx, "<extra_args>", "Extra (optional!) arguments that you can pass if you so choose!"),
    };
  
    opts_config_t config = optInit(opts, countof(opts));

    if (!optParse(config, argc, argv) || show_help) {
        optPrintUsage(
            config,
            "opt_demo",
            "A command-line tool for demoing the opt-parsing aspect and "
            "functionality 😅 contained in this non-standard library."
        );
        exit(show_help ? 0 : 99);
    }

    return 0;
}
