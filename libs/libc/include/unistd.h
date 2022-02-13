#ifndef _LIBC_UNISTD_H
#define _LIBC_UNISTD_H

#include <fcntl.h>
#include <stddef.h>
#include <sys/cdefs.h>
#include <sys/types.h>

__BEGIN_DECLS

/* tasking */
int fork();
int execve(const char* path, char** argv, char** env);
int execvpe(const char* path, char** argv, char** env);
pid_t getpid();
int setpgid(pid_t cmd, pid_t arg);
pid_t getpgid(pid_t arg);
uint32_t sleep(uint32_t seconds);

/* fs */
int close(int fd);
ssize_t read(int fd, char* buf, size_t count);
ssize_t write(int fd, const void* buf, size_t count);
int dup(int oldfd);
int dup2(int oldfd, int newfd);
int rmdir(const char* path);
int chdir(const char* path);
int unlink(const char* path);
off_t lseek(int fd, off_t off, int whence);

/* identity */
uid_t getuid();
int setuid(uid_t uid);
int setgid(gid_t gid);
int setreuid(uid_t ruid, uid_t euid);
int setregid(gid_t rgid, gid_t egid);
char* getlogin();

/* sched */
int nice(int inc);

/* conf */
#define _CS_GNU_LIBC_VERSION 1
#define _CS_GNU_LIBPTHREAD_VERSION 2
#define _CS_PATH 3
size_t confstr(int name, char* buf, size_t len);

__END_DECLS

#endif // _LIBC_UNISTD_H
