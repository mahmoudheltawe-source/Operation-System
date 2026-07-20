# Homework 3 - Memory Management and Display

## Team Members

- Mahmoud Haj Yahya
- Zaki Masarwa

## Overview

This homework extends a customized xv6 build with a VirtIO GPU display subsystem. It demonstrates two approaches for exposing framebuffer memory to userspace:

1. Mapping kernel-owned framebuffer pages into a process page table.
2. Performing a zero-copy page flip that changes the GPU's physical backing pages.

The implementation exercises page-table walking, virtual-to-physical translation, mapping validation, resource lifetime handling, and double buffering.

## Implemented Features

### 1. Memory-Mapped Framebuffer

```c
void *map_display(void *addr);
```

- If `addr == 0`, the kernel chooses a suitable page-aligned virtual address.
- If `addr != 0`, it must be page-aligned and the complete framebuffer range must be unmapped.
- The kernel maps all GPU framebuffer pages with user read/write permissions.
- The mapping is removed safely during process cleanup or `exec` without freeing the kernel-owned framebuffer pages.

### 2. Zero-Copy Page Flip

```c
int flip_display(void *buf);
```

The kernel validates the page-aligned userspace buffer, walks the process page table, converts every user virtual page into its physical address, and reattaches the GPU resource to those pages. Pixel data is not copied between userspace and kernel space.

### 3. Display Programs

- `show_map` interactively draws text through the mapped kernel framebuffer.
- `show_flip <text>` draws text in a page-aligned userspace buffer and presents it using zero-copy flipping.
- `gol`, `gol flip`, and `gol map` run Conway's Game of Life using both interfaces.

## Important Files

- `kernel/virtio_gpu.c` - GPU initialization, framebuffer mapping/unmapping, page flip, and periodic display commit.
- `kernel/sysproc.c` - `map_display` and `flip_display` system calls and validation.
- `kernel/proc.c`, `kernel/exec.c` - display-mapping cleanup during process exit and image replacement.
- `kernel/defs.h` - GPU helper declarations.
- `kernel/syscall.c`, `kernel/syscall.h` - system-call registration and numbers.
- `user/user.h`, `user/usys.pl` - userspace system-call APIs.
- `user/show_map.c` - mapped-framebuffer text demo.
- `user/show_flip.c` - zero-copy text demo.
- `user/gol.c` - Game of Life animation in flip and map modes.
- `os262-assignment3.pdf` - original assignment instructions.

## Requirements

The project needs:

- GNU Make and a C build environment.
- A RISC-V cross compiler, such as `gcc-riscv64-linux-gnu`.
- `qemu-system-riscv64` with VirtIO GPU and VNC support.
- `websockify` and noVNC for the browser-based display.
- Recommended: Docker, Visual Studio Code, and the Dev Containers extension.

The included `.devcontainer` installs the RISC-V toolchain, QEMU, noVNC, and `websockify`.

## How to Run with the Browser Display

From the `hw3` directory:

```bash
make clean
make qemu-web
```

The Makefile starts a noVNC proxy on port `6080`. Open one of these addresses in a browser:

```text
http://localhost:6080/vnc.html
http://localhost:6080/vnc_auto.html
```

Use the xv6 shell in the QEMU terminal to run:

```text
show_map
show_flip Hello World
gol
gol flip
gol map
```

### Program Behavior

- `show_map` waits for text input. Enter text to display it; type `exit` to clear the display and quit.
- `show_flip Hello World` immediately presents the supplied text using a userspace framebuffer.
- `gol` is equivalent to `gol flip` and uses double-buffered zero-copy page flipping.
- `gol map` writes directly into the mapped kernel framebuffer.

To exit QEMU, use the QEMU monitor/terminal controls available in your environment. In the common terminal setup, press `Ctrl+A`, release the keys, and then press `X`.

## Run Without noVNC

The regular target also starts xv6 with the VirtIO GPU device:

```bash
make qemu
```

Use `make qemu-web` when a browser-accessible display is required.

## Optional Debugging

```bash
make qemu-gdb
```

This starts xv6 with a GDB server for kernel debugging.

## Clean Generated Files

Before submission or packaging, run:

```bash
make clean
```

## Notes

- The framebuffer is `640 x 480` pixels with 32-bit BGRX pixels, totaling `1,228,800` bytes or `300` xv6 pages.
- Flip mode requires page-aligned buffers; the demo programs use `sbrk` because ordinary `malloc` does not guarantee page alignment.
- Double buffering prevents the GPU from reading a frame while the program is still drawing it.
- This homework runs locally in QEMU. It does not use AWS or other cloud services.
