#include <stdlib.h>

#include "std.h"

/*
  It's up to the caller to make sure there's sufficient space in opts
  for this (not a huge deal, since program options aren't typically
  dynamically allocated structures...)
*/

void optBool(opts_config_t *config, char s, char *l, bool *result) {
  config->opts[config->num_opts++] = (opt_t){
    .short_name = s,
    .long_name = strFromC(l),
    .type = optbool,
    .ptr.b = result,
  };
}

void optInt(opts_config_t *config, char s, char *l, int *result) {
  config->opts[config->num_opts++] = (opt_t){
    .short_name = s,
    .long_name = strFromC(l),
    .type = optint,
    .ptr.i = result,
  };
}

void optStr(opts_config_t *config, char s, char *l, str_t *result) {
  config->opts[config->num_opts++] = (opt_t){
    .short_name = s,
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
      /* end of flags */
      idx++;
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
      // TODO support short options
      abort();
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
