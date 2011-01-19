#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "queue.h"
#include "cmd_cat.h"

int cmd_cat(const char* jobid, const char* file) {
  if(chdir(jobid)==-1) {
    perror(jobid);
    return 1;
  }

  int fd = open(file, O_RDONLY);

  if(fd == -1) {
    perror(file);
    return 1;
  }

  char buffer[1024];
  ssize_t len;

  while((len = read(fd, buffer, 1023)) > 0) {
    write(1, buffer, len);
  }
  if(len == -1) {
    perror("write");
    return 1;
  }

  close(fd);
}

// vim: ts=2:sw=2:et:ai:tw=0
