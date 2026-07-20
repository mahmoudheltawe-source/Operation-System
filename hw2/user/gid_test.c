#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int
main(void)
{
  printf("initial gid = %d\n", getgid());
  setgid(7);
  printf("after setgid(7), gid = %d\n", getgid());
  exit(0);
}
