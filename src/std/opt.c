#include <stdlib.h>

#include "std.h"

/*
  It's up to the caller to make sure there's sufficient space in opts
  for this (not a huge deal, since program options aren't typically
  dynamically allocated structures...)
*/

void optBool(opts_config_t *config, char s, char *l, bool *result) {
  config->opts[config->num_opts++] = (opt_t){
    .short_name = utf8CharFromC(s),
    .long_name = strFromC(l),
    .type = optbool,
    .ptr.b = result,
  };
}

void optInt(opts_config_t *config, char s, char *l, int *result) {
  config->opts[config->num_opts++] = (opt_t){
    .short_name = utf8CharFromC(s),
    .long_name = strFromC(l),
    .type = optint,
    .ptr.i = result,
  };
}

void optStr(opts_config_t *config, char s, char *l, str_t *result) {
  config->opts[config->num_opts++] = (opt_t){
    .short_name = utf8CharFromC(s),
    .long_name = strFromC(l),
    .type = optstr,
    .ptr.s = result,
  };
}

void optRest(opts_config_t *config, ptrdiff_t *result) {
  config->opts[config->num_opts++] = (opt_t){
    .type = optrest,
    .ptr.r = result,
  };
}

ptrdiff_t _findBoolIdx(opts_config_t config, str_t name) {
  for (ptrdiff_t i=0; i<config.num_opts; i++) {
    if (config.opts[i].type == optbool &&
	strEquals(config.opts[i].long_name, name)) {
      return i;
    }
  }
  return config.num_opts;
}

ptrdiff_t _findValIdx(opts_config_t config, str_t name) {
  for (ptrdiff_t i=0; i<config.num_opts; i++) {
    if (config.opts[i].type != optbool &&
	strEquals(config.opts[i].long_name, name)) {
      return i;
    }
  }
  return config.num_opts;
}

ptrdiff_t _findShortBoolIdx(opts_config_t config, utf8_char_t name) {
  for (ptrdiff_t i=0; i<config.num_opts; i++) {
    if (config.opts[i].type == optbool &&
	utf8CharEquals(config.opts[i].short_name, name)) {
      return i;
    }
  }
  return config.num_opts;
}

ptrdiff_t _findShortValIdx(opts_config_t config, utf8_char_t name) {
  for (ptrdiff_t i=0; i<config.num_opts; i++) {
    if (config.opts[i].type != optbool &&
	utf8CharEquals(config.opts[i].short_name, name)) {
      return i;
    }
  }
  return config.num_opts;
}

ptrdiff_t _findRestIdx(opts_config_t config) {
  for (ptrdiff_t i=0; i<config.num_opts; i++) {
    if (config.opts[i].type == optrest) {
      return i;
    }
  }
  return config.num_opts;
}

bool _setVal(opt_t opt, str_t arg_tail, str_t nextarg, ptrdiff_t *idx) {
  str_t val = {0};

  if (arg_tail.len > 0) {
    val = arg_tail;
  } else if (nextarg.ptr) {
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
  ptrdiff_t num_opts = config.num_opts;

  ptrdiff_t idx = 1;
  for (; idx < num_args; idx++) {
    str_t arg = strFromC(args[idx]);
    str_t nextarg = idx + 1 < num_args
      ? strFromC(args[idx + 1])
      : (str_t){0};

    if (strEquals(arg, strFromC("--"))) {
      /* end of flags, skip the -- token though */
      idx++;
      break;
    } else if (strEquals(arg, strFromC("-"))) {
      /* end of flags, but do include - in the list of "rest" tokens */
      break;
    } else if (strStartsWith(arg, strFromC("--"))) {
      /* long arg */
      arg = strDropChars(arg, 2);

      /* TODO: support --x=y type flags; rn we only support -x y */
      /* str_t arg_name = strTakeToByte(arg, '='); */
      
      ptrdiff_t a = _findBoolIdx(config, arg); /* is arg a bool? */
      ptrdiff_t b = _findValIdx(config, arg);  /* or is it a value? */

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
    } else if (strStartsWith(arg, strFromC("-"))) {
      arg = strDropChars(arg, 1);
      if (!arg.len) {
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
      while (arg.len > 0) {
	ptrdiff_t a = _findShortBoolIdx(config, strFirstChar(arg));
	ptrdiff_t b = _findShortValIdx(config, strFirstChar(arg));

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

  ptrdiff_t a = _findRestIdx(config);
  if (a < num_opts) {
    *opts[a].ptr.r = idx;
  }
  
  return 1;
}
