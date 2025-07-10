#ifndef LIBPASSWD_H
#define LIBPASSWD_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

typedef struct passwdinfo {
  size_t nentries;
  char **names;
  uint32_t *user_ids;
  uint32_t *group_ids;
} passwdinfo;

bool passwdinfo_create(passwdinfo *info);

bool passwd_find_index_by_name(const passwdinfo *info, const char *name, size_t *index);
bool passwd_find_index_by_uid(const passwdinfo *info, uint32_t uid, size_t *index);
bool passwd_find_index_by_gid(const passwdinfo *info, uint32_t gid, size_t *index);

void passwdinfo_destroy(passwdinfo *info);

#endif // LIBPASSWD_H
