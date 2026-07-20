#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"

#define NISRAELI 15
#define MAX_WAITERS 16

struct israeli_lock {
    struct spinlock lock;
    int active;
    int held;
    int owner_pid;
    int owner_gid;
    int favoritism;

    struct proc *queue[MAX_WAITERS];
    int queuelen;
};

static struct israeli_lock locks[NISRAELI];

void
israeliinit(void)
{
    for(int i = 0; i < NISRAELI; i++){
        initlock(&locks[i].lock, "israeli");
        locks[i].active = 0;
        locks[i].held = 0;
        locks[i].owner_pid = -1;
        locks[i].owner_gid = -1;
        locks[i].favoritism = 0;
        locks[i].queuelen = 0;
    }
}

int
israeli_create(int favoritism)
{
    if(favoritism < 0 || favoritism > 100)
    {
        return -1;
    }
    for(int i = 0; i < NISRAELI; i++)
    {
        acquire(&locks[i].lock);
        if(locks[i].active == 0)
        {
            locks[i].active = 1;
            locks[i].held = 0;
            locks[i].owner_pid = -1;
            locks[i].owner_gid = -1;
            locks[i].favoritism = favoritism;
            locks[i].queuelen = 0;

            release(&locks[i].lock);
            return i;
        }
        release(&locks[i].lock);
    }
    return -1;
}

int
israeli_acquire(int lock_id)
{
    if(lock_id < 0 || lock_id >= NISRAELI)
    {
        return -1;
    }

    struct israeli_lock *lk = &locks[lock_id];
    struct proc *p = myproc();
    acquire(&lk->lock);
    if(lk->active == 0)
    {
        release(&lk->lock);
        return -1;
    }

    if(lk->held == 0 && lk->queuelen == 0)
    {
        lk->held = 1;
        lk->owner_pid = p->pid;
        lk->owner_gid = p->gid;
        release(&lk->lock);
        return 0;
    }

    if(lk->owner_pid == p->pid || lk->queuelen >= MAX_WAITERS)
    {
        release(&lk->lock);
        return -1;
    }

    lk->queue[lk->queuelen] = p;
    lk->queuelen++;

    while(lk->owner_pid != p->pid)
    {
        sleep(lk, &lk->lock);
    }

    release(&lk->lock);
    return 0;
}

static void
remove_waiter(struct israeli_lock *lk, int index)
{
    for(int i = index; i < lk->queuelen - 1; i++)
    {
        lk->queue[i] = lk->queue[i + 1];
    }

    lk->queuelen--;
    lk->queue[lk->queuelen] = 0;
}

static int
select_next_waiter(struct israeli_lock *lk, int releasing_gid)
{
    int same_gid_index = -1;

    for(int i = 0; i < lk->queuelen; i++)
    {
        if(lk->queue[i]->gid == releasing_gid)
        {
            same_gid_index = i;
            break;
        }
    }

    if(same_gid_index != -1 && lcg_rand() % 100 < lk->favoritism)
    {
        return same_gid_index;
    }

    return 0;
}

int
israeli_release(int lock_id)
{
    if(lock_id < 0 || lock_id >= NISRAELI)
    {
        return -1;
    }

    struct israeli_lock *lk = &locks[lock_id];
    struct proc *p = myproc();
    acquire(&lk->lock);
    if(lk->active == 0 || lk->held == 0 || lk->owner_pid != p->pid)
    {
        release(&lk->lock);
        return -1;
    }

    if(lk->queuelen == 0)
    {
        lk->held = 0;
        lk->owner_pid = -1;
        lk->owner_gid = -1;
        release(&lk->lock);
        return 0;
    }

    int next_index = select_next_waiter(lk, p->gid);
    struct proc *next = lk->queue[next_index];
    remove_waiter(lk, next_index);

    lk->held = 1;
    lk->owner_pid = next->pid;
    lk->owner_gid = next->gid;
    wakeup(lk);

    release(&lk->lock);
    return 0;
}

int
israeli_destroy(int lock_id)
{
    if(lock_id < 0 || lock_id >= NISRAELI)
    {
        return -1;
    }
    struct israeli_lock *lk = &locks[lock_id];
    acquire(&lk->lock);
    if(lk->active == 0 || lk->held || lk->queuelen > 0)
    {
        release(&lk->lock);
        return -1;
    }
    lk->active = 0;
    lk->held = 0;
    lk->owner_pid = -1;
    lk->owner_gid = -1;
    lk->favoritism = 0;
    lk->queuelen = 0;
    release(&lk->lock);
    return 0;
}
