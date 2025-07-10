#include "libpasswd_internals.h"

#include <assert.h>
#include <stdio.h>

bool init_reader(reader *reader) {
  reader->file = fopen(LIBPASSWD_PASSWD_FILE_PATH, "r");
  if (reader->file == NULL) {
    fprintf(stderr, "failed to open %s for reading",
            LIBPASSWD_PASSWD_FILE_PATH);
    perror("failed to open passwd file");
    return false;
  }
  return true;
}

// TODO: Investigate how to handle error during getc loop, or if we even need
// to.
bool line_count(reader *reader, size_t *count) {
  assert(reader);
  assert(reader->file);
  assert(count);

  size_t num_lines = 0;
  int chr;
  while ((chr = getc(reader->file)) != EOF) {
    if (chr == '\n') {
      ++num_lines;
    }
  }
  *count = num_lines;

  if (fseek(reader->file, 0, SEEK_SET) == -1) {
    fprintf(stderr, "failed resetting file to beginning\n");
    perror("fseek");
    return false;
  }

  return true;
}

bool next(reader *reader, char **line, size_t *len) {
  assert(reader);
  assert(reader->file);
  assert(line);
  assert(len);
  *line = NULL;

  ssize_t bytes_read = getline(line, len, reader->file);
  if (feof(reader->file)) {
    return false;
  }
  if (bytes_read < 0) {
    perror("getline");
    return false;
  }
  return true;
}

void deinit_reader(reader *reader) {
  assert(reader->file);

  fclose(reader->file);
  reader->file = NULL;
}
