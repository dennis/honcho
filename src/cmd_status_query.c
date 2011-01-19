#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "cmd_status_query.h"

static int connect_to_control_socket() {
  struct sockaddr_un remote;
  int fd;

  if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
    perror("socket");
    return -1;
  }

  remote.sun_family = AF_UNIX;
  strcpy(remote.sun_path, "control");
  int len = strlen(remote.sun_path) + sizeof(remote.sun_family);

  if (connect(fd, (struct sockaddr *)&remote, len) == -1) {
    close(fd);
    return -1;
  }

  return fd;
}

int cmd_status_query(const char* jobid) {
  if(chdir(jobid)==-1) {
    perror(jobid);
    return 1;
  }
  
  int fd = connect_to_control_socket();

  if(fd != -1) {
    char buffer[1024];
    ssize_t len;

    write(fd, "STATUS\n", 7);

    while((len = read(fd, buffer, 1023)) > 0) {
      write(1, buffer, len);
    }

    if(len == -1) {
      perror("write");
    }

    chdir("..");
    close(fd);
    return len == -1 ? 1 : 0;
  }
  else {
    chdir("..");
    return cmd_cat(jobid, "status");
  }
}

// vim: ts=2:sw=2:et:ai:tw=0
