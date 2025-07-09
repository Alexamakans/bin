#include <assert.h>
#include <karg.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv) {
  named_argument_definition arg_defs[3] = {
      {'\0', "path", NULL},
      {'s', "string", "default-value"},
      {'n', "number", "3"},
  };

  arguments args = {
      .named_argument_defs = arg_defs,
      .nnamed_argument_defs = sizeof(arg_defs) / sizeof(arg_defs[0]),
  };

  if (!karg_parse(&args, argc, argv)) {
    printf("failed to parse arguments\n");
    return 1;
  }

  assert(args.path);
  assert(strlen(args.path) > 0);
  printf("path=%s\n", args.path);

  char *string;
  if (!karg_string(&args, "s", &string)) {
    printf("failed getting argument \"s\" as string\n");
    return 1;
  }
  assert(string);
  printf("string=%s\n", string);

  long number;
  if (!karg_long(&args, "n", &number)) {
    printf("failed getting argument \"n\" as long\n");
    return 1;
  }
  printf("number=%ld\n", number);

  karg_free(&args);
}
