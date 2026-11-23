#include <unistd.h>
#include <proc.h>
#include <syscall.h>
#include <trap.h>
#include <stdio.h>
#include <pmm.h>
#include <assert.h>
#include <vmm.h>
#include <error.h>

static int sys_exit(uint64_t arg[]);
static int sys_fork(uint64_t arg[]);
static int sys_wait(uint64_t arg[]);
static int sys_exec(uint64_t arg[]);
static int sys_yield(uint64_t arg[]);
static int sys_kill(uint64_t arg[]);
static int sys_getpid(uint64_t arg[]);
static int sys_putc(uint64_t arg[]);
static int sys_pgdir(uint64_t arg[]);

static int (*syscalls[])(uint64_t arg[]) = {
    [SYS_exit]              sys_exit,
    [SYS_fork]              sys_fork,
    [SYS_wait]              sys_wait,
    [SYS_exec]              sys_exec,
    [SYS_yield]             sys_yield,
    [SYS_kill]              sys_kill,
    [SYS_getpid]            sys_getpid,
    [SYS_putc]              sys_putc,
    [SYS_pgdir]             sys_pgdir,
};

#define NUM_SYSCALLS        ((sizeof(syscalls)) / (sizeof(syscalls[0])))

//lab5 function1 2412351
/* Registers carry XLEN bits; the process interfaces below take signed int. */
static bool
syscall_arg_int(uint64_t raw, int *value) {
    int64_t signed_value = (int64_t)raw;
    if (signed_value < (-2147483647LL - 1) || signed_value > 2147483647LL) {
        return 0;
    }
    *value = (int)signed_value;
    return 1;
}

/* User buffers must not fall through user_mem_check's mm == NULL kernel case.
 * This is an entry check; do_wait/do_execve still own the final checked copy
 * and its ordering relative to sleeping, reaping and replacing an mm. */
static bool
syscall_user_range(uintptr_t addr, size_t len, bool write) {
    if (current->mm == NULL || trap_in_kernel(current->tf) ||
        len == 0 || len > (uintptr_t)-1 - addr) {
        return 0;
    }
    return user_mem_check(current->mm, addr, len, write);
}

static int
sys_exit(uint64_t arg[]) {
    int code;
    return syscall_arg_int(arg[0], &code) ? do_exit(code) : -E_INVAL;
}

static int
sys_fork(uint64_t arg[]) {
    struct trapframe *tf = current->tf;
    /* Ordinary fork: copy the already-advanced caller frame, not CLONE_VM. */
    return do_fork(0, tf->gpr.sp, tf);
}

static int
sys_wait(uint64_t arg[]) {
    int pid;
    int *store = (int *)(uintptr_t)arg[1];
    if (!syscall_arg_int(arg[0], &pid) ||
        (store != NULL && !syscall_user_range((uintptr_t)store, sizeof(*store), 1))) {
        return -E_INVAL;
    }
    return do_wait(pid, store);
}

static int
sys_exec(uint64_t arg[]) {
    const char *name = (const char *)(uintptr_t)arg[0];
    size_t len = (size_t)arg[1];
    unsigned char *binary = (unsigned char *)(uintptr_t)arg[2];
    size_t size = (size_t)arg[3];
    /* Zero name length denotes no bytes to read; naming/ELF semantics belong
     * to do_execve. The ELF input must contain a nonempty readable range. */
    if ((len != 0 && !syscall_user_range((uintptr_t)name, len, 0)) ||
        !syscall_user_range((uintptr_t)binary, size, 0)) {
        return -E_INVAL;
    }
    return do_execve(name, len, binary, size);
}

static int
sys_yield(uint64_t arg[]) {
    return do_yield();
}

static int
sys_kill(uint64_t arg[]) {
    int pid;
    return syscall_arg_int(arg[0], &pid) ? do_kill(pid) : -E_INVAL;
}

static int
sys_getpid(uint64_t arg[]) {
    return current->pid;
}

static int
sys_putc(uint64_t arg[]) {
    int c;
    if (!syscall_arg_int(arg[0], &c)) {
        return -E_INVAL;
    }
    cputchar(c);
    return 0;
}

static int
sys_pgdir(uint64_t arg[]) {
    print_pgdir();
    return 0;
}

void
syscall(void) {
    assert(current != NULL && current->tf != NULL);
    struct trapframe *tf = current->tf;
    uintptr_t num = tf->gpr.a0;
    int ret = -E_INVAL;
    /* Check the complete RV64 number before indexing or narrowing it. */
    if (num < NUM_SYSCALLS && syscalls[num] != NULL) {
        uint64_t arg[5] = {
            tf->gpr.a1, tf->gpr.a2, tf->gpr.a3, tf->gpr.a4, tf->gpr.a5,
        };
        ret = syscalls[num](arg);
    }
    /* Backends may edit this frame (notably exec's EPC/SP/status). */
    tf->gpr.a0 = (uintptr_t)(intptr_t)ret;
}
