#ifndef __oneOS__LibC__SYSCALLS_H
#define __oneOS__LibC__SYSCALLS_H

#include <sys/_structs.h>
#include <sys/types.h>

extern int errno;

int open(const char* pathname, int flags);
int close(int fd);
int read(int fd, char* buf, size_t count);
int write(int fd, const void* buf, size_t count);
void exit(int ret_code);
int fork();
int wait(int pid);
int execve(const char* path, char** argv, char** env);
int chdir(char* path);
int mkdir(char* path);
int rmdir(char* path);
int unlink(char* path);
int creat(char* path, uint32_t mode);
int lseek(int fd, uint32_t off, int whence);
pid_t getpid();
int kill(pid_t pid, int signo);

int raise(int signo);
int sigaction(int signo, void* callback);
int mmap(mmap_params_t* params);
int socket(int domain, int type, int protocol);
int bind(int sockfd, const char* name, int len);
int connect(int sockfd, const char* name, int len);
int getdents(int fd, char* buf, int len);
int ioctl(int fd, uint32_t cmd, uint32_t arg);
int setpgid(pid_t cmd, pid_t arg);
pid_t getpgid(pid_t arg);

int system_pthread_create(thread_create_params_t* params);

int shared_buffer_create(uint8_t** buffer, size_t size);
int shared_buffer_get(int id, uint8_t** buffer);
int shared_buffer_free(int id);

#endif /* __oneOS__LibC__SYSCALLS_H */