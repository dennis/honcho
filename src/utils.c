#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

#include "utils.h"
#include "config.h"

char queuedir[PATH_MAX];

int put_file(const char* filename, const char *data) {
  int fd = open(filename, O_WRONLY|O_CREAT, FILE_MODE);

  if(fd == -1) {
    char error[512];
    snprintf(error, 512, "Cannot create '%s' for writing", filename);
    perror(error);
    return 1;
  }

  if(write(fd, data, strlen(data)) == -1) {
    char error[512];
    snprintf(error, 512, "Cannot write to '%s'", filename);
    perror(error);
    return 1;
  }

  return 0;
}

int cat_file(int outfd, const char* file) {
  int fd = open(file, O_RDONLY);

  if(fd == -1) {
    perror(file);
    return 1;
  }

  char buffer[1024];
  ssize_t len;

  while((len = read(fd, buffer, 1023)) > 0) {
    write(outfd, buffer, len);
  }
  if(len == -1) {
    perror("write");
    close(fd);
    return 1;
  }

  close(fd);

  return 0;
}

const char* get_queuedir(const char* name) {
  char* env = getenv("HONCHO_DIR");

  queuedir[0] = 0;

  if(env) {
    strcat(queuedir, env);
  }
  else {
    strcat(queuedir, DATA_DIR);
  }
  strcat(queuedir, "/queue");

  int i = strlen(queuedir)-1;
  while(queuedir[i] == '/' && i > 0)
    i--;

  queuedir[i+1] = 0;

  strcat(queuedir, "/");
  strcat(queuedir, name);

  return queuedir;
}


// vim: ts=2:sw=2:et:ai:tw=0
