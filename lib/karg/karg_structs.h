#ifndef KARG_STRUCTS_H
#define KARG_STRUCTS_H

#include <stddef.h>

typedef struct named_argument {
  char *value;
} named_argument;

typedef struct positional_argument {
  char *value;
} positional_argument;

typedef struct named_argument_definition {
  const char short_name;
  const char *name;
  const char *default_value;
} named_argument_definition;

typedef struct arguments {
  char *path;

  const named_argument_definition *named_argument_defs;
  named_argument *named_arguments;
  size_t nnamed_argument_defs;

  positional_argument *positional_arguments;
  size_t npositional_arguments;
} arguments;
#endif // KARG_STRUCTS_H
