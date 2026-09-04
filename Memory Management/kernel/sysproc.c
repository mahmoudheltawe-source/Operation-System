#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return wait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;

  argint(0, &n);
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(killed(myproc())){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

// sys_flip_display: zero-copy page flip.
//
// Syscall argument 0: user virtual address of a page-aligned buffer
// that is exactly GPU_FB_PAGES (300) * PGSIZE bytes (i.e. 640x480x4 =
// 1,228,800 bytes).  The buffer must already be fully mapped in the
// calling process's address space.
//
static int
display_user_buffer_valid(pagetable_t pagetable, uint64 va)
{
  uint64 len = (uint64)GPU_FB_PAGES * PGSIZE;

  if((va % PGSIZE) != 0)
    return 0;
  if(va > TRAPFRAME - len)
    return 0;

  for(uint64 a = va; a < va + len; a += PGSIZE){
    pte_t *pte = walk(pagetable, a, 0);
    if(pte == 0)
      return 0;
    if((*pte & PTE_V) == 0)
      return 0;
    if((*pte & PTE_U) == 0)
      return 0;
    if(PTE_FLAGS(*pte) == PTE_V)
      return 0;
  }

  return 1;
}

uint64
sys_flip_display(void)
{
  uint64 buf;
  struct proc *p = myproc();

  argaddr(0, &buf);

  if(!display_user_buffer_valid(p->pagetable, buf))
    return -1;

  if(virtio_gpu_flip(p, buf) < 0)
    return -1;

  return 0;
}

static int
display_region_free(pagetable_t pagetable, uint64 va)
{
  uint64 len = (uint64)GPU_FB_PAGES * PGSIZE;

  if((va % PGSIZE) != 0)
    return 0;
  if(va > TRAPFRAME - len)
    return 0;

  for(uint64 a = va; a < va + len; a += PGSIZE){
    pte_t *pte = walk(pagetable, a, 0);
    if(pte != 0 && (*pte & PTE_V))
      return 0;
  }

  return 1;
}

static uint64
display_pick_addr(struct proc *p)
{
  uint64 len = (uint64)GPU_FB_PAGES * PGSIZE;
  uint64 va = PGROUNDUP(p->sz);

  if(va > TRAPFRAME - len)
    return 0;

  for(; va <= TRAPFRAME - len; va += PGSIZE)
    if(display_region_free(p->pagetable, va))
      return va;

  return 0;
}

// sys_map_display: map the GPU's kernel framebuffer pages (fb[]) directly
// into the calling process's address space with PTE_U|PTE_R|PTE_W.
//
// Syscall argument 0: desired user virtual address (must be page-aligned).
//   Pass 0 to let the kernel auto-select the next available VA above p->sz.
//
// Returns the mapped virtual address on success, (uint64)-1 on failure.
uint64
sys_map_display(void)
{
  uint64 addr;
  struct proc *p = myproc();

  argaddr(0, &addr);

  if(addr == 0){
    if(p->display_mapped)
      return p->display_va;
    addr = display_pick_addr(p);
    if(addr == 0)
      return -1;
  } else if(p->display_mapped){
    if(addr == p->display_va)
      return p->display_va;
    return -1;
  } else if(!display_region_free(p->pagetable, addr)){
    return -1;
  }

  if(virtio_gpu_map_display(p->pagetable, addr) < 0)
    return -1;

  p->display_va = addr;
  p->display_mapped = 1;
  virtio_gpu_restore_kernel();

  return addr;
}
