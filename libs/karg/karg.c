#include "karg.h"

#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define assert_null_terminated(x) assert(x[sizeof(x) - 1] == '\0');

void parse_arguments(arguments *args, const int argc, char **argv) {
  init_arguments(args, argc, argv);
  if (argc == 1) {
    return;
  }

  const char *flag_name;
  // TODO: Map which args were consumed instead, put the rest into positional arguments?
  int args_consumed = 0;
  for (int i = 1; i < argc; ++i) {
    const char *arg_str = argv[i];
    assert_null_terminated(argv[i]);
    const size_t arg_len = strlen(argv[i]);
    if (arg_len >= 2 && strncmp(arg_str, "--", 2) == 0) {
      // named long flag
      flag_name = argv[i] + 2;
      assert(strlen(flag_name) > 0);
      assert(i + 1 < argc);
      ++i;
      set_named_argument(args, flag_name, argv[i]);
      args_consumed += 2;
    } else if (arg_len >= 1 && arg_str[0] == '-') {
      // short named argument
      flag_name = argv[i] + 1;
      assert(strlen(flag_name) == 1);
      assert(i + 1 < argc);
      ++i;
      set_named_argument(args, flag_name, argv[i]);
      args_consumed += 2;
    } else {
      continue;
    }
  }
}

void init_arguments(arguments *args, const int argc, char **argv) {
  assert(args);
  assert(argc > 0);
  assert(args->path == NULL);
  assert(args->named_arguments == NULL);
  clone(&args->path, argv[0]);

  args->named_arguments =
      calloc(args->nnamed_arguments, sizeof(named_argument_definition));
  for (size_t i = 0; i < args->nnamed_arguments; ++i) {
    if (args->named_argument_defs[i].default_value == NULL) {
      continue;
    }
    clone(&args->named_arguments[i].value,
          args->named_argument_defs[i].default_value);
  }
}

// find_flags returns -1 if no argument with a matching long/short name was
// found.
int64_t find_named_argument_index(arguments *args, const char *name) {
  assert(args);
  assert(name);
  assert_null_terminated(name);
  for (int64_t i = 0; i < (int64_t)args->nnamed_arguments; ++i) {
    assert_null_terminated(args->named_argument_defs[i].name);
    if (strcmp(args->named_argument_defs[i].name, name) == 0) {
      return i;
    }
    if (strlen(name) == 1) {
      if (args->named_argument_defs[i].short_name == name[0]) {
        return i;
      }
    }
  }
  return -1;
}

named_argument *get_named_argument(arguments *args, const char *name) {
  assert(args);
  assert(name);
  assert_null_terminated(name);
  int64_t arg_index = find_named_argument_index(args, name);
  if (arg_index == -1) {
    return NULL;
  }
  return &args->named_arguments[arg_index];
}

// set_named_argument returns -1 if no argument with a matching long/short name
// was found. NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
int set_named_argument(arguments *args, const char *name, const char *value) {
  assert(args);
  assert_null_terminated(name);
  assert_null_terminated(value);
  int64_t index = find_named_argument_index(args, name);
  if (index == -1) {
    return -1;
  }
  if (strcmp(args->named_argument_defs->name, name) == 0) {
    if (args->named_arguments[index].value != NULL) {
      free(args->named_arguments[index].value);
      args->named_arguments[index].value = NULL;
    }
    assert(args->named_arguments[index].value == NULL);
    clone(&args->named_arguments[index].value, value);
  }
  return 0;
}

positional_argument *get_positional_argument(arguments *args, size_t index) {
  assert(args);
  assert(index < args->npositional_arguments);
  return &args->positional_arguments[index];
}

void set_positional_argument(arguments *args, size_t index, const char *value) {
  assert(args);
  assert(index < args->npositional_arguments);
  if (args->positional_arguments[index].value != NULL) {
    free(args->positional_arguments[index].value);
    args->positional_arguments[index].value = NULL;
  }
  assert(args->positional_arguments[index].value == NULL);
  clone(&args->named_arguments[index].value, value);
}

void clone(char **dest, const char *src) {
  assert(dest);
  assert(src);
  assert_null_terminated(src);
  *dest = malloc(sizeof(char) * strlen(src));
  assert(*dest);
  assert_null_terminated(*dest);
}
