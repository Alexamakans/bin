#ifndef KARG_H
#define KARG_H

// TODO: Provide helper function for positional arguments from stdin?

#include "karg_structs.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

bool karg_parse(arguments *args, int argc, char **argv);
void karg_free(arguments *args);

bool karg_string(const arguments *args, const char *name, char **value);
bool karg_string_alloc(const arguments *args, const char *name, char **value);
bool karg_long(const arguments *args, const char *name, long *value);
#endif // KARG_H
