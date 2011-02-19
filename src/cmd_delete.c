#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <dirent.h>

#include "cmd_delete.h"
#include "utils.h"

static int deleter(const char* dir) {
	DIR* dfd = opendir(".");
  if(dfd == NULL) {
    perror(dir);
    return 1;
  }

  struct dirent *entry;
  while(entry = readdir(dfd)) {
    if(entry->d_type == DT_DIR && entry->d_name[0] != '.') {

      if(chdir(entry->d_name) == -1) {
        perror(entry->d_name);
        return 1;
      }

      if(deleter(entry->d_name) != 0) {
        return 1;
      }

      if(chdir("..") == -1) {
        perror("..");
        return 1;
      }
    }
    else if(entry->d_type == DT_REG || entry->d_type == DT_SOCK) {
      if(unlink(entry->d_name) == -1) {
        perror(entry->d_name);
        return 1;
      }
    }
    else if(entry->d_type == DT_SOCK) {
      if(rmdir(entry->d_name) == -1) {
        perror(entry->d_name);
        return 1;
      }
    }
  }

  closedir(dfd);

  if(chdir("..") == -1) {
    perror("..");
    return 1;
  }

  if(rmdir(dir) == -1) {
    perror(dir);
    return 1;
  }
  return 0;
}

int cmd_delete(const char* jobid) {
  if(chdir(jobid)==-1) {
    perror(jobid);
    return 1;
  }

  return deleter(jobid);
}

// vim: ts=2:sw=2:et:ai:tw=0
