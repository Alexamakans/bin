#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

// TODO: format st_mode -> drwxrwxrws.
// TODO: Translate uid and gid to names by parsing /etc/passwd
// TODO: Output the last modified time as a human readable date
// TODO: Sorting

#define __USE_MISC
#include <dirent.h>

#include <karg.h>
#include <libpasswd.h>

typedef struct direntstat {
  struct dirent entry;
  struct stat stat;
  char *user_name;
  char *group_name;
} direntstat;

direntstat *list_files(const char *path, size_t *nfiles,
                       const passwdinfo *passwdinfo);
char *to_octal(unsigned int value);
size_t count_digits(double value);
bool xstat(const char *path, struct dirent *entry, struct stat *statbuf);
char *make_fmt_string(direntstat *files, size_t nfiles);

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

  size_t nfiles;

  passwdinfo info;
  passwdinfo_create(&info);
  direntstat *files = list_files(path, &nfiles, &info);
  passwdinfo_destroy(&info);
  if (files == NULL) {
    fprintf(stderr, "failed listing files in '%s'\n", path);
    return 1;
  }

  long total_blocks = 0;
  for (size_t i = 0; i < nfiles; ++i) {
    // TODO: Custom block size like ls has.
    //
    // Currently using their default, which is 1024.
    // st_blocks is how many 512-byte blocks there are, so we divide by 2.
    total_blocks += files[i].stat.st_blocks / 2;
  }
  printf("total %ld\n", total_blocks);

  char *fmt = make_fmt_string(files, nfiles);
  for (size_t i = 0; i < nfiles; ++i) {
    direntstat *file = &files[i];

    printf(fmt, file->stat.st_mode, file->stat.st_nlink, file->stat.st_ino,
           file->user_name, file->group_name, file->stat.st_size,
           file->entry.d_name);
    free(file->group_name);
    file->group_name = NULL;
    free(file->user_name);
    file->user_name = NULL;
  }
  free(fmt);
  free(files);

  karg_free(&args);
}

direntstat *list_files(const char *path, size_t *nfiles,
                       const passwdinfo *passwdinfo) {
  assert(path);
  assert(nfiles);
  assert(passwdinfo);
  *nfiles = 0;

  DIR *dir = opendir(path);
  if (dir == NULL) {
    fprintf(stderr, "failed to open directory: %s\n", path);
    perror("opendir");
    return NULL;
  }

  struct dirent *entry;
  while ((entry = readdir(dir)) != NULL) {
    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
      continue;
    }
    ++(*nfiles);
  }
  rewinddir(dir);

  direntstat *entries = calloc(*nfiles, sizeof(struct direntstat));
  direntstat *result = entries;
  if (entries == NULL) {
    fprintf(stderr, "failed to allocate memory for entries\n");
    abort();
  }
  while ((entry = readdir(dir)) != NULL) {
    if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
      continue;
    }

    const size_t real_path_len =
        1 + snprintf(NULL, 0, "%s/%s", path, entry->d_name);
    char *real_path = calloc(real_path_len, 1);
    if (real_path == NULL) {
      fprintf(stderr, "failed to allocate memory for real_path\n");
      abort();
    }
    snprintf(real_path, real_path_len, "%s/%s", path, entry->d_name);

#ifdef _DIRENT_HAVE_D_TYPE
    if (!xstat(real_path, entry, &entries->stat)) {
      fprintf(stderr, "failed to xstat entry: %s\n", entry->d_name);
    }
#else
    if (stat(real_path, &statbuf) != 0) {
      fprintf(stderr, "failed to stat entry: %s\n", entry->d_name);
      perror("stat");
    }
#endif // _DIRENT_HAVE_D_TYPE
    entries->entry = *entry;
    size_t index;
    if (!passwd_find_index_by_uid(passwdinfo, entries->stat.st_uid, &index)) {
      fprintf(stderr, "failed getting user name for file: %s\n", real_path);
    } else {
      char *name = passwdinfo->names[index];
      entries->user_name = calloc(1, strlen(name) + 1);
      strcpy(entries->user_name, name);
    }
    if (!passwd_find_index_by_gid(passwdinfo, entries->stat.st_gid, &index)) {
      fprintf(stderr, "failed getting user name for file: %s\n", real_path);
    } else {
      char *name = passwdinfo->names[index];
      entries->group_name = calloc(1, strlen(name) + 1);
      strcpy(entries->group_name, name);
    }

    free(real_path);

    ++entries;
  }
  closedir(dir);
  return result;
}

size_t count_digits(const double value) {
  if (value > 0) {
    return (size_t)log10(value) + 1;
  }
  if (value < 0) {
    return (size_t)log10(-value) + 1;
  }
  return 1;
}

bool xstat(const char *path, struct dirent *entry, struct stat *statbuf) {
  if (entry->d_type == DT_DIR || entry->d_type == DT_REG) {
    if (stat(path, statbuf) != 0) {
      perror("stat");
      return false;
    }
  } else if (entry->d_type == DT_UNKNOWN || entry->d_type == DT_LNK) {
    if (lstat(path, statbuf) != 0) {
      perror("lstat");
      return false;
    }
  } else {
    fprintf(stderr, "unaccounted for type: %cu\n", entry->d_type);
    return false;
  }
  return true;
}

char *make_fmt_string(direntstat *files, size_t nfiles) {
  size_t link_count_width = 0;
  size_t ino_width = 0;
  size_t uid_width = 0;
  size_t gid_width = 0;
  size_t size_width = 0;
  for (size_t i = 0; i < nfiles; ++i) {
    size_t width = count_digits((double)files[i].stat.st_nlink);
    link_count_width = link_count_width > width ? link_count_width : width;

    width = count_digits((double)files[i].stat.st_ino);
    ino_width = ino_width > width ? ino_width : width;

    width = strlen(files[i].user_name);
    uid_width = uid_width > width ? uid_width : width;

    width = strlen(files[i].group_name);
    gid_width = gid_width > width ? gid_width : width;

    width = count_digits((double)files[i].stat.st_size);
    size_width = size_width > width ? size_width : width;
  }

  size_t fmt_len =
      // mode, link count, ino width, uid width, gid width, size width, filename
      1 + snprintf(NULL, 0, "%%06o %%%zulu %%%zulu %%%zus %%%zus %%%zuld %%s\n",
                   link_count_width, ino_width, uid_width, gid_width,
                   size_width);
  char *fmt = malloc(fmt_len);
  if (fmt == NULL) {
    fprintf(stderr, "failed to allocate memory for fmt\n");
    abort();
  }
  snprintf(fmt, fmt_len, "%%06o %%%zulu %%%zulu %%%zus %%%zus %%%zuld %%s\n",
           link_count_width, ino_width, uid_width, gid_width, size_width);
  return fmt;
}
