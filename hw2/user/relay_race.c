#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define TEAMS 3
#define RUNNERS_PER_TEAM 5
#define TARGET_SCORE 30

static void
run_runner(int lock_id, int team_id)
{
  setgid(team_id);

  for(;;){
    if(israeli_acquire(lock_id) < 0){
      printf("Runner %d (Team %d) failed to acquire baton\n", getpid(), team_id);
      exit(1);
    }

    if(race_winner(TARGET_SCORE) >= 0){
      israeli_release(lock_id);
      exit(0);
    }

    int score = race_inc(team_id);
    printf("Runner %d (Team %d) acquired the baton\n", getpid(), team_id);
    printf("Team %d score = %d\n", team_id, score);

    if(israeli_release(lock_id) < 0){
      printf("Runner %d (Team %d) failed to release baton\n", getpid(), team_id);
      exit(1);
    }

    if(score >= TARGET_SCORE){
      exit(0);
    }

    sleep(1);
  }
}

static void
run_race(int favoritism)
{
  int total_runners = TEAMS * RUNNERS_PER_TEAM;
  int lock_id;

  printf("\nRelay race with favoritism = %d\n", favoritism);

  if(race_reset(TEAMS) < 0){
    printf("race_reset failed\n");
    exit(1);
  }

  lcg_srand(getpid() + favoritism + uptime());
  lock_id = israeli_create(favoritism);
  if(lock_id < 0){
    printf("israeli_create failed\n");
    exit(1);
  }

  for(int team = 0; team < TEAMS; team++){
    for(int runner = 0; runner < RUNNERS_PER_TEAM; runner++){
      int pid = fork();
      if(pid < 0){
        printf("fork failed\n");
        exit(1);
      }

      if(pid == 0){
        run_runner(lock_id, team);
      }
    }
  }

  for(int i = 0; i < total_runners; i++){
    wait(0);
  }

  int winner = race_winner(TARGET_SCORE);
  printf("Winner: Team %d\n", winner);
  for(int team = 0; team < TEAMS; team++){
    printf("Final score Team %d = %d\n", team, race_get(team));
  }

  if(israeli_destroy(lock_id) < 0){
    printf("israeli_destroy failed\n");
    exit(1);
  }
}

int
main(void)
{
  run_race(0);
  run_race(50);
  run_race(100);
  exit(0);
}
