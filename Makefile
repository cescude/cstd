CC=gcc
CFLAGS=-g3 -Wall -Wextra -Wconversion -Wdouble-promotion -Wno-unused-parameter -Wno-unused-function -Wno-sign-conversion -fsanitize=undefined -fsanitize-trap
# CFLAGS=-O3

all: TAGS libstd.a opt_demo reader_demo test_str test_iter test_reader

TAGS: src/*.c src/*.h demos/*.c test/*.c
	find . -type f -name '*.[ch]' | etags -

clean:
	find . -type f -name '*.o' | xargs rm
	rm libstd.a opt_demo test_str

libstd.a: std.o str.o bytes.o iter.o utf8.o opt.o test.o reader.o print.o
	rm libstd.a 2>&1 > /dev/null || true
	ar rcs libstd.a std.o str.o bytes.o iter.o utf8.o opt.o test.o reader.o print.o

std.o: src/std.h src/std.c
	${CC} ${CFLAGS} -c src/std.c

str.o: src/std.h src/str.h src/str.c
	${CC} ${CFLAGS} -c src/str.c

bytes.o: src/std.h src/bytes.h src/bytes.c
	${CC} ${CFLAGS} -c src/bytes.c

iter.o: src/std.h src/iter.h src/iter.c
	${CC} ${CFLAGS} -c src/iter.c

utf8.o: src/std.h src/utf8.h src/utf8.c
	${CC} ${CFLAGS} -c src/utf8.c

opt.o: src/std.h src/opt.h src/opt.c
	${CC} ${CFLAGS} -c src/opt.c

test.o: src/std.h src/test.h src/test.c
	${CC} ${CFLAGS} -c src/test.c

reader.o: src/std.h src/reader.h src/reader.c
	${CC} ${CFLAGS} -c src/reader.c

print.o: src/std.h src/print.h src/print.c
	${CC} ${CFLAGS} -c src/print.c

test_str: libstd.a test/test_str.c
	${CC} ${CFLAGS} -o test_str test/test_str.c -L. -lstd

test_iter: libstd.a test/test_iter.c
	${CC} ${CFLAGS} -o test_iter test/test_iter.c -L. -lstd

test_reader: libstd.a test/test_reader.c
	${CC} ${CFLAGS} -o test_reader test/test_reader.c -L. -lstd

opt_demo: libstd.a demos/opt_demo.c
	${CC} ${CFLAGS} -o opt_demo demos/opt_demo.c -L. -lstd

reader_demo: libstd.a demos/reader_demo.c
	${CC} ${CFLAGS} -o reader_demo demos/reader_demo.c -L. -lstd
