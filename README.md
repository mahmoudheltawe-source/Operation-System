# Operating Systems Homeworks - xv6

## Team Members

- Mahmoud Haj Yahya
- Zaki Masarwa

## Repository Overview

This repository contains three Operating Systems assignments implemented on the RISC-V version of **xv6**. Together, the assignments cover system calls, process scheduling, synchronization, and virtual memory/device mapping.

```text
Operation System/
|-- hw1/
|   |-- README.md
|   `-- Processes and scheduling implementation
|-- hw2/
|   |-- README.md
|   `-- Synchronization implementation
|-- hw3/
|   |-- README.md
|   `-- Memory-management and display implementation
`-- README.md
```

## Homework Summary

### HW1 - Processes and Scheduling

HW1 introduces xv6 userspace programs and kernel system calls. It includes:

- A `helloworld` userspace program.
- A `memsize()` system call that reports process memory size.
- A `co_yield(pid, value)` system call for cooperative process-to-process CPU handoff and value exchange.
- Tests for valid coroutine exchange and error conditions.

Purpose: understand the userspace-to-kernel system-call path, process control blocks, scheduling, sleeping/waking, and context switching.

Run from `hw1`:

```bash
make qemu
```

Then run inside xv6:

```text
helloworld
memsize_test
co_test
```

See [`hw1/README.md`](hw1/README.md) for full instructions.

### HW2 - Synchronization

HW2 adds kernel synchronization mechanisms and a concurrent experiment. It includes:

- A synchronized linear congruential pseudo-random generator.
- Per-process group IDs.
- An Israeli lock with a FIFO queue and configurable same-group favoritism.
- Race-score system calls.
- A relay-race simulation comparing favoritism values `0`, `50`, and `100`.

Purpose: practice kernel locking, sleep/wakeup synchronization, waiting queues, mutual exclusion, fairness, and the effects of controlled scheduling bias.

Run from `hw2`:

```bash
make qemu
```

Then run inside xv6:

```text
pseudo_random_test
gid_test
israeli_lock_test
race_test
relay_race
```

See [`hw2/README.md`](hw2/README.md) for full instructions.

### HW3 - Memory Management and Display

HW3 extends a customized xv6 template with a VirtIO GPU interface. It includes:

- `map_display()` for mapping kernel framebuffer pages into userspace.
- `flip_display()` for zero-copy GPU page flipping.
- Safe display mapping cleanup during process exit and `exec`.
- Text-display demos and Conway's Game of Life in mapped and double-buffered flip modes.

Purpose: understand page tables, virtual-to-physical translation, mapping permissions, kernel/user memory sharing, zero-copy I/O, and double buffering.

Run from `hw3`:

```bash
make qemu-web
```

Open the noVNC viewer at:

```text
http://localhost:6080/vnc.html
```

Then run inside xv6:

```text
show_map
show_flip Hello World
gol flip
gol map
```

See [`hw3/README.md`](hw3/README.md) for full instructions.

## Common Development Requirements

The assignments require:

- GNU Make and a C build environment.
- A RISC-V cross compiler.
- `qemu-system-riscv64`.
- Linux, WSL, or Docker-based development is recommended.
- HW3 additionally requires noVNC and `websockify` for the browser display.

HW1 and HW3 include `.devcontainer` configurations for Visual Studio Code. A typical setup is:

1. Install Docker.
2. Install Visual Studio Code.
3. Install the **Dev Containers** extension.
4. Open the selected homework directory.
5. Choose **Reopen in Container**.
6. Run the appropriate `make` command in the container terminal.

## General Build Commands

Run commands from inside the selected homework directory.

```bash
make clean       # remove generated files
make qemu        # build and run xv6
make qemu-gdb    # build and run xv6 with a GDB server
```

HW3 also provides:

```bash
make qemu-web    # run xv6 with VirtIO GPU and noVNC display
```

For the standard non-graphical QEMU sessions, exit by pressing `Ctrl+A`, releasing the keys, and then pressing `X`.

## AWS / Cloud Usage

These are Operating Systems/xv6 assignments and **do not use AWS services**. Compilation and execution are performed locally inside a development environment, and xv6 runs in QEMU. Docker may be used only to provide a reproducible local toolchain.

## Submission Preparation

Before packaging an individual homework, enter its directory and run:

```bash
make clean
```

This removes kernels, object files, generated system-call assembly, filesystem images, and other build outputs.

## Academic Purpose

The repository is intended for educational use in the Spring 2026 Operating Systems course. The xv6 base system remains subject to its original license, included separately in every homework directory.
