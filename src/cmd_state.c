#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "cmd_state.h"
#include "config.h"

static int show_state() {
  int fd = open("state", O_RDONLY);

  if(fd == -1) {
    if(errno == ENOENT) {
      printf("online");
      return 0;
    }
    perror("state");
    return 1;
  }

  char buffer[1024];
  ssize_t len;

  while((len = read(fd, buffer, 1023)) > 0) {
    write(1, buffer, len);
  }

  if(len == -1) {
    perror("write");
    close(fd);
    return 1;
  }

  close(fd);

  return 0;
}

int cmd_state(const char* newstate) {
  if(chdir("../../")==-1) {
    perror("../../");
    return 1;
  }

  if(newstate == NULL) {
    show_state();
  }
  else {
    return put_file("state", newstate);
  }
}

// vim: ts=2:sw=2:et:ai:tw=0
