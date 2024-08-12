#include <stdlib.h>

#include "std/std.h"

int main(int argc, char **argv) {
  bool verbose = 0;
  int energy = 5;
  str_t filename = strFromC(".");
  bool show_text = 0;
  str_t text = strFromC("");
  int columns = 80;
  bool show_help = 0;
  ptrdiff_t rest_idx;

  opt_t opts[10] = {0};
  opts_config_t config = {.opts = opts};

  optSummary(&config, 
	     "Usage: opt_demo [OPTIONS] <extra_args>...\n"
	     "\n"
	     "A command-line tool for demoing the opt-parsing aspect and "
	     "functionality😅 contained in this personal standard "
	     "library.\n\n");

  optBool(&config, &verbose, 'v', "verbose",
	  "Be extra loud or something when performing operations "
	  "here, except it really doesn't do a whole lot in this "
	  "particular example program.");
  optInt(&config, &energy, 'N', "energy", "ENERGY", "Energy count");
  optStr(&config, &filename, 'f', "filename", "FILE",
	 "Filename to do something to. Note that there's "
	 "some notes about this option that make it necessary "
	 "to show a bit more here.");
  optBool(&config, &show_text, 's', NULL,
	  "Short no long option for completeness.");
  optStr(&config, &text, 0, "text", "TEXT", "Text to word-wrap");
  optInt(&config, &columns, 'c', "columns", "NUM_COLUMNS",
	 "Number of columns to use when printing the --text option.");
  optBool(&config, &show_help, 'h', "help", "Display this help.");
  optRest(&config, &rest_idx);

  if (!optParse(config, argc, argv)) {
    optPrintUsage(config, 70);
    exit(99);
  } else if (show_help) {
    optPrintUsage(config, 70);
    exit(0);
  }

  printf("ok\n");

  print_t stdout = printerFromFile(STDOUT_FILENO, NULL);
  printStr(stdout, strFromC("verbose="));
  printU64(stdout, (uint64_t)verbose);
  printStr(stdout, strFromC(", energy="));
  printI64(stdout, energy);
  printStr(stdout, strFromC(", filename="));
  printStr(stdout, filename);
  printStr(stdout, strFromC(", rest_idx="));
  printU64(stdout, rest_idx);
  printFlush(stdout);

  for (ptrdiff_t i=rest_idx; i<argc; i++) {
    printf(" >> %s", argv[i]);
  }
  printf("\n");

  for (size i=0; i<columns; i++) {
    printChar(stdout, utf8CharFromC('v'));
  }
  printStr(stdout, strC("\n"));
  
  while (strNonEmpty(text)) {
    str_t line = strTakeLineWrapped(text, columns);
    printStr(stdout, strTrim(line, strC(" \n")));
    printStr(stdout, strC("\n"));
    text.beg = line.end;
    text = strTrimLeft(text, strC(" "));
  }
  
  return 0;
}
