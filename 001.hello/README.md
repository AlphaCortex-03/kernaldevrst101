# 001: Hello World Kernel Module

In traditional C programming, `main()` serves both as the entry and exit points. Contrarily, kernel modules require at least two distinct functions:
- A "start" or initialization function, invoked when the module is loaded.
    - return value is `int`
- An "end" or cleanup function, invoked just before the module's removal.
    - return value is `void`

These are achieved using the `module_init()` and `module_exit()` macros (in the Rust version this role is played by the `module!` macro plus the `Module::init` implementation and `Drop`, see `rust/hello.rs`).

## Licensing

It's imperative for a module to declare its licensing type, accomplished using the `MODULE_LICENSE()` macro. Below are some common licenses:

- "GPL" - Represents the GNU Public License v2 or later versions.
- "GPL v2" - Specifically for the GNU Public License v2.
- "GPL and additional rights" - Offers rights of GNU Public License v2 and potentially more.
- "Dual BSD/GPL" - Provides an option between the GNU Public License v2 and the BSD license.
- "Dual MIT/GPL" - Gives a choice between the GNU Public License v2 and the MIT license.
- "Dual MPL/GPL" - An option to pick between the GNU Public License v2 and the Mozilla license.
- "Proprietary" - Used for non-free products (taints the kernel).

## Header Files

Every C kernel module mandates the inclusion of `linux/module.h`. This inclusion aids in the macro expansion of `module_init` and `module_exit`. The `linux/kernel.h` header brings in `printk` and its log levels. In modern code, `pr_info()` (from `linux/printk.h`, included via `kernel.h`) is preferred over `printk(KERN_INFO ...)`.

## How to write a Makefile for a kernel module

Given a kernel module source file named `hello.c`, here's a basic `Makefile` that would compile it:

```makefile
obj-m := hello.o

all:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	$(MAKE) -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

.PHONY: all clean
```

> **Fix vs. the original example:** the recipe now invokes `$(MAKE)` instead of calling `make` directly. Using the `$(MAKE)` variable is required so that GNU Make can correctly pass its job server and communication settings to the sub-make.

## Explanation:

### What is `obj-m`?

In the context of kernel module compilation, the variable `obj-m` specifies which object files are to be built as loadable kernel modules. When you set `obj-m := hello.o`, you're telling the kernel build system that `hello.c` (which compiles to `hello.o`) should be built as a module, not built into the kernel image.

- `obj-m := hello.o`: The `obj-m` directive tells the kernel build system that you want to create a module ("objects loaded as modules", as opposed to `obj-y` which builds code into the kernel itself). The source file `hello.c` gets compiled into an object file named `hello.o`, which then gets linked to produce the kernel module `hello.ko`.

- `all`: This is the default rule. When you run `make` in your directory, this rule is what gets executed.

  - `-C /lib/modules/$(shell uname -r)/build`: This changes the directory to the kernel build directory for the currently running kernel. This is where the kernel source and configuration for the currently running kernel are found on most systems.

  In the context of the Linux kernel's Makefile and many other Makefiles, the `-C` option is used with `make` to change to a specified directory before reading the makefiles or performing any build actions.

  When using the `-C` option, `make` will first change to the specified directory (`/path/to/directory` in your example) and then it will look for the Makefile in that directory to execute it.

  In the context of the Linux kernel, you might see commands like:
  ```
  make -C /lib/modules/$(uname -r)/build M=$(PWD) modules
  ```
  This command essentially tells `make` to change to the kernel build directory (often where the kernel headers and configuration for the running kernel are stored) and then execute the build from there using the current directory (`$(PWD)`) as the module source.

  - `M=$(PWD)`: This tells the kernel make system to come back to your module's directory (the current directory) to build the module.
  - `M`: This is a make variable used by the kernel's build system. When building external modules, you set `M` to the path of the directory containing your module's source files.
  - `$(PWD)`: This is a variable that evaluates to the current working directory. PWD stands for "present working directory".

  - `modules`: This is the actual target for building modules. When you specify "modules" as a target, the kernel build system knows that you intend to build modules, not another kind of target.

## Why use a Makefile:

1. **Simplification**: By using a `Makefile`, you encapsulate the details of the build process in one place. This makes it easy to compile the module by just running `make` instead of remembering and typing out the full compilation command.

2. **Consistency**: It ensures that the module is always built the same way, preventing inconsistencies and errors that can arise from manually typing in the build commands.

3. **Flexibility**: As your module grows, you might add more source files, dependencies, or even additional modules. A `Makefile` can easily handle such complexities, ensuring all parts of your project are properly built.

4. **Integration with kernel build system**: Kernel modules need to be built against the kernel source and configuration that they're intended to be loaded into. The kernel's build system provides a lot of functionality for building modules, and using a `Makefile` is the easiest way to take advantage of this functionality.

In summary, a `Makefile` greatly simplifies and standardizes the process of building kernel modules, making development quicker and less error-prone.

## Load the kernel module into the kernel

### Syntax:
```bash
sudo insmod ./<module name>
```
### Usage
```
sudo insmod ./hello.ko        # C
sudo insmod ./rust_hello.ko   # Rust
```

```bash
lsmod
```

Use `lsmod` to check the module has been loaded into the kernel or not.

![](../images/Screenshot%20from%202023-08-16%2001-10-33.png)

Navigate to `/sys/module`; a module named `hello` (or `rust_hello`) should have been added.

![](../images/Screenshot%20from%202023-08-16%2001-15-58.png)

## Check kernel messages

The program logs as follows:

```C
    pr_info("%s : In init\n", __func__);
```
To monitor the output, run `dmesg`.

### When insmod is executed

![](../images/Screenshot%20from%202023-08-16%2001-22-44.png)

### While rmmod is executed

![](../images/Screenshot%20from%202023-08-16%2001-26-16.png)

## Cross-compiling

Cross-compiling refers to the process of building code for one platform (the target) on a different platform (the host).
- This is particularly useful when the target platform has limited resources.
- For instance, if you're developing for an embedded system, it might not have the necessary resources to compile code.
- In such cases, you'd use a more powerful host system (like a PC) to compile the code, and then transfer the compiled binary to the target system.

To cross-compile, you typically need:

1. **A cross-compiler:** a compiler that generates binaries for the target architecture, usually with a prefix indicating the target architecture, like `aarch64-linux-gnu-` for 64-bit ARM.

2. **Libraries and headers for the target system:** these might be different from those on the host system.

3. A way to transfer the compiled binary to the target system, such as `scp`, `rsync`, or physical media.

Modify the Makefile like this to cross-compile:

```makefile
CROSS_COMPILE ?= aarch64-linux-gnu-
ARCH ?= arm64

obj-m := hello.o

all:
	$(MAKE) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C /path/to/target/kernel/source M=$(PWD) modules

clean:
	$(MAKE) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C /path/to/target/kernel/source M=$(PWD) clean

.PHONY: all clean
```

## `printf` vs `printk`

Both `printf` and `printk` are functions used to display messages, but they have distinct purposes and are used in different contexts.

### `printf`:
- **Definition**: `printf()` is a function found in the C Standard Library.
- **Purpose**: Used in user-space programs to display formatted strings.
- **Output**: Writes to the standard output, typically the console or terminal where the program runs.

### `printk`:
- **Definition**: `printk()` (and the `pr_info()`/`pr_err()` wrappers) are kernel-level functions used in the Linux kernel.
- **Purpose**: Provides logging and debugging capabilities for kernel space. Necessary because the kernel does not have access to standard C library functions.
- **Output**: Writes messages to the kernel log buffer. They can later be fetched using commands like `dmesg`.
- **Syntax**: `printk` is called with an additional argument representing the log priority:

  ```c
  printk(KERN_log_priority "formatted string\n");
  ```

## Log Priorities
These are defined in the kernel headers and determine the importance/severity of the message. They are:

- `KERN_EMERG`
- `KERN_ALERT`
- `KERN_CRIT`
- `KERN_ERR`
- `KERN_WARNING`
- `KERN_NOTICE`
- `KERN_INFO`
- `KERN_DEBUG`

Each log priority value helps in categorizing kernel messages. For instance, error messages can be distinguished from general informational messages.

## Key Difference
- `printk()` writes messages to the kernel's ring buffer, a cyclical buffer in memory for storing log messages. On the other hand, `printf()` writes messages to the standard output.
- `printk()` allows setting a log priority which helps in filtering and organizing kernel messages. `printf()` lacks this capability.

In Rust modules, `pr_info!()` plays the exact same role as `pr_info()` on the C side (see `rust/hello.rs`).

Utilizing these functions appropriately is crucial for effective debugging and logging, whether you're in user-space or kernel-space development.
