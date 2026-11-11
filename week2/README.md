# week2 - kernel lessons 007-012

Week 2 of the kernel learning repo. Six lessons that move past
"hello world" modules: real devices, real data transfer, module
parameters, sysfs, ioctl, and kernel memory.

## What

| Lesson | Module (.ko)   | Creates                        | Learns                                   |
| ------ | -------------- | ------------------------------ | ---------------------------------------- |
| 007.char_device  | `mychar.ko`   | `/dev/mydevice`                  | misc device + file_operations            |
| 008.buffer_module| `mybuf.ko`    | `/dev/mydevice` (with a buffer)  | copy_to_user / copy_from_user            |
| 009.module_params| `myparams.ko` | `/sys/module/myparams/...`       | module_param at insmod time              |
| 010.sysfs        | `mysysfs.ko`  | `/sys/kernel/my_mod/value`       | kobject + show/store attributes          |
| 011.ioctl        | `myioctl.ko`  | `/dev/mydevice` + `user_app`     | _IO/_IOR/_IOW commands from userspace    |
| 012.memory       | `mymemory.ko` | -                                | kmalloc vs kzalloc vs vmalloc, GFP flags |

Every lesson folder contains: `main.c` (C module), `main.rs` (same
module in Rust for the kernel crate), `Makefile`, `Kbuild`,
`README.md`. Lesson 011 also has `user_app.c`, a normal C program that
talks to the module.

## How to build

Needs Arch Linux (or any Linux) with the headers of the running
kernel, kernel 6.x, x86_64:

```bash
sudo pacman -S linux-headers   # once
cd week2/007.char_device       # then per lesson
make
```

## How to test

One lesson at a time (007, 008 and 011 all use /dev/mydevice):

```bash
sudo insmod mychar.ko
ls -l /dev/mydevice
echo hi > /dev/mydevice
dmesg | tail
sudo rmmod mychar
```

Exact commands per lesson are in each folder's README.

## Expected output

Each `make` ends with `LD [M]  <module>.ko`, and `dmesg | tail` shows
the load/test/unload messages listed in that lesson's README.

Note on main.rs: the Rust files target the Rust-for-Linux ~6.8 API and
need a CONFIG_RUST=y kernel (linux-rust on Arch) plus a matching
rustup toolchain. The C files are what this repo builds and tests by
default.
