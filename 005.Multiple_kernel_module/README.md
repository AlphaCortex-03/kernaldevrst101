# 005: Building a Kernel Module with Multiple Source Files (and Multiple Modules)

## One module from multiple source files

If you have multiple source files, such as `hello.c` and `func.c`, and you want them compiled and linked together into a single kernel module, use a `Makefile` like this (same technique as lesson 004):

```makefile
obj-m := hello1.o
hello1-objs := func.o hello.o

all:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

.PHONY: all clean
```

This produces a single `hello1.ko` containing both objects.

## Several modules from one directory

If you want to generate two kernel modules, say `module1.ko` and `module2.ko`, from two different source files, `module1.c` and `module2.c`, list both in `obj-m`:

```makefile
obj-m := module1.o module2.o

all:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

.PHONY: all clean
```

## Rust equivalent

For Rust modules you can select several crates in one go:

```makefile
obj-m := module1_rust.o module2_rust.o

all:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) LLVM=1 modules

clean:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) LLVM=1 clean

.PHONY: all clean
```

Each `moduleN_rust.o` is built from its corresponding `moduleN_rust.rs` crate root. Submodule (helper) `.rs` files are compiled into their crate via `mod` declarations, as shown in lesson 004 — they are *not* listed in `obj-m`.
