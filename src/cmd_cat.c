#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "cmd_cat.h"
#include "utils.h"

int cmd_cat(const char* jobid, const char* file) {
  if(chdir(jobid)==-1) {
    perror(jobid);
    return 1;
  }

  int rc = cat_file(1, file);

  chdir("..");

  return rc;
}

// vim: ts=2:sw=2:et:ai:tw=0
