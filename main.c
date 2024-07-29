#include "std.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {

  char wdata[1<<10] = {0};
  buf wbuf = bufFromArray(wdata);
  printer stdout = printerFromFile(1, &wbuf);
  
  for (int i=1; i<argc; i++) {
    int fd = open(argv[i], O_RDONLY);
    if (fd < 0) {
      perror("can't open file");
      abort();
    }

    char rdata[1<<10] = {0};
    buf rbuf = bufFromArray(rdata);

    while (fdReadIntoBuf(fd, &rbuf)) {
      str line = strFirstLine(strFromBuf(rbuf));

      do {
	printStr(stdout, strFromC(argv[i]));
	printStr(stdout, strFromC(" ("));
	printStr(stdout, line);
	printStr(stdout, strFromC(")\n"));

	if (line.len < rbuf.len) {
	  bufDrop(&rbuf, line.len + 1);
	} else {
	  bufDrop(&rbuf, line.len);
	}

	/* maybe we can grab a new line without going to the file? */
	line = strFirstLine(strFromBuf(rbuf));
      } while (line.len < rbuf.len);
    }

    printFlush(stdout);

    close(fd);
  }
  return 0;
}


