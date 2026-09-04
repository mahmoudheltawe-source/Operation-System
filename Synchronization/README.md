# Homework 2 - Synchronization

## Team Members

- Mahmoud Haj Yahya
- Zaki Masarwa

## Overview

This homework extends xv6 with synchronization services implemented in the kernel. It introduces a synchronized pseudo-random number generator, process group identifiers, an **Israeli lock** with configurable favoritism, and a relay-race simulation that demonstrates the tradeoff between FIFO fairness and group favoritism.

## Implemented Features

### 1. Pseudo-Random Number Generator

A thread-safe linear congruential generator is exposed to userspace through:

```c
void lcg_srand(uint seed);
uint lcg_rand(void);
```

The generator uses the assignment parameters:

```text
x(n+1) = 1664525 * x(n) + 1013904223 (mod 2^32)
```

### 2. Process Group IDs

Each process has a group identifier used by the Israeli-lock policy:

```c
void setgid(int gid);
int getgid(void);
```

### 3. Israeli Lock

The kernel maintains an array of Israeli locks. Each lock uses a FIFO waiting queue but may prefer the earliest waiting process whose group matches the releasing process.

```c
int israeli_create(int favoritism);
int israeli_acquire(int lock_id);
int israeli_release(int lock_id);
int israeli_destroy(int lock_id);
```

The `favoritism` value is a percentage from `0` to `100`:

- `0` means pure FIFO selection.
- `100` always prefers an eligible same-group waiter.
- Intermediate values probabilistically select between favoritism and FIFO.

Waiting processes sleep instead of busy-waiting.

### 4. Relay Race Tournament

`user/relay_race.c` creates three teams with five runners per team. The Israeli lock represents the baton. Every lock acquisition increments the runner's team score, and the program compares races with favoritism values `0`, `50`, and `100`.

## Important Files

- `kernel/random.c` - synchronized LCG implementation.
- `kernel/israeli_lock.c` - Israeli-lock storage, waiting queue, acquisition, release, and favoritism policy.
- `kernel/race.c` - synchronized race scores and winner detection.
- `kernel/proc.h` - process `gid` field.
- `kernel/sysproc.c` - userspace-facing system-call handlers.
- `kernel/syscall.c`, `kernel/syscall.h` - system-call registration and numbers.
- `kernel/main.c` - initializes the Israeli-lock and race subsystems.
- `user/pseudo_random_test.c` - PRNG test.
- `user/gid_test.c` - process group test.
- `user/israeli_lock_test.c` - concurrent Israeli-lock test.
- `user/race_test.c` - race-score API test.
- `user/relay_race.c` - complete relay-race experiment.
- `os262-assignment2.pdf` - original assignment instructions.

## Requirements

The project needs:

- GNU Make and a C build environment.
- A RISC-V cross compiler, such as `gcc-riscv64-linux-gnu`.
- `qemu-system-riscv64`.
- Linux, WSL, or a Docker-based xv6 development environment is recommended.

## How to Run

From the `hw2` directory:

```bash
make clean
make qemu
```

After xv6 boots, run any of the following commands in the xv6 shell:

```text
pseudo_random_test
gid_test
israeli_lock_test
race_test
relay_race
```

### What Each Test Does

- `pseudo_random_test` seeds the LCG and prints generated values. With seed `1`, the first value should be `1015568748`.
- `gid_test` displays the initial group ID and verifies `setgid(7)`/`getgid()`.
- `israeli_lock_test` creates several competing child processes with different groups.
- `race_test` validates score reset, increment, query, and winner detection.
- `relay_race` automatically runs three full tournaments with favoritism values `0`, `50`, and `100`, then prints the winning team and final scores.

To exit QEMU, press `Ctrl+A`, release the keys, and then press `X`.

## Optional Debugging

```bash
make qemu-gdb
```

This starts QEMU paused with a GDB server so the kernel can be inspected from another terminal.

## Clean Generated Files

Before submission or packaging, run:

```bash
make clean
```

## Notes

- Race winners can vary because lock decisions use pseudo-randomness and process scheduling is nondeterministic.
- The implementation uses kernel synchronization to protect shared generator, lock, queue, and score state.
- This homework runs locally in QEMU. It does not use AWS or other cloud services.
