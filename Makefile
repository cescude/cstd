CC=cc
CFLAGS=-g3 -Wall -Wextra -Wconversion -Wdouble-promotion -Wno-unused-parameter -Wno-unused-function -Wno-sign-conversion -fsanitize=undefined -fsanitize-trap
#CFLAGS=-O3

all: TAGS libstd.a opt_demo test_str

TAGS: src/*.c src/*.h demos/*.c test/*.c
	find . -type f -name '*.[ch]' | etags -

clean:
	find . -type f -name '*.o' | xargs rm
	rm libstd.a opt_demo test_str

libstd.a: std.o str.o bytes.o utf8.o opt.o test.o
	ar rcs libstd.a std.o str.o bytes.o utf8.o opt.o test.o

std.o: src/std.h src/std.c
	${CC} ${CFLAGS} -c src/std.c

str.o: src/std.h src/str.h src/str.c
	${CC} ${CFLAGS} -c src/str.c

bytes.o: src/std.h src/bytes.h src/bytes.c
	${CC} ${CFLAGS} -c src/bytes.c

utf8.o: src/std.h src/utf8.h src/utf8.c
	${CC} ${CFLAGS} -c src/utf8.c

opt.o: src/std.h src/opt.h src/opt.c
	${CC} ${CFLAGS} -c src/opt.c

test.o: src/std.h src/test.h src/test.c
	${CC} ${CFLAGS} -c src/test.c

test_str: libstd.a test/test_str.c
	${CC} ${CFLAGS} -o test_str test/test_str.c -L. -lstd 

opt_demo: libstd.a demos/opt_demo.c
	${CC} ${CFLAGS} -o opt_demo demos/opt_demo.c -L. -lstd

