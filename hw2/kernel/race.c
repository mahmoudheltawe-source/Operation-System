#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"

#define MAX_TEAMS 8

static struct spinlock race_lock;
static int scores[MAX_TEAMS];
static int teams_count;

void
raceinit(void)
{
    initlock(&race_lock, "race");
    teams_count = 0;

    for(int i = 0; i < MAX_TEAMS; i++){
        scores[i] = 0;
    }
}

int
race_reset(int teams)
{
    if(teams <= 0 || teams > MAX_TEAMS){
        return -1;
    }

    acquire(&race_lock);
    teams_count = teams;
    for(int i = 0; i < MAX_TEAMS; i++){
        scores[i] = 0;
    }
    release(&race_lock);

    return 0;
}

int
race_inc(int team_id)
{
    int score;

    acquire(&race_lock);
    if(team_id < 0 || team_id >= teams_count){
        release(&race_lock);
        return -1;
    }

    scores[team_id]++;
    score = scores[team_id];
    release(&race_lock);

    return score;
}

int
race_get(int team_id)
{
    int score;

    acquire(&race_lock);
    if(team_id < 0 || team_id >= teams_count){
        release(&race_lock);
        return -1;
    }

    score = scores[team_id];
    release(&race_lock);

    return score;
}

int
race_winner(int target)
{
    if(target <= 0){
        return -1;
    }

    acquire(&race_lock);
    for(int i = 0; i < teams_count; i++){
        if(scores[i] >= target){
            release(&race_lock);
            return i;
        }
    }
    release(&race_lock);

    return -1;
}
