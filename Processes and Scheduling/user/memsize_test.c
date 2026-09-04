#include "kernel/types.h"
#include "user/user.h"

int
main(void)
{
  char *arr;

  printf("Memory before malloc: %d bytes\n", memsize());

  arr = malloc(20 * 1024);
  if(arr == 0){
    printf("malloc failed\n");
    exit(1);
  }

  printf("Memory after malloc: %d bytes\n", memsize());

  free(arr);

  printf("Memory after free: %d bytes\n", memsize());

  exit(0);
}