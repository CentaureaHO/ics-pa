/* internal use only -- mapping of "system calls" for libraries that lose
   and only provide C names, so that we end up in violation of ANSI */
#ifndef __SYSLIST_H
#define __SYSLIST_H

/* Forward declarations for all system calls */
int _fork(void);
int _wait(int *status);
int _open(const char *file, int flags, int mode);
int _close(int fd);
int _lseek(int fd, int pos, int whence);
int _read(int fd, void *buf, int cnt);
int _write(int fd, const void *buf, int cnt);
int _fstat(int fd, void *pstat);
int _stat(const char *file, void *pstat);
int _link(const char *existing, const char *new);
int _unlink(const char *name);
void *_sbrk(size_t incr);
int _kill(int pid, int sig);
int _getpid(void);
int _gettimeofday(void *ptimeval, void *ptimezone);
int _times(void *ptms);
int _fcntl(int fd, int cmd, int arg);
int _execve(const char *name, char * const argv[], char * const env[]);
/* Memory management functions */
void _free_r(struct _reent *reent_ptr, void *ptr);
void _cleanup_r(struct _reent *reent_ptr);

#ifdef MISSING_SYSCALL_NAMES
#define _close close
#define _fcntl fcntl
#define _fork fork
#define _fstat fstat
#define _getpid getpid
#define _gettimeofday gettimeofday
#define _kill kill
#define _link link
#define _lseek lseek
#define _open open
#define _read read
#define _sbrk sbrk
#define _stat stat
#define _times times
#define _unlink unlink
#define _wait wait
#define _write write
/* functions not yet sysfaked */
#define _execve execve
#define _opendir opendir
#define _readdir readdir
#define _closedir closedir
#endif
#endif
