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

static int goto_queue(const char* queue) {
  if(chdir(get_queuedir(queue))==-1) {
    perror(get_queuedir(queue));
    return 1;
  }

  return 0;
}

static void usage() {
  puts("honcho [-q queue-name] execute <ID> <cmd ..>");
  puts("honcho [-q queue-name] cat <ID> <file>");
  puts("honcho [-q queue-name] status [ID]");
  puts("honcho [-q queue-name] submit <cmd ..>");
}

int main(int argc, char *argv[]) {
  enum { none, show_usage, do_execute, do_cat, do_status, do_submit };

  char* queue = "default";
  cwd[0] = 0;
  int cmd = none;

  if(getcwd(cwd, PATH_MAX)) {
    atexit(cleanup);

    int i = 0;
    for(i = 1; i < argc; i++) {
      if(cmd == none) {
        if(strcmp(argv[i], "execute") == 0) {
          cmd = do_execute;
        }
        else if(strcmp(argv[i], "cat") == 0) {
          cmd = do_cat;
        }
        else if(strcmp(argv[i], "status") == 0) {
          cmd = do_status;
        }
        else if(strcmp(argv[i], "submit") == 0) {
          cmd = do_submit;
        }
        else if(strcmp("-q", argv[i]) == 0) {
          if(i+1 < argc) {
            i++;
            queue = argv[i];
          }
          else {
            cmd = show_usage;
            break;
          }
        }
        else {
          cmd = show_usage;
          break;
        }
      }
      else {
        break;
      }
    }

    if(goto_queue(queue) == 1)
      return 1;

    switch(cmd) {
      case none:
      case show_usage:
        usage();
        return 0;
        break;
      case do_execute: {
          char* cmd = 0;
          char* jobId = 0;
          int wait = 0;

          for(; i < argc; i++) {
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

          if(queue == NULL || jobId == NULL) {
            usage();
            return 1;
          }

          return cmd_execute(queue, jobId, cmd, wait);
        }
        break;
      case do_cat:
        if(argc-i != 2) {
          usage();
          return 1;
        }
        return cmd_cat(argv[i], argv[i+1]);
      case do_status:
        if(argc-i == 0) {
          return cmd_status_overview();
        }
        else if(argc-i == 1) {
          return cmd_status_query(argv[i]);
        }
        else {
          usage();
          return 1;
        }
      case do_submit:
        if(argc - i >= 0) {
          return cmd_submit(join_args(argc, argv, i)); 
        }
        else {
          usage();
          return 1;
        }
      default:
        puts("Bad code");
    }
  }
  else {
    perror("getcwd");
    return 1;
  }
}

// vim: ts=2:sw=2:et:ai:tw=0
