# 008.buffer_module

Lesson 007 only *saw* the `read`/`write` calls. Here we actually move
data through the device: `echo` writes bytes into a kernel buffer and
`cat` reads them back out.

The one rule of this lesson: **userspace pointers are never touched
directly**. `copy_from_user()` / `copy_to_user()` validate the address
and handle faults; `strcpy`/`memcpy` on a `__user` pointer is a
kernel-crashing bug.

## What

- `write` copies userspace bytes into the kernel buffer `kbuf[1024]`
  with `copy_from_user()` and remembers the length
- `read` copies the stored bytes back out with `copy_to_user()`,
  respecting the file position `*pos`, and returns 0 at the end
- One trailing `\n` is stripped on write, so `echo` + `cat` round-trips

## What you learn

- Why `__user` pointers must go through `copy_*_user()`
- That `copy_*_user()` returns "bytes NOT copied", not bytes copied
- How `*pos` (file position) makes `cat` stop instead of looping
- Buffer size clamping: never write past `BUF_SIZE`

## Files

| File       | Purpose                                       |
| ---------- | --------------------------------------------- |
| `main.c`   | C module with the 1024-byte kernel buffer     |
| `main.rs`  | Rust twin (`KVec` + `Mutex`, enforced-safe copies) |
| `Makefile` | Wrapper that invokes the kernel build         |
| `Kbuild`   | The real build rules (`obj-m`)                |
| `README.md`| This file                                     |

## How to build

```bash
cd 008.buffer_module
make
```

(First remove lesson 007's module if it is still loaded - both create
`/dev/mydevice`: `sudo rmmod mychar`.)

## How to test

```bash
sudo insmod mybuf.ko
echo "hello kernel" > /dev/mydevice
cat /dev/mydevice          # prints: hello kernel
dmesg | tail
sudo rmmod mybuf
dmesg | tail
```

## Expected output

```
mybuf_init: /dev/mydevice ready (kernel buffer of 1024 bytes)
my_open: device opened
my_write: write(count=13)
my_write: stored "hello kernel" (12 bytes)
my_release: device closed
my_open: device opened
my_read: read(count=4096, pos=0)
my_read: returned 12 bytes
my_release: device closed
mybuf_exit: /dev/mydevice removed
```

## The bug you MUST NOT make

```c
/* WRONG - crash or security hole: */
memcpy(kbuf, ubuf, count);   /* ubuf is a __user pointer! */
strcpy(kbuf, ubuf);          /* same crime, plus unbounded */
```

Why it is wrong: `ubuf` points into the *user process's* address space.
It may be swapped out, unmapped, or deliberately pointed at kernel
memory. `copy_from_user()`/`copy_to_user()` validate the address and
fail gracefully (returning the number of bytes NOT copied); a direct
`memcpy` dereferences it raw - on VMware you get an instant kernel
oops/hang, and with a malicious process it is an exploitable hole.

**Fix:** always use the helpers:

```c
if (copy_from_user(kbuf, ubuf, to_copy))
        return -EFAULT;   /* some bytes did not make it */
```

Also note the return convention: `copy_*_user()` returns the number of
bytes **not** copied, so success is `0`, not a positive size.

## Rust version (main.rs)

- The buffer is `KVec<u8>` behind a `Mutex` instead of a static `char[]`
- `reader.read_all(...)` and `writer.write_slice(...)` are the only way
  to touch userspace memory - the safe `copy_from_user`/`copy_to_user`
  equivalents, enforced by the type system
- Same API-version caveat as lesson 007 (targets ~6.8 rust-next, needs
  a `CONFIG_RUST=y` kernel such as `linux-rust` on Arch)
