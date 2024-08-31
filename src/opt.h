#pragma once

#include <stdbool.h>

typedef struct {
    utf8_char_t short_name;
    str_t long_name;
    str_t arg_label;		/* only used for help messages */
    str_t description;          /* only used for help messages */
    enum { optbool, optint, optstr, optrest } type;
    union {
        bool *b;
        int *i;
        str_t *s;
        ptrdiff_t *r;
    } ptr;
} opt_t;

typedef struct {
    opt_t *opts;			/* we don't own these */
  
    /*
      numopts is number of opts defined, *NOT* the count/capacity of
      allocated opts--ie., caller needs to be mindful of the capacity.
    */
    size num_opts;

   /*
     Number of columns to use when printing help screens. Feel free to
     manually override!
   */
    size help_width;
} opts_config_t;

opts_config_t optInit(opt_t *opts, size num_opts);

opt_t optBool(bool *result, char s, char *l, char *desc);
opt_t optInt(int *result, char s, char *l, char *arg_label, char *desc);
opt_t optStr(str_t *result, char s, char *l, char *arg_label, char *desc);
opt_t optRest(ptrdiff_t *result, char *arg_label, char *desc);

bool optParse(opts_config_t config, int numargs, char **args);

/* always goes to stdout */
void optPrintSection(opts_config_t config, char *section_name, char *section_desc);
void optPrintArguments(opts_config_t config);
void optPrintOptions(opts_config_t config);

/* tries to do the right thing */
void optPrintUsage(opts_config_t config, char *prog_name, char *summary);
