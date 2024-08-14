#include "std/std.h"

#include <unistd.h>

_Bool optGet_bool(int argc, char **argv, char short_name, char *long_name) {
  argc--; argv++;		/* Nobody cares about the program name */
}

typedef struct {
  int i;
  str s;
  _Bool b;
  ptrdiff_t extra; /* offset into argv where the "rest" of the parameters start */
} config_t;

enum opt_variant {
  OPT_VARIANT_INT32,
  OPT_VARIANT_UINT32,
  OPT_VARIANT_STR,
  OPT_VARIANT_BOOL,
  OPT_VARIANT_REST,
};

typedef struct {
  int64_t type;
  char short_name;
  str long_name;
  
  union {
    int32_t *int32;
    uint32_t *uint32;
    str_t *str;
    _Bool *bool;
    ptrdiff_t *rest_count;
  } ptr;
};
typedef struct {
  ptrdiff_t num_opts;
  opt_t opts[64];
} opts_t;

int main(int argc, char **argv) {
  config_t config = {0};

  opts_t opts = {0};

  optI32(&opts, 'c', "columns", &config.i);
  optStr(&opts, 'f', "filename", &config.s);
  optBool(&opts, 'v', "verbose", &config.b);
  optRest(&opts, &config.extra);
  
  if ( !optParse(opts, argc, argv) ) {
    abort();
  }
  

  // progn --abc=def --abc def -- ok1 ok2
  // progn --abc=def --def hgi -v ok1 ok2
  // progn --abc=def -vfthis-file.txt extra
  return 0;
}
