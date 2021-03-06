#include <errno.h>
#include <string.h>
#include <sys/types.h>

#include "cmd_status_query.h"
#include "utils.h"

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
