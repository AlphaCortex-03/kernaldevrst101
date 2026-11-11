# 011.ioctl

`read`/`write` only move a stream of bytes. To let a program send
*commands* ("clear the buffer", "how big is the buffer?", "store this
value") you add an ioctl handler: one function that receives a command
number plus one argument.

This lesson is the first with a **userspace program** (`user_app.c`)
talking to the kernel module.

## What

- Registers `/dev/mydevice` (misc device, like lessons 007/008)
- Handles plain `write()` (stores bytes in a 1024-byte buffer)
- Handles three ioctl commands:
  - `IOCTL_CLEAR` (`_IO`) - clear the buffer, no data
  - `IOCTL_GET_SIZE` (`_IOR`) - kernel sends the buffer size out
  - `IOCTL_SET_VALUE` (`_IOW`) - kernel receives an int
- `user_app.c` exercises all of them

## What you learn

- `_IO` / `_IOR` / `_IOW` command encoding and the magic byte
  (R = data flows **out** of the kernel, W = data flows **in** - easy
  to mix up, they are from the *device's* point of view)
- `.unlocked_ioctl = my_ioctl` in `file_operations`
- `ENOTTY` as the traditional "unknown ioctl command" error
- How `ioctl()` from libc reaches your kernel handler

## Files

| File         | Purpose                                             |
| ------------ | --------------------------------------------------- |
| `main.c`     | kernel module with the ioctl handler                |
| `user_app.c` | userspace program that calls `ioctl()`              |
| `main.rs`    | Rust twin (`ioctl` method of the `MiscDevice` trait)|
| `Makefile`   | wrapper; also builds `user_app` with gcc            |
| `Kbuild`     | the real build rules (`obj-m`)                      |
| `README.md`  | this file                                           |

## How to build

```bash
cd 011.ioctl
make          # builds myioctl.ko AND user_app
```

## How to test

```bash
sudo insmod myioctl.ko
./user_app
dmesg | tail
sudo rmmod myioctl
```

## Expected output

user_app:

```
wrote 11 bytes to the device
kernel says buffer size = 11
sent value 99 to the kernel
sent CLEAR command
size after clear = 0
```

dmesg:

```
myioctl_init: /dev/mydevice ready (ioctl commands 0x67*)
my_open: device opened
my_write: stored 11 bytes
my_ioctl: cmd=0x80046702 arg=0x7ffc...
my_ioctl: told userspace size=11
my_ioctl: cmd=0x40046703 arg=0x7ffc...
my_ioctl: stored value=99
my_ioctl: cmd=0x6701 arg=0x0
my_ioctl: buffer cleared
my_ioctl: cmd=0x80046702 arg=0x7ffc...
my_ioctl: told userspace size=0
my_release: device closed
```

## Common bug + fix

**Bug:** `./user_app` fails with `ioctl: Inappropriate ioctl for
device` (errno `ENOTTY`).

**Cause:** the command numbers in `user_app.c` and `main.c` disagree -
different magic byte, different sequence number, or a different `type`
in `_IOR`/`_IOW`. The number IS the command; the kernel only compares
raw integers.

**Fix:** keep one definition. The magic and numbers must match
*exactly*, including the data type inside the macro (`_IOR(magic, 2,
int)` vs `_IOR(magic, 2, long)` produce different numbers). Best
practice: put the `#define`s in a shared header when you grow out of
two files.

## Rust version (main.rs)

- Command numbers are plain `const u32` values computed with the same
  bit layout the C macros use (some trees also ship `kernel::ioctl`
  helpers)
- `int __user *` does not exist in Rust: `UserSlice::new(arg, size)`
  gives you the enforced-safe `copy_to_user`/`copy_from_user`
- The unknown-command branch returns the same `ENOTTY`
- Same API-version caveat as lesson 007 (targets ~6.8 rust-next, needs
  a `CONFIG_RUST=y` kernel such as `linux-rust` on Arch)
