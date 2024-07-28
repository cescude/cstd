main: std.c std.h main.c
	cc -Wall main.c std.c -o main

maino3: std.c std.h main.c
	cc -Wall -O3 main.c std.c -o maino3
