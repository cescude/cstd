# (Chandler's) Alternative Standard C Library

## Background

Wanting to explore the idea that the C is actually a nice language to
program in, in spite of the C standard library itself. To that end,
this is just an experiment to see what I sort of an API I can build in
a "nice" way.

Some goals:
* Minimize allocations, try to avoid malloc/free
* Minimize reliance on c-strings
* Use structs as values as much as possible
* Figure out what the "right" thing is, and make that easy
* Don't design an API for infinite memory
* User code doesn't need libc as a matter of course

## What I have so far

* A set of string functions supporting utf8 by default
* A set of buffer functions for byte manipulation
* A set of file functions for reading/writing
* A set of printer functions to write buffered/unbuffered data
* A set of option-parsing functions to handle CLI concerns

Note about strings vs buffers--

`str_t` is my string type, and it's a struct with a pointer to the
beginning & end of a string. The assumption made is that this points
to memory that is NOT owned, and that narrowing the string
(ie. incrementing the beginning, or decrementing the end) is always a
valid operation. Additionally, at this point, no string functions
modify the underlying data, so it's considered a readonly view into
memory.

`buf_t` is my buffer type (ie., view into read/write memory), and it's
a struct with a starting pointer, a length, and a capacity value. Like
`str_t`, this is assumed to not own its memory, however the starting
pointer and capacity is treated as "fixed in place" by all functions
(ie., dropping 10 characters from a `str_t` will adjust the beginning
pointer, wherease dropping 10 characters from a `buf_t` will move
memory and decrement the length).

Strings and buffers are creating with backing memory, and the conceit
here is that the API should encourage addressing memory limits as part
of any "straightforward" usage patterns.

    <insert example>

## TODO

* Create `bytes_t` interface similar to `str_t`, and move `strLenBytes` (etc) out
** bytesLen(bytes_t b)
** bytesTakeUntilByte(bytes_t b, byte b)
** bytesIntoStruct(bytes_t b, void *ptr, size sz)
** bytesIntoArray(bytes_t b, void *ptr, size sz, size count)
* More test coverage


