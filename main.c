#include "std.h"

#include <stdio.h>
#include <stdlib.h>	

int main(int argc, char **argv) {
  for (int i=1; i<argc; i++) {
    int fd = open(argv[i], O_RDONLY);
    if (fd < 0) {
      perror("can't open file");
      abort();
    }

    char data[1<<10] = {0};
    buf buf = {data, 0, sizeof(data)};
    
    while (readToBufFromFile(&buf, fd) && buf.len) {
      str line = strTakeLine(strFromBuf(buf));

      do {
	if ( !writeStrToFile(line, 1) || !writeCharToFile('\n', 1) ) {
	  perror("can't write to stdout");
	  abort();
	}

	if (line.len < buf.len) {
	  bufDropBytes(&buf, line.len + 1);
	} else {
	  bufDropBytes(&buf, line.len);
	}

	/* maybe we can grab a new line without going to the file? */
	line = strTakeLine(strFromBuf(buf));
      } while (line.len < buf.len);
    }

    close(fd);
  }
  return 0;
}
