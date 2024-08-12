#include "../src/std/std.h"

#include <stdlib.h>

#define countof(x) (size)(sizeof(x)/sizeof(x[0]))

size suiteno = 1;
size num_passed = 0;
size num_failed = 0;

#define runtest(name, body) {					\
  {body;}							\
  fprintf(stderr,						\
	  "[%ld] %50s num_passed=%ld, num_failed=%ld\n",	\
	  suiteno++, name, num_passed, num_failed);		\
  num_passed = num_failed = 0;					\
  }

#define assert(bln)					\
  if (!(bln)) {						\
    fprintf(stderr, "Failed at line %d\n", __LINE__);	\
    num_failed++;					\
  } else {						\
    num_passed++;					\
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

  runtest("strStartsWith() Basic functionality", ({
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
	  { strC("Mình nói"), strC("Mình nói tiếng Việt") }
	};
	  
	for (size i=0; i<countof(neg_cases); i++) {
	  assert(!strStartsWith(neg_cases[i].str,
				neg_cases[i].prefix));
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
