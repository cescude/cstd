#include <stdlib.h>

#include "std/std.h"

int main(int argc, char **argv) {
  bool verbose = 0;
  int energy = 5;
  str_t filename = strFromC(".");
  ptrdiff_t rest_idx;

  opt_t opts[10] = {0};
  opts_config_t config = {
    .opts = opts,
  };

  optBool(&config, 'v', "verbose", &verbose);
  optInt(&config, 'N', "energy", &energy);
  optStr(&config, 'f', "filename", &filename);
  optRest(&config, &rest_idx);

  if (!optParse(config, argc, argv)) {
    printf("optPrintHelp(config, stdout, 80)\n");
    exit(99);
  }

  printf("ok\n");

  print_t stdout = printerFromFile(STDOUT_FILENO, NULL);
  printStr(stdout, strFromC("verbose="));
  printU64(stdout, (uint64_t)verbose);
  printStr(stdout, strFromC(", energy="));
  printU64(stdout, energy);
  printStr(stdout, strFromC(", filename="));
  printStr(stdout, filename);
  printStr(stdout, strFromC(", rest_idx="));
  printU64(stdout, rest_idx);
  printFlush(stdout);

  for (ptrdiff_t i=rest_idx; i<argc; i++) {
    printf("-- %s ", argv[i]);
  }
  printf("\n");

  return 0;
}
