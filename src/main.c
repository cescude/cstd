#include "std/std.h"

#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {

  char wdata[1<<10] = {0};
  buf_t wbuf = bufFromArray(wdata);
  print_t stdout = printerFromFile(STDOUT_FILENO, &wbuf);
  
  for (int i=1; i<argc; i++) {
    int fd = open(argv[i], O_RDONLY);
    if (fd < 0) {
      perror("can't open file");
      abort();
    }

    char rdata[1<<10] = {0};
    buf_t rbuf = bufFromArray(rdata);

    while (fdReadIntoBuf(fd, &rbuf)) {
      str_t line = strFirstLine(strFromBuf(rbuf));

      do {
	printU64F(stdout, strHash_djb2(line), (format_t){.width=20, .right=1});
	printCharF(stdout, utf8CharFromC(' '), (format_t){.width=5});
	printStr(stdout, strFromC(argv[i]));
	printStr(stdout, strC(" ("));
	printStr(stdout, line);
	printStr(stdout, strC(")\n"));

	if (line.len < rbuf.len) {
	  bufDropBytes(&rbuf, line.len + 1);
	} else {
	  bufDropBytes(&rbuf, line.len);
	}

	/* maybe we can grab a new line without going to the file? */
	line = strFirstLine(strFromBuf(rbuf));
      } while (line.len < rbuf.len);
    }

    close(fd);
  }

  printU64(stdout, 0xffffFFFFffffFFFF);
  printChar(stdout, utf8CharFromC('\n'));
  
  printFlush(stdout);

  return 0;
}


