#ifndef UTIL_H_
#define UTIL_H_

int put_file(const char* filename, const char *data);
int cat_file(int fd, const char* file);
const char* get_queuedir(const char*);
int connect_to_control_socket();

#endif
// vim: ts=2:sw=2:et:ai:tw=0
