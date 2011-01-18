#include <string.h>
#include <limits.h>
#include <unistd.h>

#include "queue.h"
#include "worker_execute.h"
#include "worker_cat.h"

static char cwd[PATH_MAX];

static void cleanup() {
  if(cwd)
    chdir(cwd);
}

int main(int argc, char *argv[]) {
  cwd[0] = 0;

  if(getcwd(cwd, PATH_MAX)) {
    atexit(cleanup);

    if(chdir(get_queuedir())==-1) {
      perror(get_queuedir());
      return 1;
    }

    if(argc > 2 && strcmp(argv[1], "execute") == 0) {
      int i;
      int cmdLen = 0;
      char* cmd = 0;

      for(i=0; i < argc-3; i++) 
        cmdLen += strlen(argv[i+3]) + 1; 

      cmd = malloc(cmdLen);
      for(i=0; i < argc-3; i++) {
        strcat(cmd, argv[i+3]);
        strcat(cmd, " ");
      }

      if(argc-3) 
        cmd[strlen(cmd)-1] = 0;

      return worker_execute(argv[2], cmd);
    }
    else if(argc == 4 && strcmp(argv[1], "cat") == 0) {
      return worker_cat(argv[2], argv[3]);
    }
    else {
      puts("honcho execute <ID> <cmd> [args..]");
      puts("honcho cat <ID> <file>");
      return 1;
    }
  }
  else {
    perror("getcwd");
    return 1;
  }
}

// vim: ts=2:sw=2:et:ai:tw=0
