#include <defs.h>
#include <unistd.h>
#include <syscall.h>

//lab5 function1 2412351: retain the public ABI; supply all five argument slots.
static inline int
syscall(int64_t num, uint64_t arg0, uint64_t arg1, uint64_t arg2,
        uint64_t arg3, uint64_t arg4) {
    register uint64_t r0 asm("a0") = (uint64_t)num;
    register uint64_t r1 asm("a1") = arg0;
    register uint64_t r2 asm("a2") = arg1;
    register uint64_t r3 asm("a3") = arg2;
    register uint64_t r4 asm("a4") = arg3;
    register uint64_t r5 asm("a5") = arg4;
    asm volatile ("ecall"
        : "+r"(r0), "+r"(r1), "+r"(r2), "+r"(r3), "+r"(r4), "+r"(r5)
        :
        : "memory");
    return (int)(int64_t)r0;
}

int
sys_exit(int64_t error_code) {
    return syscall(SYS_exit, (uint64_t)error_code, 0, 0, 0, 0);
}

int
sys_fork(void) {
    return syscall(SYS_fork, 0, 0, 0, 0, 0);
}

int
sys_wait(int64_t pid, int *store) {
    return syscall(SYS_wait, (uint64_t)pid, (uintptr_t)store, 0, 0, 0);
}

int
sys_yield(void) {
    return syscall(SYS_yield, 0, 0, 0, 0, 0);
}

int
sys_kill(int64_t pid) {
    return syscall(SYS_kill, (uint64_t)pid, 0, 0, 0, 0);
}

int
sys_getpid(void) {
    return syscall(SYS_getpid, 0, 0, 0, 0, 0);
}

int
sys_putc(int64_t c) {
    return syscall(SYS_putc, (uint64_t)c, 0, 0, 0, 0);
}

int
sys_pgdir(void) {
    return syscall(SYS_pgdir, 0, 0, 0, 0, 0);
}

