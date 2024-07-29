#include "std/std.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
  char data[1<<10];
  buf_t buffer = bufFromArray(data);

  for (int i=1; i<argc; i++) {
    int fd = open(argv[i], O_RDONLY);
    if (fd < 0) {
      perror("");
      return 1;
    }

    while (fdReadIntoBuf(fd, &buffer) && fdFlush(STDOUT_FILENO, &buffer)) {
      /* empty */
    }
  }

  return 0;
}
