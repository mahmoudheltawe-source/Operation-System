#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

#define NCHILD 6

int
main(void)
{
  int lock_id = israeli_create(100);
  if(lock_id < 0){
    printf("failed to create Israeli lock\n");
    exit(1);
  }

  setgid(1);
  if(israeli_acquire(lock_id) < 0){
    printf("parent failed to acquire lock\n");
    exit(1);
  }

  printf("parent %d holds lock %d\n", getpid(), lock_id);

  for(int i = 0; i < NCHILD; i++){
    int pid = fork();
    if(pid < 0){
      printf("fork failed\n");
      exit(1);
    }

    if(pid == 0){
      int gid = i % 3;
      setgid(gid);
      printf("child %d gid=%d waiting\n", getpid(), getgid());

      if(israeli_acquire(lock_id) < 0){
        printf("child %d failed to acquire\n", getpid());
        exit(1);
      }

      printf("child %d gid=%d acquired\n", getpid(), getgid());
      sleep(1);

      if(israeli_release(lock_id) < 0){
        printf("child %d failed to release\n", getpid());
        exit(1);
      }

      printf("child %d released\n", getpid());
      exit(0);
    }
  }

  sleep(20);
  printf("parent releasing lock\n");

  if(israeli_release(lock_id) < 0){
    printf("parent failed to release lock\n");
    exit(1);
  }

  for(int i = 0; i < NCHILD; i++){
    wait(0);
  }

  if(israeli_destroy(lock_id) < 0){
    printf("failed to destroy lock\n");
    exit(1);
  }

  printf("israeli_lock_test done\n");
  exit(0);
}
