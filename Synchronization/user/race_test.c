#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(void)
{
  printf("race_reset(3) = %d\n", race_reset(3));

  printf("team 0 inc = %d\n", race_inc(0));
  printf("team 0 inc = %d\n", race_inc(0));
  printf("team 1 inc = %d\n", race_inc(1));

  printf("team 0 get = %d\n", race_get(0));
  printf("team 1 get = %d\n", race_get(1));
  printf("team 2 get = %d\n", race_get(2));
  printf("team 3 get = %d\n", race_get(3));

  printf("winner target 2 = %d\n", race_winner(2));
  printf("winner target 3 = %d\n", race_winner(3));

  exit(0);
}
