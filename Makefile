CC=gcc
CFLAGS=-g3 -Wall -Wextra -Wconversion -Wdouble-promotion -Wno-unused-parameter -Wno-unused-function -Wno-sign-conversion -fsanitize=undefined -fsanitize-trap
#CFLAGS=-O3

all: TAGS libstd.a opt_demo reader_demo csv test_str test_buf test_iter test_reader test_fmt test_opt

TAGS: src/*.c src/*.h demos/*.c test/*.c
	find . -type f -name '*.[ch]' | etags -

clean:
	find . -type f -name '*.o' | xargs rm
	rm libstd.a csv opt_demo test_*

libstd.a: std.o str.o bytes.o buf.o iter.o utf8.o opt.o test.o fd.o reader.o print.o fmt.o
	rm libstd.a 2>&1 > /dev/null || true
	ar rcs libstd.a std.o str.o bytes.o buf.o iter.o utf8.o opt.o fd.o test.o reader.o print.o fmt.o

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

buf.o: src/std.h src/buf.h src/buf.c
	${CC} ${CFLAGS} -c src/buf.c

opt.o: src/std.h src/opt.h src/opt.c
	${CC} ${CFLAGS} -c src/opt.c

test.o: src/std.h src/test.h src/test.c
	${CC} ${CFLAGS} -c src/test.c

fd.o: src/std.h src/fd.h src/fd.c
	${CC} ${CFLAGS} -c src/fd.c

reader.o: src/std.h src/reader.h src/reader.c
	${CC} ${CFLAGS} -c src/reader.c

print.o: src/std.h src/print.h src/print.c
	${CC} ${CFLAGS} -c src/print.c

fmt.o: src/std.h src/fmt.h src/fmt.c
	${CC} ${CFLAGS} -c src/fmt.c

test_str: libstd.a test/test_str.c
	${CC} ${CFLAGS} -o test_str test/test_str.c -L. -lstd

test_buf: libstd.a test/test_buf.c
	${CC} ${CFLAGS} -o test_buf test/test_buf.c -L. -lstd

test_iter: libstd.a test/test_iter.c
	${CC} ${CFLAGS} -o test_iter test/test_iter.c -L. -lstd

test_reader: libstd.a test/test_reader.c
	${CC} ${CFLAGS} -o test_reader test/test_reader.c -L. -lstd

test_fmt: libstd.a test/test_fmt.c
	${CC} ${CFLAGS} -o test_fmt test/test_fmt.c -L. -lstd

test_opt: libstd.a test/test_opt.c
	${CC} ${CFLAGS} -o test_opt test/test_opt.c -L. -lstd

test: test_str test_buf test_iter test_reader test_fmt test_opt
	./test_str
	./test_buf
	./test_iter
	./test_reader
	./test_fmt
	./test_opt

opt_demo: libstd.a demos/opt_demo.c
	${CC} ${CFLAGS} -o opt_demo demos/opt_demo.c -L. -lstd

reader_demo: libstd.a demos/reader_demo.c
	${CC} ${CFLAGS} -o reader_demo demos/reader_demo.c -L. -lstd

csv: libstd.a demos/csv.c
	${CC} ${CFLAGS} -o csv demos/csv.c -L. -lstd
