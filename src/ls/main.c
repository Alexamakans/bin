#include <assert.h>
#include <ctype.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <time.h>

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
size_t count_digits(double value);
bool xstat(const char *path, struct dirent *entry, struct stat *statbuf);
char *make_fmt_string(direntstat *files, size_t nfiles);
// allocates memory if *presult is NULL
void make_mode_string(unsigned int mode, char **presult);

// NOLINTNEXTLINE(readability-identifier-length)
int default_sort(const void *va, const void *vb);

int main(int argc, char **argv) {
  tzset();

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
  qsort(files, nfiles, sizeof(direntstat), default_sort);
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
  char *mode_string = NULL;

  struct tm timedata;
  memset(&timedata, 0, sizeof(timedata));
  const size_t last_modified_max_len = 20;
  char *last_modified = calloc(last_modified_max_len, sizeof(char));
  if (last_modified == NULL) {
    fprintf(stderr, "failed to allocate memory for last modified string\n");
    return 1;
  }

  for (size_t i = 0; i < nfiles; ++i) {
    direntstat *file = &files[i];

    struct timespec mtim = file->stat.st_mtim;
    if (localtime_r(&mtim.tv_sec, &timedata) == NULL) {
      fprintf(stderr, "failed to format last modified time into date\n");
      return 1;
    }

    make_mode_string(file->stat.st_mode, &mode_string);
    strftime(last_modified, last_modified_max_len, "%b %e %H:%M", &timedata);
    printf(fmt, mode_string, file->stat.st_nlink, file->user_name,
           file->group_name, file->stat.st_size, last_modified,
           file->entry.d_name);
    free(file->group_name);
    file->group_name = NULL;
    free(file->user_name);
    file->user_name = NULL;
  }
  if (last_modified != NULL) {
    free(last_modified);
    last_modified = NULL;
  }
  free(mode_string);
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
  size_t uid_width = 0;
  size_t gid_width = 0;
  size_t size_width = 0;
  for (size_t i = 0; i < nfiles; ++i) {
    size_t width = count_digits((double)files[i].stat.st_nlink);
    link_count_width = link_count_width > width ? link_count_width : width;

    width = strlen(files[i].user_name);
    uid_width = uid_width > width ? uid_width : width;

    width = strlen(files[i].group_name);
    gid_width = gid_width > width ? gid_width : width;

    width = count_digits((double)files[i].stat.st_size);
    size_width = size_width > width ? size_width : width;
  }

  size_t fmt_len =
      // mode string, link count, uid width, gid width, size width,
      // last modified, filename
      1 + snprintf(NULL, 0, "%%06s %%%zulu %%%zus %%%zus %%%zuld %%s %%s\n",
                   link_count_width, uid_width, gid_width, size_width);
  char *fmt = malloc(fmt_len);
  if (fmt == NULL) {
    fprintf(stderr, "failed to allocate memory for fmt\n");
    abort();
  }
  snprintf(fmt, fmt_len, "%%06s %%%zulu %%%zus %%%zus %%%zuld %%s %%s\n",
           link_count_width, uid_width, gid_width, size_width);
  return fmt;
}

// fill_file_type advances the index and returns it.
size_t fill_file_type(char *result, size_t index, const unsigned int mode,
                      const size_t bit_offset) {
  const char file_type_octal = (char)((mode >> bit_offset) & 017);
  switch (file_type_octal) {
  case DT_REG:
    result[index++] = '-';
    break;
  case DT_DIR:
    result[index++] = 'd';
    break;
  case DT_LNK:
    result[index++] = 'l';
    break;
  case DT_FIFO:
    result[index++] = 'p';
    break;
  case DT_BLK:
    result[index++] = 'b';
    break;
  case DT_CHR:
    result[index++] = 'c';
    break;
  case DT_SOCK:
    result[index++] = 's';
    break;
  default:
    result[index++] = '?';
  }
  return index;
}

// fill_rwx advances the index and returns it.
size_t fill_rwx(char *result, size_t index, const unsigned int mode,
                const size_t bit_offset, const bool special_condition,
                const char special_has_exec_char_lowercase) {
  assert(islower(special_has_exec_char_lowercase));
#define HAS_READ(x) (((x) & ~04) != (x))
#define HAS_WRITE(x) (((x) & ~02) != (x))
#define HAS_EXEC(x) (((x) & ~01) != (x))
  const char perms = (char)((mode >> bit_offset) & 07);
  if (HAS_READ(perms)) {
    result[index++] = 'r';
  } else {
    result[index++] = '-';
  }
  if (HAS_WRITE(perms)) {
    result[index++] = 'w';
  } else {
    result[index++] = '-';
  }
  if (HAS_EXEC(perms)) {
    result[index++] = 'x';
  } else {
    result[index++] = '-';
  }
  if (special_condition) {
    const size_t exec_index = index - 1;
    if (result[exec_index] == 'x') {
      result[exec_index] = special_has_exec_char_lowercase;
    } else {
      result[exec_index] = (char)toupper(special_has_exec_char_lowercase);
    }
  }
  return index;
#undef HAS_READ
#undef HAS_WRITE
#undef HAS_EXEC
}

void make_mode_string(const unsigned int mode, char **presult) {
  assert(presult);

  const size_t mode_num_chars = 11;
  static const size_t octal_digit_bit_size = 3;
  if (*presult == NULL) {
    *presult = calloc(mode_num_chars + 1, sizeof(char));
  }
  char *result = *presult;

  static const size_t file_type_bit_offset = 4 * octal_digit_bit_size;
  size_t index = fill_file_type(result, 0, mode, file_type_bit_offset);

#define STICKY_MASK 01
#define SETUID_MASK 02
#define SETGID_MASK 04
  static const size_t special_flag_bit_offset = 3 * octal_digit_bit_size;
  const bool is_sticky = (mode >> special_flag_bit_offset) & STICKY_MASK;
  const bool is_setuid = (mode >> special_flag_bit_offset) & SETUID_MASK;
  const bool is_setgid = (mode >> special_flag_bit_offset) & SETGID_MASK;

  index =
      fill_rwx(result, index, mode, 2 * octal_digit_bit_size, is_setuid, 's');
  index =
      fill_rwx(result, index, mode, 1 * octal_digit_bit_size, is_setgid, 's');
  index =
      fill_rwx(result, index, mode, 0 * octal_digit_bit_size, is_sticky, 't');
  result[index++] = '\0';

#undef STICKY_MASK
#undef SETUID_MASK
#undef SETGID_MASK
}

// NOLINTBEGIN(readability-identifier-length)
int default_sort(const void *va, const void *vb) {
  const direntstat *a = (direntstat *)va;
  const direntstat *b = (direntstat *)vb;
  // NOLINTEND(readability-identifier-length)
  const char *simplified_name_a = a->entry.d_name;
  while (simplified_name_a[0] == '.') {
    ++simplified_name_a;
  }
  const char *simplified_name_b = b->entry.d_name;
  while (simplified_name_b[0] == '.') {
    ++simplified_name_b;
  }
  // Revert case change
  int cmp = strcasecmp(simplified_name_a, simplified_name_b);
  return cmp;
}
