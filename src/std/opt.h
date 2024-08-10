#pragma once

#include <stdbool.h>

typedef struct {
  char short_name;
  str_t long_name;
  enum { optbool, optint, optstr, optrest } type;
  union {
    bool *b;
    int *i;
    str_t *s;
    ptrdiff_t *r;
  } ptr;
} opt_t;

typedef struct {
  str_t summary;
  opt_t *opts;			/* we don't own these */
  
  /* numopts is number of opts defined, *NOT* the count/capacity of
     allocated opts--ie., caller needs to be mindful of the
     capacity */
  
  ptrdiff_t num_opts;
} opts_config_t;

void optBool(opts_config_t *config, char s, char *l, bool *result);
void optInt(opts_config_t *config, char s, char *l, int *result);
void optStr(opts_config_t *config, char s, char *l, str_t *result);
void optRest(opts_config_t *config, ptrdiff_t *result);

bool optParse(opts_config_t config, int numargs, char **args);
