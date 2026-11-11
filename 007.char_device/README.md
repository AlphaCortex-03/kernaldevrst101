# 007.char_device

A character device is the simplest way to expose kernel functionality to
userspace: it shows up as a normal file in `/dev`, and every `open()`,
`read()`, `write()` and `close()` on that file is routed by the kernel
into one of your functions.

We use `misc_register()` instead of registering a real major number:
misc devices all share major 10, the kernel picks a free minor, and it
even creates the `/dev/mydevice` node for us automatically.

## What

- Registers `/dev/mydevice` via `misc_register()`
- Implements `file_operations`: `open`, `read`, `write`, `release`
- Every callback only logs to dmesg (real data transfer = lesson 008)

## What you learn

- What `file_operations` is and how the VFS routes file calls into it
- Why `miscdevice` is the easy way to make a character device
- What `__user` on a pointer parameter means (a userspace pointer you
  must never dereference directly)
- That returning `0` from `read()` means EOF

## Files

| File       | Purpose                                        |
| ---------- | ---------------------------------------------- |
| `main.c`   | C module: registers the misc device            |
| `main.rs`  | Rust twin using the `miscdevice` bindings      |
| `Makefile` | Wrapper that invokes the kernel build          |
| `Kbuild`   | The real build rules (`obj-m`)                 |
| `README.md`| This file                                      |

## How to build

```bash
# once per kernel: Arch needs the headers of the RUNNING kernel
sudo pacman -S linux-headers

cd 007.char_device
make
```

Expected: `CC [M]  main.o` ... `LD [M]  mychar.ko`

## How to test

```bash
sudo insmod mychar.ko
ls -l /dev/mydevice      # note "10, XX" = major 10 = misc device
echo hi > /dev/mydevice  # triggers open -> write -> release
cat /dev/mydevice        # prints nothing: read() returns 0 = EOF
dmesg | tail
sudo rmmod mychar
dmesg | tail
```

## Expected output

```
mychar_init: /dev/mydevice registered (minor=XX)
my_open: device opened
my_write: userspace wrote 3 bytes (ignored for now)
my_release: device closed
mychar_exit: /dev/mydevice removed
```

(`echo hi` sends the 3 bytes `h`, `i`, `\n`.)

## Common bug + fix

**Bug:** `cat /dev/mydevice` never stops and dmesg floods with read
messages.

**Cause:** your `read()` returns a positive size every call and never
advances `*pos`, so `cat` loops forever, always believing there is more
data.

**Fix:** honor the `*pos` offset and return `0` when there is nothing
left - `0` from `read()` is EOF and makes `cat` stop. Lesson 008 shows a
`read()` that does this correctly.

## Rust version (main.rs)

`main.rs` is the same module written against the Rust-for-Linux
bindings. Differences to know:

- There is no `file_operations` struct: you implement the `MiscDevice`
  trait and the bindings build the table for you.
- There is no raw `__user` pointer: the `UserSlicePtrReader` /
  `UserSlicePtrWriter` handed to `write`/`read` are the enforced-safe
  equivalents of `copy_from_user()` / `copy_to_user()`.
- The Rust-for-Linux API is not in mainline yet and changes between
  releases; this file targets the ~6.8 rust-next API. Building Rust
  kernel modules needs a `CONFIG_RUST=y` kernel (on Arch: `linux-rust`)
  and a matching rustup toolchain. The C version above is what this
  folder builds and tests by default.
