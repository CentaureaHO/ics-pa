/* connector for fork */

#include <reent.h>
#include <_syslist.h>

int fork()
{
#ifdef REENTRANT_SYSCALLS_PROVIDED
    return _fork_r(_REENT);
#else
    return _fork();
#endif
}
