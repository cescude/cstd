CC=cc
CFLAGS=-g3 -Wall -Wextra -Wconversion -Wdouble-promotion -Wno-unused-parameter -Wno-unused-function -Wno-sign-conversion -fsanitize=undefined -fsanitize-trap
#CFLAGS=-O3

all: TAGS libstd.a opt_demo test_str

TAGS: src/*.c src/*.h src/std/*.c src/std/*.h test/*.c
	find . -type f -name '*.[ch]' | etags -

clean:
	find . -type f -name '*.o' | xargs rm
	rm libstd.a

libstd.a: std.o str.o bytes.o utf8.o opt.o
	ar rcs libstd.a std.o str.o bytes.o utf8.o opt.o

std.o: src/std/std.h src/std/std.c
	${CC} ${CFLAGS} -c src/std/std.c

str.o: src/std/std.h src/std/str.c
	${CC} ${CFLAGS} -c src/std/str.c

bytes.o: src/std/std.h src/std/bytes.h src/std/bytes.c
	${CC} ${CFLAGS} -c src/std/bytes.c

utf8.o: src/std/std.h src/std/utf8.h src/std/utf8.c
	${CC} ${CFLAGS} -c src/std/utf8.c

opt.o: src/std/std.h src/std/opt.h src/std/opt.c
	${CC} ${CFLAGS} -c src/std/opt.c

test_str: libstd.a test/test_str.c
	${CC} ${CFLAGS} -o test_str test/test_str.c -L. -lstd 

opt_demo: libstd.a src/opt_demo.c
	${CC} ${CFLAGS} -o opt_demo src/opt_demo.c -L. -lstd

