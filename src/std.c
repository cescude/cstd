#include "std.h"

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

void die(char *msg) {
    errlog(strFromC(msg));
    exit(99);
}

void assert(bool test, char *fail_msg) {
    if (!test) {
        die(fail_msg);
    }
}

void errlog(str_t msg) {
    print_t p = printInitUnbuffered(2);
    printStr(p, msg);
}
