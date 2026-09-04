#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(void)
{
  lcg_srand(1);
  printf("%d\n", lcg_rand()); // first value should be 1015568748
  printf("%d\n", lcg_rand());
  exit(0);
}
