#include "karg.h"

#include "karg_internals.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
bool karg_parse(arguments *args, const int argc, char **argv) {
  init_arguments(args, argv[0]);
  if (argc == 1) {
    return true;
  }

  const char *flag_name;
  for (int i = 1; i < argc; ++i) {
    const char *arg_str = argv[i];
    const size_t arg_len = strlen(argv[i]);
    if (arg_len >= 1 && arg_str[0] == '-') {
      // short/long named argument
      if (arg_len >= 2 && arg_str[1] == '-') {
        flag_name = argv[i] + 2;
      } else {
        flag_name = argv[i] + 1;
        assert(strlen(flag_name) == 1);
      }
      assert(i + 1 < argc);
      ++i;
      if (!set_named_argument(args, flag_name, argv[i])) {
        fprintf(stderr, "failed setting flag %s to %s\n", arg_str, argv[i]);
        return false;
      }
      debug_printf("flag { name: %s, value: %s }\n", flag_name, argv[i]);
    } else {
      fprintf(stderr, "positional arguments are not implemented yet: %s\n",
              arg_str);
      assert(false);
    }
  }
  return true;
}

void karg_free(arguments *args) {
  if (args->positional_arguments) {
    for (size_t i = 0; i < args->npositional_arguments; ++i) {
      assert(args->positional_arguments[i].value);
      free(args->positional_arguments[i].value);
      args->positional_arguments[i].value = NULL;
    }
    free(args->positional_arguments);
  }
  args->positional_arguments = NULL;
  args->npositional_arguments = 0;

  if (args->named_arguments) {
    for (size_t i = 0; i < args->nnamed_argument_defs; ++i) {
      if (args->named_arguments[i].value) {
        free(args->named_arguments[i].value);
      }
      args->named_arguments[i].value = NULL;
    }
    free(args->named_arguments);
  }
  args->named_arguments = NULL;

  // named_argument_defs is owned by the caller so we just unassign it.
  args->named_argument_defs = NULL;
  args->nnamed_argument_defs = 0;

  assert(args->path);
  free(args->path);
  args->path = NULL;
}

bool karg_string(const arguments *args, const char *name, char **value) {
  assert(args);
  assert(name);
  named_argument *arg = get_named_argument(args, name);
  if (!arg) {
    return false;
  }
  *value = arg->value;
  return true;
}

bool karg_string_alloc(const arguments *args, const char *name, char **value) {
  if (!karg_string(args, name, value)) {
    return false;
  }
  clone(value, *value);
  return true;
}

bool karg_long(const arguments *args, const char *name, long *value) {
  char *str;
  if (!karg_string(args, name, &str)) {
    return false;
  }

  const int base = 10;
  char *end;
  *value = strtol(str, &end, base);
  if (end == str || *end != '\0') {
    return false;
  }
  return true;
}
