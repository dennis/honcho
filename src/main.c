#include <string.h>
#include <limits.h>
#include <unistd.h>
#include <stdlib.h>

#include "queue.h"
#include "cmd_cat.h"
#include "cmd_submit.h"
#include "cmd_execute.h"
#include "cmd_status_overview.h"
#include "cmd_status_query.h"

static char cwd[PATH_MAX];

static void cleanup() {
  if(cwd)
    chdir(cwd);
}

static char* join_args(int argc, char* argv[], int from) {
  char* cmd = 0;
  int cmdLen = 0;
  int i;

  for(i=from; i < argc; i++) 
    cmdLen += strlen(argv[i]) + 1; 

  cmd = malloc(cmdLen);

  for(i=from; i < argc; i++) {
    strcat(cmd, argv[i]);
    strcat(cmd, " ");
  }

  cmd[strlen(cmd)-1] = 0;

  return cmd;
}

int main(int argc, char *argv[]) {
  cwd[0] = 0;

  if(getcwd(cwd, PATH_MAX)) {
    atexit(cleanup);

    if(chdir(get_queuedir())==-1) {
      perror(get_queuedir());
      return 1;
    }

    if(argc > 3 && strcmp(argv[1], "execute") == 0) {
      int i;
      char* cmd = 0;
      char* jobId = 0;
      int wait = 0;

      for(i = 2; i < argc; i++) {
        if(strcmp("-w", argv[i]) == 0) {
          wait = 1;
        }
        else if(jobId == NULL) {
          jobId = argv[i];
        }
        else {
          cmd = join_args(argc, argv, i);
          break;
        }
      }

      return cmd_execute(jobId, cmd, wait);
    }
    else if(argc == 4 && strcmp(argv[1], "cat") == 0) {
      return cmd_cat(argv[2], argv[3]);
    }
    else if(argc == 3 && strcmp(argv[1], "status") == 0) {
      return cmd_status_query(argv[2]);
    }
    else if(argc == 2 && strcmp(argv[1], "status") == 0) {
      return cmd_status_overview();
    }
    else if(argc > 2 && strcmp(argv[1], "submit") == 0) {
      return cmd_submit(join_args(argc, argv, 2));
    }
    else {
      puts("honcho execute <ID> <cmd ..>");
      puts("honcho cat <ID> <file>");
      puts("honcho status [ID]");
      puts("honcho submit <cmd ..>");
      return 1;
    }
  }
  else {
    perror("getcwd");
    return 1;
  }
}

// vim: ts=2:sw=2:et:ai:tw=0
