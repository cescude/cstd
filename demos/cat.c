#include "std/std.h"

#include <stdio.h>
#include <stdlib.h>

char data[1<<10];
buf_t buffer = bufFromArray(data);

void pipe_data(int fd) {
  while (fdReadIntoBuf(fd, &buffer) && fdFlush(STDOUT_FILENO, &buffer)) {
    /* empty */
  }
}

int main(int argc, char **argv) {

  for (int i=1; i<argc; i++) {
    int fd = open(argv[i], O_RDONLY);
    if (fd < 0) {
      perror("");
      return 1;
    }
    
    pipe_data(fd);
    
    close(fd);
  }

  if (argc == 1) {
    pipe_data(STDIN_FILENO);
  }

  return 0;
}
