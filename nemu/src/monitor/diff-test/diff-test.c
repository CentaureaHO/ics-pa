#include "monitor/monitor.h"
#include "nemu.h"
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#else
#include <signal.h>
#include <sys/prctl.h>
#include <unistd.h>
#endif

#include "protocol.h"

bool gdb_connect_qemu(void);
bool gdb_memcpy_to_qemu(uint32_t, void*, int);
bool gdb_getregs(union gdb_regs*);
bool gdb_setregs(union gdb_regs*);
bool gdb_si(void);
void gdb_exit(void);

static bool is_skip_qemu;
static bool is_skip_nemu;

void diff_test_skip_qemu() { is_skip_qemu = true; }
void diff_test_skip_nemu() { is_skip_nemu = true; }

#define regcpy_from_nemu(regs) \
    do                         \
    {                          \
        regs.eax = cpu.eax;    \
        regs.ecx = cpu.ecx;    \
        regs.edx = cpu.edx;    \
        regs.ebx = cpu.ebx;    \
        regs.esp = cpu.esp;    \
        regs.ebp = cpu.ebp;    \
        regs.esi = cpu.esi;    \
        regs.edi = cpu.edi;    \
        regs.eip = cpu.eip;    \
    } while (0)

static uint8_t mbr[] = {
    // start16:
    0xfa,  // cli
    0x31,
    0xc0,  // xorw   %ax,%ax
    0x8e,
    0xd8,  // movw   %ax,%ds
    0x8e,
    0xc0,  // movw   %ax,%es
    0x8e,
    0xd0,  // movw   %ax,%ss
    0x0f,
    0x01,
    0x16,
    0x44,
    0x7c,  // lgdt   gdtdesc
    0x0f,
    0x20,
    0xc0,  // movl   %cr0,%eax
    0x66,
    0x83,
    0xc8,
    0x01,  // orl    $CR0_PE,%eax
    0x0f,
    0x22,
    0xc0,  // movl   %eax,%cr0
    0xea,
    0x1d,
    0x7c,
    0x08,
    0x00,  // ljmp   $GDT_ENTRY(1),$start32

    // start32:
    0x66,
    0xb8,
    0x10,
    0x00,  // movw   $0x10,%ax
    0x8e,
    0xd8,  // movw   %ax, %ds
    0x8e,
    0xc0,  // movw   %ax, %es
    0x8e,
    0xd0,  // movw   %ax, %ss
    0xeb,
    0xfe,  // jmp    7c27
    0x8d,
    0x76,
    0x00,  // lea    0x0(%esi),%esi

    // GDT
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0x00,
    0xff,
    0xff,
    0x00,
    0x00,
    0x00,
    0x9a,
    0xcf,
    0x00,
    0xff,
    0xff,
    0x00,
    0x00,
    0x00,
    0x92,
    0xcf,
    0x00,

    // GDT descriptor
    0x17,
    0x00,
    0x2c,
    0x7c,
    0x00,
    0x00};

#ifdef _WIN32
static PROCESS_INFORMATION pi;
static void qemu_exit() {
    TerminateProcess(pi.hProcess, 0);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
}
#endif

void init_difftest(void)
{
#ifdef _WIN32
    STARTUPINFO si = {0};
    si.cb = sizeof(si);

    char cmd[] = "qemu-system-i386 -S -s -nographic";
    if (!CreateProcess(NULL, cmd, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        perror("CreateProcess");
        panic("CreateProcess failed");
    }
#else
    int ppid_before_fork = getpid();
    int pid              = fork();
    if (pid == -1) {
        perror("fork");
        panic("fork error");
    }
    else if (pid == 0)
    {
        // child

        // install a parent death signal in the chlid
        int r = prctl(PR_SET_PDEATHSIG, SIGTERM);
        if (r == -1) {
            perror("prctl error");
            panic("prctl");
        }

        if (getppid() != ppid_before_fork) {
            panic("parent has died!");
        }

        close(STDIN_FILENO);
        execlp("qemu-system-i386", "qemu-system-i386", "-S", "-s", "-nographic", NULL);
        perror("exec");
        panic("exec error");
    }
#endif

    gdb_connect_qemu();
    Log("Connect to QEMU successfully");

#ifdef _WIN32
    atexit(qemu_exit);
#endif
    atexit(gdb_exit);

    bool ok = gdb_memcpy_to_qemu(0x7c00, mbr, sizeof(mbr));
    assert(ok == 1);

    union gdb_regs r;
    gdb_getregs(&r);

    r.eip = 0x7c00;
    r.cs  = 0x0000;
    ok    = gdb_setregs(&r);
    assert(ok == 1);

    int i;
    for (i = 0; i < 20; i++) {
        gdb_si();
    }
}

void init_qemu_reg()
{
    union gdb_regs r;
    gdb_getregs(&r);
    regcpy_from_nemu(r);
    bool ok = gdb_setregs(&r);
    assert(ok == 1);
}

#define TEST_FLAG_REGS \
    T(CF, 0)           \
    T(ZF, 6)           \
    T(SF, 7)           \
    T(OF, 11)

void difftest_step(uint32_t eip)
{
    static uint32_t eflags_mask = 0;
    static bool     init_mask   = false;
    if (!init_mask) {
#define X(name, shift)                                  \
    static const uint32_t name##_mask = (0x1 << shift); \
    (void)name##_mask;
        X86_FLAGS
#undef X

#define T(name, shift) eflags_mask |= name##_mask;
        TEST_FLAG_REGS
#undef T

        init_mask = true;
    }

    union gdb_regs r;
    bool           diff = false;

    if (is_skip_nemu) {
        is_skip_nemu = false;
        return;
    }

    if (is_skip_qemu) {
        // to skip the checking of an instruction, just copy the reg state to qemu
        gdb_getregs(&r);
        regcpy_from_nemu(r);
        gdb_setregs(&r);
        is_skip_qemu = false;
        return;
    }

    gdb_si();
    gdb_getregs(&r);

    // TODO: Check the registers state with QEMU.
    // Set `diff` as `true` if they are not the same.

    if (diff) {
        Log("Diff test failed at eip = 0x%x, exec_cnt = %d", eip, cpu.exec_cnt);
        nemu_state = NEMU_END;
    }
}
