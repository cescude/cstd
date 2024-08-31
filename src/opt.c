#include <stdlib.h>

#include "std.h"

static print_t out = printInitUnbuffered(STDOUT_FILENO);

/*
  It's up to the caller to make sure there's sufficient space in opts
  for this (not a huge deal, since program options aren't typically
  dynamically allocated structures...)
*/

opt_t optBool(bool *result, char s, char *l, char *desc) {
  return (opt_t){
    .short_name = utf8CharFromC(s),
    .long_name = strFromC(l),
    .description = strFromC(desc),
    .type = optbool,
    .ptr.b = result,
  };
}

opt_t optInt(int *result, char s, char *l, char *arg_label, char *desc) {
  return (opt_t){
    .short_name = utf8CharFromC(s),
    .long_name = strFromC(l),
    .arg_label = strFromC(arg_label),
    .description = strFromC(desc),
    .type = optint,
    .ptr.i = result,
  };
}

opt_t optStr(str_t *result, char s, char *l, char *arg_label, char *desc) {
  return (opt_t){
    .short_name = utf8CharFromC(s),
    .long_name = strFromC(l),
    .arg_label = strFromC(arg_label),
    .description = strFromC(desc),
    .type = optstr,
    .ptr.s = result,
  };
}

opt_t optArg(str_t *result, char *arg_label, char *desc) {
    return (opt_t){
        .arg_label = strFromC(arg_label),
        .description = strFromC(desc),
        .type = optposarg,
        .ptr.s = result,
    };
}

opt_t optRest(ptrdiff_t *result, char *arg_label, char *desc) {
    return (opt_t){
        .arg_label = strFromC(arg_label),
        .description = strFromC(desc),
        .type = optrest,
        .ptr.r = result,
    };
}

opts_config_t optInit(opt_t *opts, size num_opts) {
  return (opts_config_t){
    .opts = opts,
    .num_opts = num_opts,
    .help_width = 80,
  };
}

size _findBoolIdx(opts_config_t config, str_t name) {
  for (ptrdiff_t i=0; i<config.num_opts; i++) {
    if (config.opts[i].type == optbool &&
	strEquals(config.opts[i].long_name, name)) {
      return i;
    }
  }
  return config.num_opts;
}

size _findValIdx(opts_config_t config, str_t name) {
  for (ptrdiff_t i=0; i<config.num_opts; i++) {
    if (config.opts[i].type != optbool &&
	strEquals(config.opts[i].long_name, name)) {
      return i;
    }
  }
  return config.num_opts;
}

size _findShortBoolIdx(opts_config_t config, utf8_char_t name) {
  for (ptrdiff_t i=0; i<config.num_opts; i++) {
    if (config.opts[i].type == optbool &&
	utf8CharEquals(config.opts[i].short_name, name)) {
      return i;
    }
  }
  return config.num_opts;
}

size _findShortValIdx(opts_config_t config, utf8_char_t name) {
  for (ptrdiff_t i=0; i<config.num_opts; i++) {
    if (config.opts[i].type != optbool &&
	utf8CharEquals(config.opts[i].short_name, name)) {
      return i;
    }
  }
  return config.num_opts;
}

size _findRestIdx(opts_config_t config) {
  for (ptrdiff_t i=0; i<config.num_opts; i++) {
    if (config.opts[i].type == optrest) {
      return i;
    }
  }
  return config.num_opts;
}

bool _setVal(opt_t opt, str_t arg_tail, str_t nextarg, ptrdiff_t *idx) {
  str_t val = {0};

  if (strNonEmpty(arg_tail)) {
    val = arg_tail;
  } else if (nextarg.beg) {
    val = nextarg;
    (*idx) += 1;
  } else {
    return 0;
  }

  switch (opt.type) {
  case optint:
    if (!strMaybeParseInt(val, opt.ptr.i)) {
      return 0;
    }
    break;
  case optstr:
    *opt.ptr.s = val;
    break;
  default:
    abort();
  }

  return 1;
}

bool optParse(opts_config_t config, int num_args, char **args) {
  opt_t *opts = config.opts;
  size num_opts = config.num_opts;

  size idx = 1;
  for (; idx < num_args; idx++) {
    str_t arg = strFromC(args[idx]);
    str_t nextarg = idx + 1 < num_args
      ? strFromC(args[idx + 1])
      : (str_t){0};

    if (strEquals(arg, strC("--"))) {
      /* end of flags, skip the -- token though */
      idx++;
      break;
    } else if (strEquals(arg, strC("-"))) {
      /* end of flags, but do include - in the list of "rest" tokens */
      break;
    } else if (strStartsWith(arg, strC("--"))) {
      /* long arg */
      arg = strDropChars(arg, 2);

      /* TODO: support --x=y type flags; rn we only support -x y */
      /* str_t arg_name = strTakeToByte(arg, '='); */
      
      size a = _findBoolIdx(config, arg); /* is arg a bool? */
      size b = _findValIdx(config, arg);  /* or is it a value? */

      if (a < num_opts) {
	*opts[a].ptr.b = 1;
      } else if (b < num_opts) {
	/*
	  Will incr idx if nextarg is used, which it should be since
	  we're passing nul for the current arg!
	*/
	if (!_setVal(opts[b], (str_t){0}, nextarg, &idx)) {
	  return 0;
	}
      } else {
	return 0;		/* unrecognized long option */
      }
    } else if (strStartsWith(arg, strC("-"))) {
      arg = strDropChars(arg, 1);
      if (strIsEmpty(arg)) {
	return 0;
      }

      /*
	Because short-args can be smushed together, we need to pluck
	them out one-by-one here (eg. 'ls -lrth').

	Of note, encountering a boolean flag means there's the
	possibility for another short-arg to follow (eg. 'grep -nHC3'
	has two booleans, each of which allows for the inclusion of
	additional flags; the C option, however, ends the chain).
      */
      while (strNonEmpty(arg)) {
	size a = _findShortBoolIdx(config, strFirstChar(arg));
	size b = _findShortValIdx(config, strFirstChar(arg));

	arg = strDropChars(arg, 1); /* move beyond this named arg */
	
	if (a < num_opts) {
	  *opts[a].ptr.b = 1;
	} else if (b < num_opts) {
	  /* will incr idx if nextarg is used */
	  if (!_setVal(opts[b], arg, nextarg, &idx)) {
	    return 0;
	  }
	  /*
	    setVal either used the rest of arg or all of nextarg for
	    the value (eg. '-Haccept' or '-H accept')...either way,
	    we're done looking for short flags in the current arg!
	  */
	  break;
	} else {
	  return 0;		/* unrecognized short option */
	}
      }
    } else {
      /* end of flags, remaining args are positional args */
      break;
    }
  }

  for (size i=0; i<num_opts; i++) {

      if (opts[i].type == optposarg) {
          /*
            OK, we need a positional argument; if we've run out, it's
            a problem!
          */
          if (idx >= num_args) return 0;
          
          *opts[i].ptr.s = strFromC(args[idx]);
          idx++;
      }
  }

  size a = _findRestIdx(config);
  if (a < num_opts) {
    *opts[a].ptr.r = idx;
  }
  
  return 1;
}

void printWrappedParagraph(str_t prefix, size cols, str_t para) {
  size prefix_len = strLen(prefix);
  while (strNonEmpty(para)) {
    str_t line = strTakeLineWrapped(para, cols - prefix_len);
    printStr(out, prefix);
    printStr(out, strTrim(line, strC(" \n")));
    printStr(out, strC("\n"));
    para.beg = line.end;	/* skip past the plucked line */
    para = strTrimLeft(para, strC(" "));
  }
}

void optPrintSection(opts_config_t config, char *section_name, char *section_desc) {
    fmt_t fmt = fmtInit(out);
    
    fmtNew(&fmt, "{}\n\n");
    fmtStr(&fmt, strFromC(section_name));

    printWrappedParagraph(strC("  "), config.help_width, strFromC(section_desc));
    printStr(out, strC("\n"));
}

void optPrintArguments(opts_config_t config) {
    fmt_t fmt = fmtInit(out);

    fmtNew(&fmt, "Arguments:\n\n");

    opt_t *opts = config.opts;
    size num_opts = config.num_opts;

    for (size i=0; i<num_opts; i++) {
        if (opts[i].type == optposarg) {
            if (strNonEmpty(opts[i].arg_label)) {
                fmtNew(&fmt, "  {}\n");
                fmtStr(&fmt, opts[i].arg_label);

                /*
                  Only print the desc if we have a name to go with it!
                */
                if (strNonEmpty(opts[i].description)) {
                    printWrappedParagraph(
                        strC("    "),
                        config.help_width,
                        opts[i].description
                    );
                }
            }
        }
    }
    
    size idx = _findRestIdx(config);
    if (idx < config.num_opts) {
        opt_t opt = config.opts[idx];

        if (strNonEmpty(opt.arg_label)) {
            fmtNew(&fmt, "  {}...\n");
            fmtStr(&fmt, opt.arg_label);

            /*
              Only print the desc if we have a name to go with it!
             */
            if (strNonEmpty(opt.description)) {
                printWrappedParagraph(
                    strC("    "),
                    config.help_width,
                    opt.description
                );
            }
        }
    }

    printC(out, "\n");
}

void optPrintOptions(opts_config_t config) {
    fmt_t fmt = fmtInit(out);

    fmtNew(&fmt, "Options:\n\n");

    for (size idx=0; idx<config.num_opts; idx++) {
        opt_t opt = config.opts[idx];

        if (opt.type != optposarg && opt.type != optrest) {
            int has_short = (opt.short_name!=0) << 2;
            int has_long  = strNonEmpty(opt.long_name) << 1;
            int has_label = strNonEmpty(opt.arg_label);

            switch (has_short | has_long | has_label) {
            case 0x7: fmtNew(&fmt, "  -{}, --{} {}\n"); break;
            case 0x6: fmtNew(&fmt, "  -{}, --{}\n"); break;
            case 0x5: fmtNew(&fmt, "  -{} {}\n"); break;
            case 0x4: fmtNew(&fmt, "  -{}\n"); break;
            case 0x3: fmtNew(&fmt, "     --{} {}\n"); break;
            case 0x2: fmtNew(&fmt, "     --{}\n"); break;
            default:
                assert(false, strC("Invalid options passed, needs a name!"));
            }

            if (has_short) fmtChar(&fmt, opt.short_name);
            if (has_long) fmtStr(&fmt, opt.long_name);
            if (has_label) fmtStr(&fmt, opt.arg_label);

            if (strNonEmpty(opt.description)) {
                printWrappedParagraph(strC("          "), config.help_width, opt.description);
            }
        }
    }

    printC(out, "\n");
}

void optPrintUsage(opts_config_t config, char *prog_name, char *summary) {
    fmt_t fmt = fmtInit(out);

    size rest_idx = _findRestIdx(config);
    bool has_options = 0;
    bool has_args = 0;
    
    for (size i=0; i<config.num_opts; i++) {
        if (config.opts[i].type != optrest) {
            has_options = 1;
        } else if (config.opts[i].type == optposarg ||
                   config.opts[i].type == optrest) {
            has_args = 1;
        }
    }

    fmtNew(&fmt, "Usage: {}");
    fmtStr(&fmt, strFromC(prog_name));

    if (has_options) {
        printC(out, " [OPTIONS]");
    }

    for (size i=0; i<config.num_opts; i++) {
        if (config.opts[i].type == optposarg) {
            fmtNew(&fmt, " {}");
            fmtStr(&fmt, strIsEmpty(config.opts[i].arg_label)
                   ? strC("<arg>")
                   : config.opts[i].arg_label
            );
        }
    }
    
    if (rest_idx < config.num_opts) {
        fmtNew(&fmt, " [{}]...");
        fmtStr(
            &fmt,
            strIsEmpty(config.opts[rest_idx].arg_label)
                ? strC("ARG")
                : config.opts[rest_idx].arg_label)
        ;
    }

    printC(out, "\n\n");

    printWrappedParagraph(strC("  "), config.help_width, strFromC(summary));
    printStr(out, strC("\n"));

    if (has_args) {
        optPrintArguments(config);
    }

    if (has_options) {
        optPrintOptions(config);
    }
}
