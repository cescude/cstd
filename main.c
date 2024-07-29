#include "std.h"

#include <stdio.h>
#include <stdlib.h>

char wdata[1<<10] = {0};
buf wbuf = {wdata, 0, sizeof(wdata)};

int main(int argc, char **argv) {
  
  for (int i=1; i<argc; i++) {
    int fd = open(argv[i], O_RDONLY);
    if (fd < 0) {
      perror("can't open file");
      abort();
    }

    char rdata[1<<10] = {0};
    buf rbuf = {rdata, 0, sizeof(rdata)};

    while (fdFillBuf(fd, &rbuf) || rbuf.len) {
      str line = strFirstLine(strFromBuf(rbuf));

      do {
	fdPrintStr(1, &wbuf, strFromC("("));
	fdPrintStr(1, &wbuf, line);
	fdPrintStr(1, &wbuf, strFromC(")\n"));

	if (line.len < rbuf.len) {
	  bufDrop(&rbuf, line.len + 1);
	} else {
	  bufDrop(&rbuf, line.len);
	}

	/* maybe we can grab a new line without going to the file? */
	line = strFirstLine(strFromBuf(rbuf));
      } while (line.len < rbuf.len);
    }

    fdFlush(1, &wbuf);

    close(fd);
  }
  return 0;
}
