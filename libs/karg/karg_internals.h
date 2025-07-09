#ifndef KARG_INTERNALS_H
#define KARG_INTERNALS_H

#include "karg_structs.h"

#include <stdbool.h>

#ifndef NDEBUG
#define debug_printf(...) printf(__VA_ARGS__)
#else
#define debug_printf(...)                                                      \
  do {                                                                         \
  } while (0)
#endif

void init_arguments(arguments *args, const char *argv0);

int find_named_argument_index(const arguments *args, const char *name);

named_argument *get_named_argument(const arguments *args, const char *name);
bool set_named_argument(const arguments *args, const char *name,
                        const char *value);

void clone(char **dest, const char *src);

#endif // KARG_INTERNALS_H
