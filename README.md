# (Chandler's) Alternative Standard C Library

## Background

Wanting to explore the idea that the C is actually a nice language to
program in, in spite of the C standard library itself. To that end,
this is just an experiment to see what I sort of an API I can build in
a "nice" way.

This is a learning repo, to try to figure out some things.

Some goals:

* Minimize allocations, try to avoid malloc/free
* Minimize reliance on c-strings
* Use structs as values as much as possible
* Figure out what the "right" thing is, and make that easy
* Don't design an API for infinite memory
* User code doesn't need libc as a matter of course

Even though I'm still experimenting w/ the APIs, I'm particularly
proud of the simplicity and flexibility provided by the formatting &
argument-parsing sections.

## Utilities

Defines the following things which are generally helpful:

* `countof(thing)` -- count elements in a C array
* `die(char*)` -- abort the program with the given error
* `assert(bool,char*)` -- if the first arg evaluates false call `die(char*)`
* `size` -- just a typedef for `ptrdiff_t`

Note that here, as elsewhere in the library, I use `char*`
intentionally in places that should be *not* dynamically
created. Eg. when calling `assert()`, just pass in a const C-string
that you handwrote.

## Strings

Functions to aid scanning, parsing, and testing utf8 string data.

    str_t s0 = strC("One Two Three");
    str_t s1 = strDropSuffix(s0, strC(" Three"));
    
    assert(strEquals(s0, strC("One Two Three")), "Original unchanged");
    assert(strEquals(s1, strC("One Two")), "Dropped ` Three`");

    int n = 0;
    str_t s2 = strC("   123  \n");
    assert(!strMaybeParseInt(s2, &n), "Not a number, it's got extra junk");

    str_t s3 = strTrim(s2, strC(" \n"));
    assert(strMaybeParseInt(s3, &n), "Ok we're good now");
    assert(n == 123, "Parsed 123");
      
The string type is called `str_t` & is just two pointers, a beginning
and an end (ie. a slice). The expectation for all string functions
written is that strings are readonly, which allows flexibility to have
them point anywhere (be it readonly program data or somewhere on the
heap).

Strings can be initialzed as `(str_t){0}` if you have no data, the
functions can (should!) handle this as an empty string.

All string functions work with utf8 data...if they don't it's a
bug. Currently there's a handful of `utf8`-prefixed functions
(eg. `utf8StrLen(...)`), but these should be merged into the main
string collection (originally was ascii strings with utf8 separately,
but realized this is the wrong approach and the lib should be split as
str* functions and bytes* functions instead).

All that being said, I'm not super happy with the utf8_char_t stuff,
and usually end up avoiding it due to awkwardness. Still some design
work to go with those bits I guess...

## Bytes

Sister-collection to Strings, not particularly filled out as I haven't
had a need for raw byte manipulation yet & I don't know exactly what
would be a good design offhand.

## Buffers

Whereas strings used to inspect memory, buffers represent read/write
data. Conceptually I visualize a buffers as a workbench with clutter
on it, and strings as what's between my hands.

Buffers need backing memory to work, whether it's statically reserved
or from the heap.

    byte data[1<<10] = {0}; /* reserve 1k data */
    buf_t buf = bufFromC(data);

Aside from occasionally calling `bufClear(&buf)`, there's really not a
huge need for direcly working with buffers (mostly they're passed into
higher level abstractions like the reader/printer/etc functions).

I did include a basic arena allocator built ontop of buffers, but so
far it's only used in unit tests...rn it aborts if you run out of
memory, but it should definitely do better.

TODO: create a method to resize a buffer, ie. something like:

    byte data_small[1<<10] = {0};
    buf_t buf = bufFromC(data_small);

    /* ... some stuff ... */
    byte data_large[1<<20] = {0};
    byte* old = bufResize(&buf, data_large);

The mechanics of it is that `bufResize()` will copy its contents from
the old spot to the new. I haven't written this yet because I'm not
100% sure if it's danger-vs-helpfulness ratio is too far weighted in
the danger side...

Moreover, Care needs to be taken that no pointers are pointing into
the old memory (which is easiest to do by using the callstack to
handle lifetimes). Even then, higher level structures that embed
buffers need to make sure that resizes adjust the other members as
well.

## Iterators

Iterators are used to progressively scan across bits of memory (think
string splitting, reading lines out of a buffer, etc).

    iter_t it = iterFromStr(strC("one;;two;;three"));
    while (iterTakeToStr(&it, strC(";;"))) {
        str_t tok = iterStr(it);

        /* ...can now do something with the token... */
    }

One design note when building the iterator API is that I've found it's
helpful to always include the trailing character. It's a one-liner to
remove it (`strDropSuffix(iterStr(it), ...)`), it makes the
implementation much clearer, and it satisfies both styles of iteration
(both "suffix required" & "suffix not wanted").

## Testing

Minimal needs for testing.

    void test_strDropPrefix_shouldDropAPrefix(test_t *t) {
    
        struct { str_t s, pre, expected; } cases[] = {
            { strC("proactive"), strC("pro"), strC("active") },
            { strC("active"), strC("pro"), strC("active") },
        };

        for (size i=0; i<countof(cases); i++) {
            str_t result = strDropPrefix(cases[i].s, cases[i].pre);
            assertTrue(strEquals(result, cases[i].expected), strC("Prefix was dropped"));
        }
    }

    int main(int argc, char **argv) {
        bool verbose = 1;
        
        test_defn_t tests[] = {
            { test_strDropPrefix_shouldDropAPrefix, "strDropPrefix should drop a prefix" },
            /* additional tests */
        };

        return (int)testRunner(tests, countof(tests), verbose);
    }

Look at `test/test_*.c` for larger examples.

## File Handles

...

## Readers

...

## Printers

...

## Formatting

Alternative to *printf functions...needed because those work on c
strings, and this library makes it difficult to convert strings to c
strings.

That being said, I think this interface is actually a win over printf
style formatting?

    fmt_t fmt = fmtToFile(1); /* stdout, unbuffered */

    fmtStart(&fmt, "power={}, coins={}, helpful_message={}\n"); // prints "power="
    fmtNum(&fmt, 100);                                          // prints "100, coins="
    fmtNum(&fmt, 30);                                           // prints "30, helpful_message="
    fmtStr(&fmt, strC("You've got this"));                      // prints "You've got this\n"

    fmtStart(&fmt, "What do {}?\n");                            // prints "What do "
    fmtStr(&fmt, strC("you think"));                            // prints "you think?\n"

Notice the typechecking etc happens at parameter declaration, and so
there's no real need for extra compiler magic or macros to introspect
parameters.

A function `fmtSkip(fmt_t*)` is provided so you can write your own
formatting functions without needing to extend the pattern syntax:

    fmtStart(&fmt, "Hey {}, what about {}!\n");
    fmtStr(&fmt, strC("there"));

    // Anything you print here will be written at the second {}
    printC(fmt.out, "this or...");
    printC(fmt.out, "that?");

    // Tell the formatter to either skip this hole (since we did our own printing)
    fmtSkip(&fmt);

This works because the fmt functions print everything up to the next
{} in the pattern, then return. So the previous example looks like:

    fmtStart(...) => "Hey {}"
    fmtStr(...)   => "Hey there, what about {}"
    printC(...)   => "Hey there, what about this or...{}"
    printC(...)   => "Hey there, what about this or...that?{}"
    fmtSkip(...)  => "Hey there, what about this or...that?!\n"

FWIW the `fmtStart` takes a char* for two reasons:

* It's pretty easy (for me) to type `fmtStr(...)` by accident, and
  this prevents that.
  
* Also don't think dynamically generating format strings is a great
  idea, so this makes it harder to do :^(

## Commandline option parsing

Set of functions for parsing commandline options. Essentially, you
statically declare a set of options, positional arguments, etc, then
use some helper methods to parse them out. Uses const cstrings for
configuration since this is basically static data.

For example:

    /*
      provide default values; if an option isn't specified, the
      default will be retained.
    */
    bool verbose = 0;
    int energy = 0;
    int coins = 0;
    str_t nickname = strC("");
    bool show_help = 0;
    
    opt_t opts[] = {
        optBool(&verbose, 'v', "verbose", "Be extra loud or something."),
        optInt(&energy, 'E', "energy", "Specify energy levels."),
        optInt(&coins, 'c', "coins", "How many coins do you want?"),
        optStr(&nickname, 'n', "nick", "Provide a nickname for this user."),
        optBool(&show_help, 'h', "help", "Displays this help"),
    };

    opts_config_t config = optInit(opts, countof(opts));

    if (!optParse(config, argc, argv) || show_help) {
       optPrintUsage(config, argv[0], "A commandline demonstration of a few features");
       exit(0);
    }

    fmt_t fmt = fmtToFile(1); /* unbuffered writes to stdout */
    fmtStart(&fmt, "verbose={}, energy={}, coins={}, nickname={}\n");
    fmtBool(&fmt, verbose);
    fmtInt(&fmt, energy);
    fmtInt(&fmt, coins);
    fmtStr(&fmt, nickname);

Can check out `demos/opt_demo.c` or `demos/csv.c` for examples that
exercise more functionality.

## Scratch/notes below

Would like to add a "scanner" interface, basically the read to fmt's
write.

Something like:

    scan_t sc = scanInit(my_str, "Title={}, count={}\n");
    str_t title = scanStr(&sc);
    int count = scanNum(&sc);
    if (scanFailed(sc)) {
        // ...
    }

Now that I think about it, why not use this pattern for the fmt thing too?

    fmt_t fmt = fmtToFile(1, "verbose={}, energy={}!\n");
    fmtBool(&fmt, verbose);
    fmtInt(&fmt, energy);
    if (fmtFailed(fmt)) {
       // ...
    }

## TODO

I'm putting more function signatures in header files than I have implemented;
I'm using this "declare but don't implement" to jot down ideas about missing
functionality without getting into the weeds.

General cleanup
* unify naming
