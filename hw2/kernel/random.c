#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "defs.h"

static struct spinlock lcg_lock;
static uint lcg_state = 1;

void
lcginit(void)
{
    initlock(&lcg_lock, "lcg");
}

void
lcg_srand(uint seed)
{
    acquire(&lcg_lock);
    lcg_state = seed;
    release(&lcg_lock);
}

uint
lcg_rand(void)
{
    uint x;
    acquire(&lcg_lock);
    //lcg_state = lcg_state * 1664525U + 1013904223U;
    lcg_state = (lcg_state * 1664525 + 1013904223) % 4294967296;
    x = lcg_state;
    release(&lcg_lock);
    return x;
}
