#ifndef KARG_H
#define KARG_H

// TODO: Provide helper function for positional arguments from stdin?

#include <stddef.h>
#include <stdint.h>

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

  named_argument_definition *named_argument_defs;
  named_argument *named_arguments;
  size_t nnamed_arguments;

  positional_argument *positional_arguments;
  size_t npositional_arguments;
} arguments;

void parse_arguments(arguments *args, int argc, char **argv);
void init_arguments(arguments *args, int argc, char **argv);

int64_t find_named_argument_index(arguments *args, const char *name);

named_argument *get_named_argument(arguments *args, const char *name);
int set_named_argument(arguments *args, const char *name, const char *value);

void clone(char **dest, const char *src);
#endif // KARG_H
