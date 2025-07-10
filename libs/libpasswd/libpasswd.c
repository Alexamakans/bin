#include "libpasswd.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "libpasswd_internals.h"

bool passwdinfo_create(passwdinfo *info) {
  assert(info);

  reader reader;
  init_reader(&reader);

  size_t num_lines;
  if (!line_count(&reader, &num_lines)) {
    fprintf(stderr, "failed getting line count\n");
    return false;
  }
  char **lines = (char **)calloc(num_lines, sizeof(char *));
  size_t line_len;
  for (size_t i = 0; i < num_lines; ++i) {
    if (!next(&reader, &lines[i], &line_len)) {
      fprintf(stderr, "error reading line %zu\n", i);
      return false;
    }
  }

  deinit_reader(&reader);

  info->names = (char **)calloc(num_lines, sizeof(char *));
  info->user_ids = calloc(num_lines, sizeof(uint32_t));
  info->group_ids = calloc(num_lines, sizeof(uint32_t));

  // name:x:uid:gid:desc:root:shell
  const int uid_base = 10;
  const int gid_base = 10;
  for (size_t i = 0; i < num_lines; ++i) {
    char *line = lines[i];
    char *token = strtok(line, ":");
    info->names[i] = calloc(strlen(token) + 1, sizeof(char));
    strcpy(info->names[i], token);
    token = strtok(NULL, ":");
    token = strtok(NULL, ":");
    info->user_ids[i] = (uint32_t)strtoul(token, NULL, uid_base);
    token = strtok(NULL, ":");
    info->group_ids[i] = (uint32_t)strtoul(token, NULL, gid_base);
    free(line);
  }

  free((void *)lines);

  info->nentries = num_lines;

  return true;
}

bool passwd_find_index_by_name(const passwdinfo *info, const char *name,
                               size_t *index) {
  assert(info);
  assert(name);
  assert(index);

  size_t name_len = strlen(name);
  for (size_t i = 0; i < info->nentries; ++i) {
    if (strncmp(info->names[i], name, name_len) == 0) {
      *index = i;
      return true;
    }
  }
  return false;
}

bool passwd_find_index_by_uid(const passwdinfo *info, uint32_t uid,
                              size_t *index) {
  assert(info);
  assert(index);

  for (size_t i = 0; i < info->nentries; ++i) {
    if (info->user_ids[i] == uid) {
      *index = i;
      return true;
    }
  }
  return false;
}
bool passwd_find_index_by_gid(const passwdinfo *info, uint32_t gid,
                              size_t *index) {
  assert(info);
  assert(index);

  for (size_t i = 0; i < info->nentries; ++i) {
    if (info->group_ids[i] == gid) {
      *index = i;
      return true;
    }
  }
  return false;
}

void passwdinfo_destroy(passwdinfo *info) {
  assert(info);
  assert(info->group_ids);
  assert(info->user_ids);
  assert(info->names);

  free(info->group_ids);
  info->group_ids = NULL;
  free(info->user_ids);
  info->user_ids = NULL;
  for (size_t i = 0; i < info->nentries; ++i) {
    free(info->names[i]);
    info->names[i] = NULL;
  }
  free((void *)info->names);
  info->names = NULL;
  info->nentries = 0;
}
