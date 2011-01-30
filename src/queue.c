#include <stdlib.h>
#include <limits.h>
#include <string.h>

#include "queue.h"
#include "config.h"

char queuedir[PATH_MAX];

const char* get_queuedir(const char* name) {
  char* env = getenv("HONCHO_DIR");

  queuedir[0] = 0;

  if(env) {
    strcat(queuedir, env);
  }
  else {
    strcat(queuedir, DATA_DIR);
  }
  strcat(queuedir, "/queue");

  int i = strlen(queuedir)-1;
  while(queuedir[i] == '/' && i > 0)
    i--;

  queuedir[i+1] = 0;

  strcat(queuedir, "/");
  strcat(queuedir, name);

  return queuedir;
}

// vim: ts=2:sw=2:et:ai:tw=0
