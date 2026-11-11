# 007: Character Device

A character device is the simplest way to expose kernel functionality to userspace: it shows up as a normal file in `/dev`, and every `open()`, `read()`, `write()` and `close()` on that file is routed by the kernel into one of your functions.

We use `misc_register()` instead of registering a real major number: misc devices all share major 10, the kernel picks a free minor, and it even creates the `/dev/mydevice` node for us automatically.

## What the module does

- Registers `/dev/mydevice` via `misc_register()`
- Implements `file_operations`: `open`, `read`, `write`, `release`
- Every callback only logs to dmesg (real data transfer is lesson 008)

## Licensing

The module declares `MODULE_LICENSE("GPL")`. Without it, loading fails or taints the kernel, and GPL-only kernel symbols are not available.

## The four file_operations callbacks

| Callback | Called when | Return value |
| -------- | ----------- | ------------ |
| `open` | a program opens the file | `0` on success |
| `read` | a program reads the file | bytes delivered; `0` means EOF |
| `write` | a program writes the file | bytes accepted |
| `release` | the file is closed | `0` on success |

`.owner = THIS_MODULE` keeps the module loaded while the file is open, so the module cannot be unloaded under a running file descriptor.

## How to build

```bash
sudo pacman -S linux-headers   # once: Arch needs headers of the running kernel
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

**Bug:** `cat /dev/mydevice` never stops and dmesg floods with read messages.

**Cause:** `read()` returns a positive size every call and never advances `*pos`, so `cat` loops forever, always believing there is more data.

**Fix:** honor the `*pos` offset and return `0` when there is nothing left - `0` from `read()` is EOF and makes `cat` stop. Lesson 008 shows a `read()` that does this correctly.

## Rust version (main.rs)

`main.rs` is the same module against the Rust-for-Linux bindings: you implement the `MiscDevice` trait and the bindings build the file_operations table for you. There is no raw `__user` pointer either - the `UserSlicePtrReader`/`Writer` handed to write/read are the enforced-safe `copy_from_user()`/`copy_to_user()`.

The Rust-for-Linux API is not in mainline yet and changes between releases; `main.rs` targets the ~6.8 rust-next API and needs a `CONFIG_RUST=y` kernel (on Arch: `linux-rust`) plus a matching rustup toolchain. The C version is what this folder builds and tests by default.
