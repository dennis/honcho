#include <stdlib.h>
#include <limits.h>

#include "queue.h"
#include "config.h"

const char* get_queuedir(const char* name) {
  char* env = getenv("HONCHO_QUEUE_DIR");
  char queuedir[PATH_MAX];

  queuedir[0] = 0;

  if(env)
    strcat(queuedir, env);
  else
    strcat(queuedir, QUEUE_DIR);

  int i = strlen(queuedir)-1;
  while(queuedir[i] == '/' && i > 0)
    i--;

  queuedir[i+1] = 0;

  strcat(queuedir, "/");
  strcat(queuedir, name);

  return queuedir;
}

// vim: ts=2:sw=2:et:ai:tw=0
