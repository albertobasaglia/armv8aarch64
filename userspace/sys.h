#ifndef SYS_H
#define SYS_H

int sys_open(char* filename);

char sys_read(int fd);

void sys_log(char* message);

void sys_spawn(char* filename);

#endif
