#include <string.h>

#include "worker_execute.h"

int main(int argc, char *argv[]) {
	if(argc > 2) {
		if(strcmp(argv[1], "execute") == 0) {
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
		else {
			puts("Invalid syntax");
		}
	}
	else {
		puts("honcho execute <ID> <cmd> [args..]");
	}

	return 1;
}

// vim: ts=2:sw=2:et:ai:tw=0
