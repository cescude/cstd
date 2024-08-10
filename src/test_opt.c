#include <stdlib.h>

#include "std/std.h"

int main(int argc, char **argv) {
  bool verbose = 0;
  int energy = 5;
  str_t filename = strFromC(".");
  str_t text = strFromC("");
  int columns = 80;
  bool show_help = 0;
  ptrdiff_t rest_idx;

  opt_t opts[10] = {0};
  opts_config_t config = {
    .opts = opts,
  };

  optSummary(
	     &config,
	     "A command-line benchmarking tool.\n"
	     "\n"
	     "Usage: test_opt\n\n"
	     );

  optBool(&config, &verbose, 'v', "verbose",
	  "Be extra loud or something when performing operations "
	  "here, except it really doesn't do a whole lot in this "
	  "particular example program");
  optInt(&config, &energy, 'N', "energy", "Energy count");
  optStr(&config, &filename, 'f', "filename",
	 "Filename to do something to. Note that there's "
	 "some notes about this option that make it necessary "
	 "to show a bit more here.");
  optStr(&config, &text, 0, "text", "Text to word-wrap");
  optInt(&config, &columns, 'c', "columns",
	 "Number of columns to use when printing the --text option.");
  optBool(&config, &show_help, 'h', "help", "Display this help");
  optRest(&config, &rest_idx);

  if (!optParse(config, argc, argv)) {
    printf("optParse failed!\n");
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

  while (text.len) {
    str_t line = strTakeLineWrapped(text, columns);
    printStr(stdout, strFromC("["));
    printStr(stdout, line);
    printStr(stdout, strFromC("]\n"));
    text = strDropBytes(text, line.len);
    text = strSkipByte(text, ' ');
  }
  
  return 0;
}
