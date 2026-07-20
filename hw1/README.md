# Homework 1 - Processes and Scheduling

## Team Members

- Mahmoud Haj Yahya
- Zaki Masarwa

## Overview

This homework extends the RISC-V version of **xv6** with user programs and new process-related system calls. Its purpose is to practice the complete path from a userspace API to kernel implementation, and to study process state transitions, scheduling, sleeping, waking, and direct context switching.

## Implemented Features

### 1. Hello World Userspace Program

`user/helloworld.c` is a simple xv6 userspace program that prints:

```text
Hello World xv6
```

### 2. `memsize` System Call

```c
int memsize(void);
```

The system call returns the current process memory size in bytes. The program `user/memsize_test.c` prints the memory size before allocating 20 KB, after the allocation, and after freeing the memory.

### 3. `co_yield` Coroutine System Call

```c
int co_yield(int pid, int value);
```

`co_yield` implements cooperative process handoff. A process voluntarily transfers execution and an integer value to another process. The implementation handles invalid PIDs, missing or killed targets, self-yield attempts, sleeping/waking, and direct process-to-process switching.

The homework intentionally runs xv6 with one CPU because the assignment requires `co_yield` to work correctly for processes executing on the same CPU.

## Important Files

- `kernel/proc.c` - process and scheduling logic, including coroutine handoff support.
- `kernel/sysproc.c` - kernel implementations of `memsize` and `co_yield`.
- `kernel/syscall.c`, `kernel/syscall.h` - system-call registration and numbers.
- `kernel/defs.h` - kernel function declarations.
- `user/user.h`, `user/usys.pl` - userspace system-call interfaces.
- `user/helloworld.c` - Hello World program.
- `user/memsize_test.c` - `memsize` test.
- `user/co_test.c` - `co_yield` error and ping-pong tests.
- `Makefile` - includes the new programs and sets `CPUS := 1`.
- `os262-assignment1.pdf` - original assignment instructions.

## Requirements

The project needs:

- GNU Make and a C build environment.
- A RISC-V cross compiler, such as `gcc-riscv64-linux-gnu`.
- `qemu-system-riscv64`.
- Recommended: Docker, Visual Studio Code, and the Dev Containers extension.

The included `.devcontainer` configuration installs the required compiler and QEMU packages.

## How to Run

From the `hw1` directory:

```bash
make clean
make qemu
```

After xv6 boots, run the programs from the xv6 shell:

```text
helloworld
memsize_test
co_test
```

Expected behavior:

- `helloworld` prints `Hello World xv6`.
- `memsize_test` prints the process memory size before and after allocation/free.
- `co_test` first checks error cases and then enters an infinite parent-child ping-pong test that repeatedly exchanges the values `1` and `2`.

To exit QEMU, press `Ctrl+A`, release the keys, and then press `X`.

## Optional Debugging

Start xv6 with its GDB server:

```bash
make qemu-gdb
```

Then connect using a RISC-V-compatible GDB session from another terminal.

## Clean Generated Files

Before submission or packaging, run:

```bash
make clean
```

## Notes

- The `co_test` ping-pong stage is intentionally infinite; stop QEMU manually after observing correct output.
- This homework runs locally in QEMU. It does not use AWS or other cloud services.
- The original xv6 license and acknowledgements remain in `LICENSE` and `README`.
