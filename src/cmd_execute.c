#include <limits.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <time.h>
#include <sys/un.h>
#include <stdlib.h>


#include "cmd_execute.h"
#include "queue.h"
#include "config.h"

#define STDBUF_SIZE 1024

enum job_status { job_none = 0, job_started, job_done };

struct job_state_t {
  pid_t pid;
  time_t started_time;
  enum job_status status;
};

static int create_file_content(const char* filename, const char *data) {
  int fd = open(filename, O_WRONLY|O_CREAT, FILE_MODE);
  if(fd == -1) {
    char error[512];
    snprintf(error, 512, "Cannot create '%s' for writing", filename);
    perror(error);
    return 1;
  }

  if(write(fd, data, strlen(data)) == -1) {
    char error[512];
    snprintf(error, 512, "Cannot write to '%s'", filename);
    perror(error);
    return 1;
  }

  return 0;
}

static int prepare_new_job(const char* jobid) {
  if(mkdir(jobid, DIR_MODE)==-1) {
    if(errno == EEXIST) {
      const char* error = "job-id already used, ignoring\n";
      write(2, error, strlen(error));
    }
    else {
      char error[512];
      snprintf(error, 512, "Cannot create %s/%s", get_queuedir(), jobid);
      perror(error);
    }
    return 1;
  }

  if(chdir(jobid)==-1) {
    perror(jobid);
    return 1;
  }

  return 0;
}

static int create_std_files(int* fd_stdout, int* fd_stderr) {
  *fd_stdout = -1;
  *fd_stderr = -1;

  if((*fd_stdout = open("stdout", O_WRONLY|O_CREAT, FILE_MODE)) == -1) {
    perror("Cannot create stdout");
    return 1;
  }

  if((*fd_stderr = open("stderr", O_WRONLY|O_CREAT, FILE_MODE)) == -1) {
    close(*fd_stdout);
    *fd_stdout = -1;
    perror("Cannot create stderr");
    return 1;
  }

  return 0;
}

static int create_control_socket(unsigned int* fd) {
  struct sockaddr_un local, remote;
  int len;

  if((*fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
    perror("control-socket (socket)");
    return 1;
  }

  local.sun_family = AF_UNIX;
  strcpy(local.sun_path, "control");
  unlink(local.sun_path);
  len = strlen(local.sun_path) + sizeof(local.sun_family);
  if(bind(*fd, (struct sockaddr *)&local, len) == -1 ) {
    perror("control-socket (bind)");
    close(*fd);
    *fd = -1;
    return 1;
  }

  if(listen(*fd, 5) == -1) {
    perror("control-socket (listen)");
    close(*fd);
    *fd = -1;
  }

  return 0;
}

static int write_text(int fd, const char* str) {
  return write(fd, str, strlen(str));
}

static void dump_status(int fd, struct job_state_t* job_state) {
  time_t now = time(NULL);

  char buffer[1024];

  if(job_state->status == job_started) {
    snprintf(buffer, 1024, "pid: %d\n", job_state->pid);
    write_text(fd, buffer);
  }

  snprintf(buffer, 1024, "duration: %ld\n", now - job_state->started_time);
  write_text(fd, buffer);

  switch(job_state->status) {
    case job_none: 
      write_text(fd, "status: none\n");
      break;
    case job_started: 
      write_text(fd, "status: running\n");
      break;
    case job_done: 
      write_text(fd, "status: done\n");
      break;
    default:
      write_text(fd, "status: unknown\n");
  }
}

#define buffer_max 1024
static int buffer_len = 0;
static char buffer[buffer_max];
static int handle_talk(int fd, struct job_state_t* job_state) {
  ssize_t s = read(fd, buffer, buffer_max-buffer_len);
  if(s == 0)
    return;

  if(s == -1) {
    perror("read");
    return -1;
  }

  buffer_len += s;
  buffer[1023] = 0;

  if(strncmp("STATUS\n", buffer, 7) == 0) {
    dump_status(fd, job_state);
    return -1;
  }
  else {
    int i;
    for(i=0; i < buffer_len; i++)
      if(buffer[i] == '\n')
        return -1;
  }
  return 0;
}

static void write_status(struct job_state_t* job_state) {
  int fd = open("status", O_WRONLY|O_CREAT, FILE_MODE);
  if(fd == -1) {
    char error[512];
    snprintf(error, 512, "Cannot create 'status' for writing");
    perror(error);
  }

  dump_status(fd, job_state);

  close(fd);
}

static void child(const char* cmd) {
  //system(cmd);
  char* args[4];
  args[0] = SHELL_BIN;
  args[1] = "-c";
  args[2] = (char*)cmd;
  args[3] = 0;

  execv(SHELL_BIN, args);
}

static int logical_stamp = 0;
static int stamping(char* buf, int len) {
  snprintf(buf, len, "%d: ", logical_stamp++);
}

static void flush(int fd, char* buf, int len, int* buf_len) {
  if(*buf_len == 0)
    return;

  int i;
  int start = 0;
  for(i = 0; i < len; i++) {
    if(buf[i] == '\n') {
      char stamp[32];
      stamping(stamp, 32);
      if(stamp[0])
        write(fd, stamp, strlen(stamp));
      write(fd, buf+start, i - start + 1);
      start = i+1;
    }
  }

  if(start == 0) {
    write(fd, buf, len);
    start = len;
  }
  else {
    memmove(buf, buf+start, len-start);
  }
  *buf_len =- start;
}

static int handle_input(int infd, char* buf, int* buf_len, int buf_size, int outfd) {
  ssize_t s = read(infd, buf+(*buf_len), buf_size);
  if(s < 0) {
    perror("read");
    return 0;
  }
  
  int i;
  for(i = (*buf_len)+s-1; i > 0; i--) {
    if(buf[i] == '\n') {
      flush(outfd, buf, (*buf_len) + i, buf_len);
      break;
    }
  }

  *buf_len += s;
}

static int parent(int pid, int pipe_stdout, int pipe_stderr) {
  char stdout_buf[STDBUF_SIZE]; int stdout_buflen = 0; stdout_buf[0] = 0;
  char stderr_buf[STDBUF_SIZE]; int stderr_buflen = 0; stderr_buf[1] = 0;
  int fd_control;
  int fd_stdout, fd_stderr; // for the 'stdout' and 'stderr' files
  int fd_client = -1;
  int rc = 1;
  
  struct job_state_t job_state;

  job_state.started_time = time(NULL);
  job_state.pid = pid;
  job_state.status = job_started;

  if(create_std_files(&fd_stdout, &fd_stderr) == 0) {
    if(create_control_socket(&fd_control) == 0) {
      char pid[10];
      snprintf(pid, 10, "%d", job_state.pid);
      create_file_content("pid", pid); 
      int status;

      do {
        switch(waitpid(job_state.pid, &status, WNOHANG)) {
          case -1: // ErrorS
            perror("waitpid");
            job_state.status = job_done;
            break;

          case 0: { // Nothing yet
            struct timeval timeout;
            timeout.tv_sec = 0;
            timeout.tv_usec = 100000;

            int nfds = fd_control;

            fd_set rfds;
            FD_ZERO(&rfds);
            FD_SET(fd_control, &rfds);
            if(fd_client != -1) {
              FD_SET(fd_client, &rfds);
              if(fd_client > nfds)
                nfds = fd_client;
            }
            FD_SET(pipe_stdout, &rfds);
            if(pipe_stdout > nfds)
              nfds = pipe_stdout;
            FD_SET(pipe_stderr, &rfds);
            if(pipe_stderr > nfds)
              nfds = pipe_stderr;

            nfds++;

            if(select(nfds, &rfds, NULL, NULL, &timeout)) {
              if(FD_ISSET(fd_control, &rfds)) {
                struct sockaddr_un remote;
                int len = sizeof(struct sockaddr_un);
                fd_client = accept(fd_control, (struct sockaddr*)&remote, &len);
              }
              if(fd_client != -1 && FD_ISSET(fd_client, &rfds)) {
                if(handle_talk(fd_client, &job_state) == -1) {
                  close(fd_client);
                  fd_client = -1;
                }
              }
              if(FD_ISSET(pipe_stdout, &rfds)) {
                handle_input(pipe_stdout, stdout_buf, &stdout_buflen, STDBUF_SIZE, fd_stdout); 
              }
              if(FD_ISSET(pipe_stderr, &rfds)) {
                handle_input(pipe_stderr, stderr_buf, &stderr_buflen, STDBUF_SIZE, fd_stderr); 
              } 
            }
          }
          break;
        default: 
          if(WIFEXITED(status)) {
            job_state.status = job_done;
            char txt[10];
            snprintf(txt, 10, "%d", WEXITSTATUS(status));
            create_file_content("return_code", txt); 
            rc = 0;
          }
          else if(WIFSIGNALED(status)) {
            job_state.status = job_done;
            char txt[10];
            snprintf(txt, 10, "%d", WTERMSIG(status)+128);
            create_file_content("return_code", txt); 
            rc = 0;
          }
        }
      } while(job_state.status == job_started);

      flush(pipe_stdout, stdout_buf, stdout_buflen, &stdout_buflen);
      flush(pipe_stderr, stderr_buf, stderr_buflen, &stderr_buflen);

      close(pipe_stdout);
      close(pipe_stderr);
      unlink("control");
      unlink("pid");

      write_status(&job_state);
    }

    close(fd_control);
  }
  close(fd_stderr);
  close(fd_stdout);
}

static int daemonize() {
  pid_t pid, sid;

  if ( getppid() == 1 ) return;

  pid = fork();

  if(pid < 0) 
    exit(EXIT_FAILURE);
  if(pid > 0) 
    exit(EXIT_SUCCESS);

  umask(0);
  sid = setsid();

  if(sid < 0) 
    exit(EXIT_FAILURE);

  int fd = open("/dev/null", O_RDWR);

  if(fd == -1) 
    exit(EXIT_FAILURE);
  
  dup2(fd, 0);
  dup2(fd, 1);
  dup2(fd, 2);

  return 0;
}

int cmd_execute(const char* jobid, const char* cmd, int wait) {
  int fd_pipe_stdout[2]; 
  int fd_pipe_stderr[2]; 
  int rc = 1;

  if(!wait) 
    daemonize();

  if(prepare_new_job(jobid) == 0) {
    create_file_content("command", cmd);

    if(pipe(fd_pipe_stdout) == 0 && pipe(fd_pipe_stderr) == 0) {
      int pid = fork();

      switch(pid) {
        case -1: // failure
          perror("fork()");
          return 1;
          break;
        case 0: // Child
          close(0);
          close(fd_pipe_stdout[0]);
          close(fd_pipe_stderr[0]);

          dup2(fd_pipe_stdout[1], 1);
          dup2(fd_pipe_stderr[1], 2);

          child(cmd);

          break;
        default: { // parent
          close(fd_pipe_stdout[1]);
          close(fd_pipe_stderr[1]);

          rc = parent(pid, fd_pipe_stdout[0], fd_pipe_stderr[0]);
        }
      }
    }
    else {
      perror("pipe");
    }
  }

  return rc;
}

// vim: ts=2:sw=2:et:ai:tw=0
