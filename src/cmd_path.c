#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <linux/limits.h>

#include "cmd_path.h"
#include "utils.h"

int cmd_path(const char* jobid) {
  char path[PATH_MAX];

  if( jobid && chdir(jobid)==-1) {
    perror(jobid);
    return 1;
  }

  if(getcwd(path, PATH_MAX) == NULL) {
    perror("cwd");
    return 1;
  }

  puts(path);

  return 0;
}

// vim: ts=2:sw=2:et:ai:tw=0
