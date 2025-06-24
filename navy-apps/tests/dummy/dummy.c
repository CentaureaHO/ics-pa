#include <stdint.h>

#define SYS_none 0
extern int syscall(int, intptr_t, intptr_t, intptr_t);

int main()
{
    // this system call will trap into OS but do nothing
    int r = syscall(SYS_none, 0, 0, 0);
    return (r == 1 ? 0 : 1);
}
