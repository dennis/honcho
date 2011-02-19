#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include "cmd_signal.h"
#include "utils.h"

int cmd_signal(const char* jobid, int signum) {
  if(chdir(jobid)==-1) {
    perror(jobid);
    return 1;
  }
  
  int fd = connect_to_control_socket();

  if(fd != -1) {
    char buffer[1024];
    ssize_t len;

    snprintf(buffer, 1024, "SIGNAL %d\n", signum);

    write(fd, buffer, strlen(buffer));

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
    write(2, "Process not running\n", 20);
    chdir("..");
    return 1;
  }
  return 1;
}

// vim: ts=2:sw=2:et:ai:tw=0
