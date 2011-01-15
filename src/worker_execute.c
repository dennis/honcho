#include <limits.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>

#include "worker_execute.h"
#include "queue.h"
#include "config.h"

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
	if(chdir(get_queuedir())==-1) {
		perror(get_queuedir());
		return 1;
	}

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

static int create_socket(int* fd) {
	if(mkfifo("control", FILE_MODE) != 0) {
		perror("Cannot create control-socket");
		return 1;
	}

	return 0;
}

int worker_execute(const char* jobid, const char* cmd) {
	char cwd[PATH_MAX];
	int fd_stdout, fd_stderr;
	int fd_control;
	int rc = 1;

	if(getcwd(cwd, PATH_MAX)) {
		if(prepare_new_job(jobid) == 0) {
			if(create_std_files(&fd_stdout, &fd_stderr) == 0) {
				if(create_socket(&fd_control) == 0) {
					pid_t jobpid = fork();

					create_file_content("command", cmd);

					switch(jobpid) {
					case 0:	// Child
						close(0);

						dup2(fd_stdout, 1);
						dup2(fd_stderr, 2);

						close(fd_stdout);
						close(fd_stderr);

						//system(cmd);
						char* args[4];
						args[0] = SHELL_BIN;
						args[1] = "-c";
						args[2] = cmd;
						args[3] = 0;

						execv(SHELL_BIN, args);

						break;
					case -1: // failure
						perror("Error launching job");
						return 1;
						break;
					default: { // parent
							{
								char pid[10];
								snprintf(pid, 10, "%d", jobpid);
								create_file_content("pid", pid); 
							}

							int status;

							do {
								if(waitpid(jobpid, &status, 0) == -1) { //HUSK WNOHANG
									perror("waitpid");
								}

								if(WIFEXITED(status)) {
									char rc[10];
									snprintf(rc, 10, "%d", WEXITSTATUS(status));
									create_file_content("return_code", rc); 
								}
								else if(WIFSIGNALED(status)) {
									char rc[10];
									snprintf(rc, 10, "%d", WTERMSIG(status)+128);
									create_file_content("return_code", rc); 
								}
							} while(!WIFEXITED(status) && !WIFSIGNALED(status));

							// cleanup
							unlink("control");
							unlink("pid");
						}
					}

					close(fd_control);
				}

				close(fd_stderr);
				close(fd_stdout);
			}
		}
		chdir(cwd);
	}
	else {
		perror("Cannot get CWD");
	}

	return 1;
}

