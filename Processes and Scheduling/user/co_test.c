#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

static void
print_result(char *name, int ok)
{
  if (ok)
    printf("[OK] %s\n", name);
  else
    printf("[FAIL] %s\n", name);
}

static void
check_basic_errors(void)
{
  int ok;

  printf("Checking basic error cases...\n");

  ok = (co_yield(-1, 1) == -1) && (co_yield(0, 1) == -1);
  print_result("invalid pid", ok);

  ok = (co_yield(9999, 1) == -1);
  print_result("missing pid", ok);

  ok = (co_yield(getpid(), 1) == -1);
  print_result("self yield", ok);
}

static void
check_killed_target(void)
{
  int pid;
  int ok;

  printf("Checking killed-target case...\n");

  pid = fork();
  if (pid < 0) {
    printf("fork failed in killed-target test\n");
    exit(1);
  }

  if (pid == 0) {
    sleep(100);
    exit(0);
  }

  kill(pid);
  sleep(1);

  ok = (co_yield(pid, 1) == -1);
  print_result("yield to killed process", ok);

  wait(0);
}

static void
run_parent_loop(int child_pid)
{
  for (;;) {
    int x = co_yield(child_pid, 2);
    printf("Parent got: %d\n", x);
  }
}

static void
run_child_loop(int parent_pid)
{
  for (;;) {
    int x = co_yield(parent_pid, 1);
    printf("Child got: %d\n", x);
  }
}

static void
check_ping_pong(void)
{
  int parent_pid;
  int child_pid;

  printf("Starting ping-pong test...\n");

  parent_pid = getpid();
  child_pid = fork();

  if (child_pid < 0) {
    printf("fork failed in ping-pong test\n");
    exit(1);
  }

  if (child_pid == 0) {
    run_child_loop(parent_pid);
    exit(0);
  }

  run_parent_loop(child_pid);
}

int
main(void)
{
  printf("\n=== co_yield test program ===\n\n");

  check_basic_errors();
  check_killed_target();

  printf("\nNow entering infinite ping-pong test.\n");
  printf("Stop QEMU manually when you have seen enough output.\n\n");

  check_ping_pong();

  exit(0);
}