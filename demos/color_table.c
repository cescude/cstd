#include "../src/std.h"

int main(int argc, char **argv) {
    fmt_t fmt = fmtToFile(1);
    
    for (size i=0; i<8; i++) {
        fmtStart(&fmt, "\x1b[3{}m {}={}  ");
        fmtNum(&fmt, i);
        fmtNum(&fmt, i);
        fmtStr(&fmt, strC("This color"));
    }

    fmtStart(&fmt, "\x1b[39m\n");
    return 0;
}
