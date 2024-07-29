CFLAGS=-g3 -Wall -Wextra -Wconversion -Wdouble-promotion -Wno-unused-parameter -Wno-unused-function -Wno-sign-conversion -fsanitize=undefined -fsanitize-trap

main: main.o str.o buf.o fd.o print.o
	gcc ${CFLAGS} main.o str.o buf.o fd.o print.o -o main

str.o: std.h str.c
	gcc ${CFLAGS} -c str.c

buf.o: std.h buf.c
	gcc ${CFLAGS} -c buf.c

fd.o: std.h fd.c
	gcc ${CFLAGS} -c fd.c

print.o: std.h print.c
	gcc ${CFLAGS} -c print.c

main.o: std.h main.c
	gcc ${CFLAGS} -c main.c
