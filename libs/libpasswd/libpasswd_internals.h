#ifndef LIBPASSWD_INTERNALS_H
#define LIBPASSWD_INTERNALS_H

#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <unistd.h>

#ifndef LIBPASSWD_PASSWD_FILE_PATH
#define LIBPASSWD_PASSWD_FILE_PATH "/etc/passwd"
#endif

#ifndef LIBPASSWD_BUFFER_SIZE
#define LIBPASSWD_BUFFER_SIZE 1024
#endif

typedef struct reader {
  FILE *file;
} reader;

bool init_reader(reader *reader);

bool line_count(reader *reader, size_t *count);

// returns NULL on EOF/error.
//
// len is set to the number of bytes read.
// len includes the newline character, but not the null terminator.
//
// The caller is responsible for freeing the memory.
bool next(reader *reader, char **line, size_t *len);

void deinit_reader(reader *reader);

#endif // LIBPASSWD_INTERNALS_H
