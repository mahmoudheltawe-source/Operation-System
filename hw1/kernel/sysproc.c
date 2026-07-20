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
  return 0; // not reached
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
  if (growproc(n) < 0)
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
  while (ticks - ticks0 < n)
  {
    if (killed(myproc()))
    {
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

uint64
sys_memsize(void)
{
  struct proc *p = myproc();
  return p->sz;
}

//---------task 3
/*
extern struct proc proc[NPROC];

uint64
sys_co_yield(void)
{
 int pid, value;
  argint(0, &pid);
  argint(1, &value);

  return co_yield(pid, value);


//  struct proc *cur = myproc();
//   struct proc *target = 0;
//   struct proc *p;


//   if (pid <= 0 || value <= 0 || pid == cur->pid)
//     return -1;

//   yield_proc( pid , value );
//   // Find target and keep target->lock held.
//   for (p = proc; p < &proc[NPROC]; p++)
//   {
//     acquire(&p->lock);
//     if (p->state != UNUSED && p->pid == pid)
//     {
//       if (p->killed)
//       {
//         release(&p->lock);
//         return -1;
//       }
//       target = p;
//       break;
//     }

//     if (!holding(&p->lock))
//       panic("sys_co_yield: lock not held after acquire");
//     release(&p->lock);
//   }

//   if (target == 0)
//     return -1;

//   acquire(&cur->lock);

//   if (cur->killed || target->killed)
//   {
//     if (!holding(&cur->lock))
//       panic("cur lock not held before release [killed cur]");
//     if (!holding(&target->lock))
//       panic("target lock not held before release [killed target]");
//     release(&cur->lock);
//     release(&target->lock);
//     return -1;
//   }

//   // Rendezvous: target is already sleeping and waiting for me.
//   if (target->state == SLEEPING && target->chan == (void *)cur)
//   {
//     // Value for the target's co_yield return.
//     target->trapframe->a0 = value;

//     // Current process goes to sleep waiting for the opposite yield later.
//     cur->trapframe->a0 = -1;
//     cur->chan = (void *)target;
//     cur->state = SLEEPING;

//     // Wake target and hand off CPU directly.
//     target->chan = 0;
//     target->state = RUNNING;

//     // co_handoff(cur, target);
//     //======================try=============================
//     mycpu()->proc = target;
//     release(&cur->lock);

//     swtch(&cur->context, &target->context);
//     //======================try=============================
//     // We resume here later, with cur->lock held again.
//     cur->chan = 0;

//     if (cur->killed)
//     {
//       if (!holding(&cur->lock))
//         panic("case1: cur lock not held before release [killed cur]");
//       release(&cur->lock);
//       return -1;
//     }

//     int ret = cur->trapframe->a0;
//     if (!holding(&cur->lock))
//       panic("case1: cur lock not held before release[returning ret]");
//     release(&cur->lock);
//     return ret;
//   }

//   // Target is not ready yet: sleep until it yields to me later.
//   if (!holding(&target->lock))
//     panic("case2: target lock not held before release [not ready yet]");
//   release(&target->lock);

//   cur->trapframe->a0 = -1;
//   cur->chan = (void *)target;
//   cur->state = SLEEPING;

//   sched(); // returns with cur->lock still held

//   cur->chan = 0;

//   if (cur->killed)
//   {
//     if (!holding(&cur->lock))
//       panic("case2: cur lock not held before release [killed cur]");
//     release(&cur->lock);
//     return -1;
//   }

//   int ret = cur->trapframe->a0;
//   if (!holding(&cur->lock))
//     panic("case2: cur lock not held before release[returning ret]");
//   release(&cur->lock);
//   return ret;
}
*/
static int
invalid_co_yield_args(struct proc *cur, int target_pid, int value)
{
  return (target_pid <= 0 || target_pid == cur->pid || value <= 0 || target_pid >= NPROC);
}

static void
cancel_sleep_setup(struct proc *cur)
{
  cur->chan = 0;
  cur->state = RUNNING;
}

static struct proc *
find_target_proc(struct proc *cur, int target_pid)
{
  struct proc *p;

  extern struct proc proc[NPROC];
  for (p = proc; p < &proc[NPROC]; p++)
  {
    if (p == cur)
      continue;

    acquire(&p->lock);
    if (p->pid == target_pid)
      return p; // return the target locked
    release(&p->lock);
  }

  return 0;
}

static int
target_waiting_for_me(struct proc *target, struct proc *cur)
{
  return (target->state == SLEEPING &&
          target->chan == (void *)(uint64)cur->pid);
}
/**
 * Helper function to wake up the target process with the given value. Assumes target->lock is held and does not release it.
 */
static void
wake_target_with_value(struct proc *target, int value)
{
  target->trapframe->a0 = value;
  target->state = RUNNING;
}

uint64
sys_co_yield(void)
{
  int target_pid, value;
  struct proc *cur = myproc();
  struct proc *target;

  argint(0, &target_pid);
  argint(1, &value);

  if (invalid_co_yield_args(cur, target_pid, value))
    return -1;

  // make curr ready to switch
  acquire(&cur->lock);
  cur->trapframe->a0 = -1;
  cur->chan = (void *)(uint64)target_pid;
  cur->state = SLEEPING;

  // get target
  target = find_target_proc(cur, target_pid);
  if (target == 0)
  {
    cancel_sleep_setup(cur);
    release(&cur->lock);
    return -1;
  }

  if (target->killed)
  {
    cancel_sleep_setup(cur);
    release(&target->lock);
    release(&cur->lock);
    return -1;
  }

  // CASE 1 : TARGET WAITING FOR ME ALREADY
  if (target_waiting_for_me(target, cur))
  {
    wake_target_with_value(target, value);
    mycpu()->proc = target;
    co_handoff(cur, target);
  }
  else
  {

    // CASE 2 : TARGET NOT READY YET, SLEEP UNTIL TARGET YIELDS TO ME LATER
    release(&target->lock);
    sched(); // returns with cur->lock still held
  }

  cur->chan = 0;

  if (cur->killed)
  {
    release(&cur->lock);
    return -1;
  }

  int ret = cur->trapframe->a0;
  release(&cur->lock);
  return ret;
}