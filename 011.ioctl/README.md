# 011: ioctl

`read`/`write` only move a stream of bytes. To let a program send *commands* ("clear the buffer", "how big is the buffer?", "store this value") you add an ioctl handler: one function that receives a command number plus one argument.

This lesson is the first with a **userspace program** (`user_app.c`) talking to the kernel module.

## What the module does

- Registers `/dev/mydevice` (misc device, like lessons 007/008)
- Handles plain `write()` (stores bytes in a 1024-byte buffer)
- Handles three ioctl commands:
  - `IOCTL_CLEAR` (`_IO`) - clear the buffer, no data
  - `IOCTL_GET_SIZE` (`_IOR`) - kernel sends the buffer size out
  - `IOCTL_SET_VALUE` (`_IOW`) - kernel receives an int
- `user_app.c` exercises all of them

## How the command numbers work

The magic byte is an arbitrary character identifying our device; `_IO`/`_IOR`/`_IOW` pack magic, sequence number, data size and direction into one number:

```c
#define IOCTL_MAGIC 'g'
#define IOCTL_CLEAR     _IO(IOCTL_MAGIC, 1)       /* no data          */
#define IOCTL_GET_SIZE  _IOR(IOCTL_MAGIC, 2, int) /* int sent OUT     */
#define IOCTL_SET_VALUE _IOW(IOCTL_MAGIC, 3, int) /* int sent IN      */
```

R = data flows **out** of the kernel, W = data flows **in** (from the *device's* point of view - easy to mix up). The same three `#define`s must appear in `user_app.c`: the number IS the command.

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

**Bug:** `./user_app` fails with `ioctl: Inappropriate ioctl for device` (errno `ENOTTY`).

**Cause:** the command numbers in `user_app.c` and `main.c` disagree - different magic byte, different sequence number, or a different data type inside the macro. The kernel only compares raw integers.

**Fix:** keep one definition. Magic and numbers must match *exactly*, including the type (`_IOR(magic, 2, int)` vs `_IOR(magic, 2, long)` produce different numbers). Best practice: put the `#define`s in a shared header as the project grows.

## Rust version (main.rs)

Command numbers are plain `const u32` values computed with the same bit layout the C macros use, and there is no `int __user *` cast: `UserSlice::new(arg, size)` gives the enforced-safe `copy_to_user`/`copy_from_user`. Unknown commands return the same `ENOTTY`. Same API-version caveat as lesson 007 (targets ~6.8 rust-next, needs a `CONFIG_RUST=y` kernel such as `linux-rust` on Arch).
