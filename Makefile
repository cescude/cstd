CFLAGS=-g3 -Wall -Wextra -Wconversion -Wdouble-promotion -Wno-unused-parameter -Wno-unused-function -Wno-sign-conversion -fsanitize=undefined -fsanitize-trap
#CFLAGS=-O3

all: cat main test_opt

libstd.a: std.o opt.o utf8.o
	ar rcs libstd.a std.o opt.o utf8.o

opt.o: src/std/std.h src/std/opt.h src/std/opt.c
	gcc ${CFLAGS} -c src/std/opt.c

utf8.o: src/std/std.h src/std/utf8.h src/std/utf8.c
	gcc ${CFLAGS} -c src/std/utf8.c

std.o: src/std/std.h src/std/std.c
	gcc ${CFLAGS} -c src/std/std.c

main: libstd.a main.o
	gcc ${CFLAGS} main.o -L. -lstd -o main

main.o: src/std/std.h src/main.c
	gcc ${CFLAGS} -c src/main.c

cat: libstd.a src/cat.c
	gcc ${CFLAGS} src/cat.c -L. -lstd -o cat

test_opt.o: libstd.a src/test_opt.c
	gcc ${CFLAGS} src/test_opt.c -c

test_opt: libstd.a test_opt.o
	gcc ${CFLAGS} test_opt.o -L. -lstd -o test_opt

clean:
	rm *.o
	rm main cat test_opt

