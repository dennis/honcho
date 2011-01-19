#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>

#include "cmd_status_overview.h"

int cmd_status_overview() {
	DIR* dfd = opendir(".");
  if(dfd == NULL) {
    perror("opendir");
    return 1;
  }

  struct dirent *entry;
  while(entry = readdir(dfd)) {
    if(entry->d_type == DT_DIR && entry->d_name[0] != '.') {
      puts(entry->d_name);
      cmd_status_query(entry->d_name);
      puts("");
    }
  }

  closedir(dfd);
}
// vim: ts=2:sw=2:et:ai:tw=0
