CFLAGS=-g3 -Wall -Wextra -Wconversion -Wdouble-promotion -Wno-unused-parameter -Wno-unused-function -Wno-sign-conversion -fsanitize=undefined -fsanitize-trap
#CFLAGS=-O3

all: cat main

main: main.o std.o 
	gcc ${CFLAGS} main.o std.o -o main

std.o: src/std/std.h src/std/std.c
	gcc ${CFLAGS} -c src/std/std.c

main.o: src/std/std.h src/main.c
	gcc ${CFLAGS} -c src/main.c

cat: std.o src/cat.c
	gcc ${CFLAGS} src/cat.c -o cat std.o

clean:
	rm *.o
	rm main cat

