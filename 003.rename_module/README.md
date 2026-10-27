# 003: Renaming the Output Kernel Module

If you have a source code file named `hello.c` but you want the output module file to have a different name, you can achieve this by defining the desired module name and then mapping it to the appropriate object file.

## C version

Given `c/hello.c`, this `Makefile` produces a `renamed_hello.ko`:

```makefile
obj-m := renamed_hello.o
renamed_hello-objs := hello.o

all:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

.PHONY: all clean
```

Explanation:
------------

- `obj-m := renamed_hello.o`: This line informs the kernel build system that you're aiming to produce a module named `renamed_hello.ko`.

- `renamed_hello-objs := hello.o`: This specifies that the `renamed_hello.ko` module should be built using the `hello.o` object file which comes from compiling `hello.c`.

When you execute `make`, the kernel build system will first compile `hello.c` to get `hello.o`. Then, instead of creating a module named `hello.ko` (the default), it links `hello.o` into `renamed_hello.ko`.

Load and verify:

```bash
sudo insmod ./renamed_hello.ko
lsmod        # shows: renamed_hello
sudo rmmod renamed_hello
```

> **Note:** The original version of this lesson used `linux` as the module name — that is confusing because a module called `linux` masks the real kernel identity in `modinfo` output and is easy to mistake for the kernel itself in `lsmod`. A descriptive name like `renamed_hello` is the better habit.

## Rust version

In Rust for Linux there is no `MODULE_*` macro Renaming works differently: the module's registered name comes from the `name:` field of the `module!` macro, and the `.ko` file name comes from the `.rs` source file name selected by `obj-m`. So to "rename":

1. Name your source file what you want the `.ko` to be called (`renamed_rust.rs` -> `renamed_rust.ko`), and
2. Set `name:` inside `module! { ... }` (this is the name `lsmod`, `/sys/module/`, and `modinfo` show).

Build:

```bash
make LLVM=1 -C /lib/modules/$(uname -r)/build M=$PWD modules
sudo insmod ./renamed_rust.ko
lsmod        # shows: my_renamed_module
sudo rmmod my_renamed_module
```
