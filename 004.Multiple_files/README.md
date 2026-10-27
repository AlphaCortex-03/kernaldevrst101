# 004: Including Additional Source Files in Kernel Module Compilation

When building a kernel module that involves multiple source files, you need to adjust the `Makefile` to consider the additional files.

## C version

Given `c/hello.c` as the main source file plus a helper `c/func.c` and a shared header `c/func.h`, here's the `Makefile`:

```makefile
obj-m := multi_hello.o
multi_hello-objs := hello.o func.o

all:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

.PHONY: all clean
```

## Explanation:

`multi_hello-objs := hello.o func.o`: This line tells the kernel build system to compile both `hello.c` and `func.c` and then link the resulting object files into the final module `multi_hello.ko`.

When you run `make`, the kernel build system will compile both sources into their respective object files, then link them together to produce the `multi_hello.ko` module.

Load and verify:

```bash
sudo insmod ./multi_hello.ko
dmesg | tail      # shows both init and the "Hello Linux" message from func.c
sudo rmmod multi_hello
```

> **Fix vs. the original example:** the lesson text told you to "ensure that any header files or dependencies shared between hello.c and func.c are properly included" — but no header existed. This version adds `func.h` which declares `void func(void);` and is included by *both* source files, so the compiler cross-checks that the prototype and the definition always match. A bare function prototype declared inline in `hello.c` (as in the original) works but is fragile: if `func.c` is ever changed, the compiler cannot verify the two files agree.

## Rust version

In Rust, split a module into several files with `mod` submodules instead of linking separate objects. `rust/rust_multi.rs` declares `mod func;` and `rust/func.rs` implements it; both are compiled into a single crate object:

```makefile
obj-m := rust_multi.o

all:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) LLVM=1 modules

clean:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) LLVM=1 clean

.PHONY: all clean
```

Do **not** add `func.o` to the Makefile — `rustc` compiles the submodule together with the crate root into the one `rust_multi.o` object.

> **Note:** because `func.rs` is not a separate kbuild object, incremental rebuilds may not notice a change to it; run `make clean` (or `touch rust_multi.rs`) after editing submodules.

Build and verify:

```bash
sudo insmod ./rust_multi.ko
dmesg | tail      # shows RustMulti init + the greeting from func.rs
sudo rmmod rust_multi
```
