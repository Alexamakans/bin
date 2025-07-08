#include <karg.h>
#include <string.h>

int main(int argc, char** argv) {
  named_argument_definition arg_defs[2] = {
    { NULL, 0, "path", strlen("path") },
    { "v", strlen("v"), "verbose", strlen("verbose") },
  };

  arguments *args = parse_arguments(argc, argv, &arg_defs[0], sizeof(arg_defs)/sizeof(arg_defs[0]))
}
