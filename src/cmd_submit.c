#include <uuid/uuid.h>

#include "queue.h"
#include "cmd_submit.h"
#include "utils.h"

static void generate_uuid(char out[37]) {
  // at least 36+1 chars
  uuid_t jobid;
  uuid_generate(jobid);
  uuid_unparse(jobid, out);
}

int cmd_submit(const char* cmd) {
  char jobid[37];
  char filename[45]; // uuid + ".pending" = 8char + null

  generate_uuid(jobid);

  snprintf(filename, 45, "%s.pending", jobid);

  if(put_file(filename, cmd) == 0) {
    puts(jobid);
    return 0;
  }
  else {
    return 1;
  }

}

// vim: ts=2:sw=2:et:ai:tw=0
