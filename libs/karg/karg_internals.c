#include "karg_internals.h"
#include "karg_structs.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void init_arguments(arguments *args, const char *argv0) {
  assert(args);
  assert(args->path == NULL);
  assert(args->named_arguments == NULL);
  clone(&args->path, argv0);
  args->named_arguments =
      calloc(args->nnamed_argument_defs, sizeof(named_argument_definition));
  for (size_t i = 0; i < args->nnamed_argument_defs; ++i) {
    if (args->named_argument_defs[i].default_value == NULL) {
      continue;
    }
    clone(&args->named_arguments[i].value,
          args->named_argument_defs[i].default_value);
  }
}

// find_flags returns -1 if no argument with a matching long/short name was
// found.
int find_named_argument_index(const arguments *args, const char *name) {
  assert(args);
  assert(name);
  for (int i = 0; i < (int)args->nnamed_argument_defs; ++i) {
    if (strcmp(args->named_argument_defs[i].name, name) == 0) {
      return i;
    }
    if (strlen(name) == 1) {
      if (args->named_argument_defs[i].short_name == '\0') {
        continue;
      }
      if (args->named_argument_defs[i].short_name == name[0]) {
        return i;
      }
    }
  }
  return -1;
}

named_argument *get_named_argument(const arguments *args, const char *name) {
  assert(args);
  assert(name);
  int arg_index = find_named_argument_index(args, name);
  if (arg_index == -1) {
    return NULL;
  }
  return &args->named_arguments[arg_index];
}

// set_named_argument returns -1 if no argument with a matching long/short name
// was found. NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
bool set_named_argument(const arguments *args, const char *name,
                        const char *value) {
  assert(args);
  int index = find_named_argument_index(args, name);
  if (index == -1) {
    debug_printf("couldn't find flag with name %s\n", name);
    return false;
  }
  if (args->named_arguments[index].value != NULL) {
    free(args->named_arguments[index].value);
    args->named_arguments[index].value = NULL;
  }
  assert(args->named_arguments[index].value == NULL);
  clone(&args->named_arguments[index].value, value);
  return true;
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
  *dest = malloc(strlen(src) + 1);
  assert(*dest);
  strcpy(*dest, src);
  assert(strcmp(*dest, src) == 0);
}
