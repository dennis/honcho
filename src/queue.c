#include <stdlib.h>

#include "queue.h"
#include "config.h"

const char* get_queuedir() {
  char* dir = getenv("HONCHO_QUEUE_DIR");
  if(dir)
    return dir;
  else
    return QUEUE_DIR;
}

// vim: ts=2:sw=2:et:ai:tw=0
