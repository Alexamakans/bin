#include <assert.h>
#include <dirent.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#include <karg.h>

bool list_files(const char *path);

int main(int argc, char **argv) {
  const named_argument_definition arg_defs[1] = {
      {'p', "path", "."},
  };

  arguments args = {
      .named_argument_defs = arg_defs,
      .nnamed_argument_defs = sizeof(arg_defs) / sizeof(arg_defs[0]),
  };

  if (!karg_parse(&args, argc, argv)) {
    fprintf(stderr, "failed to parse arguments\n");
    return 1;
  }
  char *path;
  if (!karg_string(&args, "path", &path)) {
    fprintf(stderr, "failed getting argument 'path'\n");
    return 1;
  }

  if (!list_files(path)) {
    fprintf(stderr, "failed listing files in '%s'\n", path);
    return 1;
  }

  karg_free(&args);
}

bool list_files(const char *path) {
  assert(path);

  DIR *dir = opendir(path);
  if (dir == NULL) {
    perror("unable to open directory");
    return false;
  }

  struct dirent *entry;
  while ((entry = readdir(dir)) != NULL) {
    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
      continue;
    }

    const size_t real_path_len =
        1 + snprintf(NULL, 0, "%s/%s", path, entry->d_name);
    char *real_path = malloc(real_path_len);
    snprintf(real_path, real_path_len, "%s/%s", path, entry->d_name);

    //    struct stat statbuf;
    // #ifdef _DIRENT_HAVE_D_TYPE
    //    if (S_ISDIR(entry->d_type) || S_ISREG(entry->d_type)) {
    //      if (stat(real_path, &statbuf) != 0) {
    //        fprintf(stderr, "failed to stat entry: %s\n", entry->d_name);
    //        perror("stat");
    //        return false;
    //      }
    //    }
    // #else
    //    if (stat(real_path, &statbuf) != 0) {
    //      fprintf(stderr, "failed to stat entry: %s\n", entry->d_name);
    //      perror("stat");
    //      continue;
    //    }
    // #endif // _DIRENT_HAVE_D_TYPE

    printf("%s\n", entry->d_name);
  }

  closedir(dir);
  return true;
}
