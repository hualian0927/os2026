/* Controlled U/S frame-return fixture. No production ELF loader or sys_* replacement. */
#include <pmm.c>
#include <console.h>
#include <trap.h>
#include <intr.h>
#include <dtb.h>
#define USER_CODE 0x400000UL
#define USER_TOP 0x403000UL
static char trap_stack[8192] __attribute__((aligned(4096)));
static struct trapframe *expected_frame;
static unsigned calls;
uintptr_t kernel_resume_sp, final_status, final_sscratch;
extern char user_probe_begin[],user_probe_end[];
extern void enter_user_fixture(struct trapframe *tf),user_fixture_return(void),__alltraps(void);
void __noreturn __panic(const char *file,int line,const char *fmt,...) {
    intr_disable();cprintf("USER_TRAP_FAIL %s:%d ",file,line);
    va_list ap;va_start(ap,fmt);vcprintf(fmt,ap);va_end(ap);cprintf("\n");
    sbi_shutdown();while(1){}
}
static uintptr_t __attribute__((noinline)) work(unsigned n) {
    volatile uintptr_t a[8];uintptr_t sum=0;
    for(unsigned i=0;i<8;i++){a[i]=n+i;sum+=a[i];}return sum;
}
void trap(struct trapframe *tf) {
    assert(tf==expected_frame && ((uintptr_t)tf & 15)==0);
    assert(tf->cause==8 && !(tf->status&SSTATUS_SPP));
    assert(!(tf->status&SSTATUS_SIE) && (tf->status&SSTATUS_SPIE));
    assert(read_csr(sscratch)==0);
    assert(work(calls)==8*calls+28);
    if(tf->gpr.a0==0x5a5) {
        assert(calls<64 && tf->gpr.sp==USER_TOP-16);
        assert(tf->gpr.t0==0x123 && tf->gpr.s0==0x456 && tf->gpr.a1==0x5a5);
        tf->gpr.a0=0xabc;tf->gpr.t0=0xdef;tf->gpr.s0=0x789;
        tf->epc+=4;calls++;
    } else {
        assert(tf->gpr.a0==0xcafe && calls==64 && tf->gpr.sp==USER_TOP);
        tf->epc=(uintptr_t)user_fixture_return;
        tf->status|=SSTATUS_SPP|SSTATUS_SPIE;
        tf->gpr.sp=kernel_resume_sp;
        calls++;
    }
    asm volatile("li t0, 1; li t1, 2; li a0, 3; li a1, 4" ::: "t0","t1","a0","a1");
}
int kern_init(void) {
    extern char edata[],end[],boot_page_table_sv39[];
    memset(edata,0,end-edata);intr_disable();write_csr(sie,0);write_csr(sscratch,0);
    write_csr(stvec,(uintptr_t)__alltraps);cons_init();dtb_init();init_pmm_manager();page_init();
    size_t before=nr_free_pages();
    struct Page *middle=alloc_page(),*leaf=alloc_page(),*code=alloc_page(),*stack=alloc_page();
    assert(middle&&leaf&&code&&stack);
    pde_t *root=(pde_t*)boot_page_table_sv39,*mid=page2kva(middle);
    pte_t *pt=page2kva(leaf);
    memset(mid,0,PGSIZE);memset(pt,0,PGSIZE);memset(page2kva(code),0,PGSIZE);memset(page2kva(stack),0,PGSIZE);
    assert(user_probe_end-user_probe_begin<PGSIZE);
    memcpy(page2kva(code),user_probe_begin,user_probe_end-user_probe_begin);
    /* Test-only explicit Sv39 tables isolate Trap mechanics from framework get_pte policy. */
    root[PDX1(USER_CODE)]=ptd_create(page2ppn(middle));
    mid[PDX0(USER_CODE)]=ptd_create(page2ppn(leaf));
    pt[PTX(USER_CODE)]=pte_create(page2ppn(code),PTE_R|PTE_X|PTE_U|PTE_A);
    pt[PTX(USER_TOP-PGSIZE)]=pte_create(page2ppn(stack),PTE_R|PTE_W|PTE_U|PTE_A|PTE_D);
    flush_tlb();asm volatile("fence.i" ::: "memory");
    expected_frame=(struct trapframe*)(trap_stack+sizeof(trap_stack))-1;
    memset(expected_frame,0,sizeof(*expected_frame));
    expected_frame->status=(read_csr(sstatus)&~(SSTATUS_SPP|SSTATUS_SIE))|SSTATUS_SPIE;
    expected_frame->epc=USER_CODE;expected_frame->gpr.sp=USER_TOP;expected_frame->gpr.s1=64;
    asm volatile("mv %0, gp":"=r"(expected_frame->gpr.gp));
    asm volatile("mv %0, tp":"=r"(expected_frame->gpr.tp));
    enter_user_fixture(expected_frame);
    assert(calls==65 && final_sscratch==0);
    assert((final_status&SSTATUS_SIE) && (final_status&SSTATUS_SPIE) && !(final_status&SSTATUS_SPP));
    root[PDX1(USER_CODE)]=0;flush_tlb();
    free_page(middle);free_page(leaf);free_page(code);free_page(stack);
    assert(nr_free_pages()==before);
    cprintf("USER_TRAP_PASS: actual forkrets/alltraps/restore; 64 U returns, trusted kernel stack, frame edits, S return, page recovery\n");
    sbi_shutdown();while(1){}
}
