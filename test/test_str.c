#include "../src/std/std.h"

#include <stdlib.h>

#define countof(x) (size)(sizeof(x)/sizeof(x[0]))

size suiteno = 1;
size caseno = 1;
size num_passed = 0;
size num_failed = 0;

#define runtest(name, body) {				\
    {body;}						\
    fprintf(stderr,					\
	    "[%03ld]\tpass= %ld\tfail=%ld\t%s\n",	\
	    suiteno++, num_passed, num_failed, name);	\
    caseno = num_passed = num_failed = 0;		\
  }

#define assert(bln) {					\
    caseno++;						\
    if (!(bln)) {					\
      fprintf(stderr,					\
	      "Subtest: %ld -- Failed at line %d\n",	\
	      caseno, __LINE__);			\
      num_failed++;					\
    } else {						\
      num_passed++;					\
    }							\
  }

/* taken from wikipedia */
char phrase1[] = "Mình nói tiếng Việt";
char phrase2[] = "𨉟呐㗂越";

int main(int argc, char **argv) {
  runtest("strLen Should return zero", ({
	str_t s[] = {
	  strC(""),
	  (str_t){0}
	};

	for (size i=0; i<countof(s); i++) {
	  assert(strLen(s[i]) == 0);
	  assert(strLenBytes(s[i]) == 0);
	}
      }));
  
  runtest("strLen Should count utf8 characters", ({
	/* Examples taken from Wikipedia */
	struct {
	  str_t s;
	  size len;
	  size bytes;
	} examples[] =
	  {
	    { strC(phrase1), 19, 25 },
	    { strC(phrase2), 4, 13 },
	  };

	for (size i=0; i<countof(examples); i++) {
	  assert(strLen(examples[i].s) == examples[i].len);
	  assert(strLenBytes(examples[i].s) == examples[i].bytes);
	}  
      }));
  
  runtest("strIsEmpty Should function appropriately", ({
	str_t empty[] = { (str_t){0}, strC("") };
	str_t non_empty[] = {
	  strC(" "), strC("one"), strC(phrase1), strC(phrase2)
	};

	for (size i=0; i<countof(empty); i++) {
	  assert(strIsEmpty(empty[i]));
	  assert(!strNonEmpty(empty[i]));
	}

	for (size i=0; i<countof(non_empty); i++) {
	  assert(strNonEmpty(non_empty[i]));
	  assert(!strIsEmpty(non_empty[i]));
	}
      }));

  runtest("strEquals Should return true on same bytes", ({
	struct {
	  str_t a, b;
	} cases[] = {
	  { strC("one"), strC("one") },
	  { strC(""), (str_t){0} },
	  { strC(phrase1), strC(phrase1) },
	};

	for (size i=0; i<countof(cases); i++) {
	  assert(strEquals(cases[i].a, cases[i].b));
	  assert(strEquals(cases[i].b, cases[i].a));
	}
      }));
  
  runtest("strEquals Should return false on different bytes", ({
	struct {
	  str_t a, b;
	} cases[] = {
	  { strC("one"), strC("eno") },
	  { strC("a"), (str_t){0} },
	  { strC(phrase1), strC(phrase2) },
	};

	for (size i=0; i<countof(cases); i++) {
	  assert(!strEquals(cases[i].a, cases[i].b));
	  assert(!strEquals(cases[i].b, cases[i].a));
	}
      }));

  runtest("strStartsWith Should return 1 when prefixed", ({
	struct { str_t str, prefix; } cases[] = {
	  { strC(""), strC("") },
	  { (str_t){0}, strC("") },
	  { strC("anything"), (str_t){0} },
	  { strC("anything"), strC("") },
	  { strC("anything"), strC("any") },
	  { strC(phrase1), strC(phrase1) },
	  { strC("Mình nói tiếng Việt"), strC("Mình nói") }
	};

	for (size i=0; i<countof(cases); i++) {
	  assert(strStartsWith(cases[i].str, cases[i].prefix));
	}

	struct { str_t str, prefix; } neg_cases[] = {
	  { (str_t){0}, strC("a") },
	  { strC(""), strC("a") },
	  { strC("any"), strC("anything") },
	  { strC("something"), strC("soem") },
	  { strC("Mình nói"), strC("Mình nói tiếng Việt") }
	};
	  
	for (size i=0; i<countof(neg_cases); i++) {
	  assert(!strStartsWith(neg_cases[i].str,
				neg_cases[i].prefix));
	}
      }));

  runtest("strTrimLeft Basic functionality", ({
	struct { str_t src, chars, result; } cases[] = {
	  { strC("one"), strC(""), strC("one") }, /* nothing to trim here */
	  { strC("one"), strC(" "), strC("one") },
	  { strC("   \t one"), strC(" \t"), strC("one") },
	  { strC("abcbabcdefabc"), strC("abc"), strC("defabc") }
	};

	for (size i=0; i<countof(cases); i++) {
	  assert(strEquals(strTrimLeft(cases[i].src, cases[i].chars), cases[i].result));
	}
      }));

  runtest("strTrimRight Basic functionality", ({
	struct { str_t src, chars, result; } cases[] = {
	  { strC("one"), strC(""), strC("one") }, /* nothing to trim here */
	  { strC("one"), strC(" "), strC("one") },
	  { strC("one  \t  "), strC(" \t"), strC("one") },
	  { strC("abcbabcdefabcba"), strC("abc"), strC("abcbabcdef") }
	};

	for (size i=0; i<countof(cases); i++) {
	  assert(strEquals(strTrimRight(cases[i].src, cases[i].chars), cases[i].result));
	}
      }));

  runtest("strTakeLineWrapped Should return full text for short rows", ({
	struct { str_t src; size cols; } cases[] = {
	  { strC("wrap"), 80 },
	  { strC("wrap?"), 5 }
	};

	for (size i=0; i<countof(cases); i++) {
	  assert(strEquals(cases[i].src, strTakeLineWrapped(cases[i].src, cases[i].cols)));
	}
      }));

  runtest("strTakeLineWrapped Should return rows no longer than cols", ({
	struct { str_t src; size cols; } cases[] = {
	  { strC("wrap wrapwrap ok"), 10 },
	  { strC("wrap?"), 5 },
	  { strC("wrap!"), 3 }	/* hard split in the middle of the word */
	};

	for (size i=0; i<countof(cases); i++) {
	  str_t result = strTakeLineWrapped(cases[i].src, cases[i].cols);
	  assert(strLen(result) <= cases[i].cols);
	}
      }));

  runtest("strTakeLineWrapped Should break on a space", ({
	str_t para = strC("Here's an amazing piece of writing that spans "
			  "a fair amount of text yielding many opportunities "
			  "for wrapping.");

	/* not looking for edge cases (ie. why 10...len-10) */
	for (size i=10; i<strLen(para) - 10; i++) {
	  str_t result = strTakeLineWrapped(para, i);
	  assert(*result.end == ' ');
	}
      }));

  runtest("utf8CharEquals Basic functionality", ({
	struct { utf8_char_t a, b; } cases[] = {
	  { utf8CharFromC('v'), utf8CharFromC('v') },
	  { utf8FirstChar(strC("verbose")), utf8CharFromC('v') }
	};

	for (size i=0; i<countof(cases); i++) {
	  assert(utf8CharEquals(cases[i].a, cases[i].b));
	}
      }));
  
  return 0;
}
