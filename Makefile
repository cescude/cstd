CFLAGS=-g3 -Wall -Wextra -Wconversion -Wdouble-promotion -Wno-unused-parameter -Wno-unused-function -Wno-sign-conversion -fsanitize=undefined -fsanitize-trap
#CFLAGS=-O3

# all: TAGS dist/opt_demo dist/main dist/cat
all: TAGS dist/opt_demo test_str

test_str: dist dist/libstd.a test/test_str.c
	gcc ${CFLAGS} -o dist/test_str test/test_str.c -Ldist -lstd 

TAGS: src/*.c src/*.h src/std/*.c src/std/*.h
	find src -type f -name '*.[ch]' | etags -

dist/libstd.a: dist src/std/*.c src/std/*.h
	gcc ${CFLAGS} -c src/std/std.c -o dist/std.o
	gcc ${CFLAGS} -c src/std/str.c -o dist/str.o
	gcc ${CFLAGS} -c src/std/utf8.c -o dist/utf8.o
	gcc ${CFLAGS} -c src/std/opt.c -o dist/opt.o
	cd dist && ar rcs libstd.a std.o str.o utf8.o opt.o

# dist/main: dist dist/libstd.a dist/main.o
# 	gcc ${CFLAGS} dist/main.o -Ldist -lstd -o dist/main

# dist/main.o: dist src/std/std.h src/main.c
# 	gcc ${CFLAGS} -c src/main.c -o dist/main.o

# dist/cat: dist dist/libstd.a src/cat.c
# 	gcc ${CFLAGS} src/cat.c -Ldist -lstd -o dist/cat

dist/opt_demo.o: dist dist/libstd.a src/opt_demo.c
	gcc ${CFLAGS} src/opt_demo.c -c -o dist/opt_demo.o

dist/opt_demo: dist dist/libstd.a dist/opt_demo.o
	gcc ${CFLAGS} dist/opt_demo.o -Ldist -lstd -o dist/opt_demo

clean:
	rm -rf dist

dist:
	mkdir dist
