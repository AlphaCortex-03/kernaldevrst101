# Kernel Development 101 (C & Rust)

This repository is a beginner-friendly introduction to Linux kernel module development.
Every example is provided in **two languages**: C and Rust (Rust-for-Linux, kernel 6.1+ with `CONFIG_RUST=y`).

## What is a Device Driver?

A device driver, often simply referred to as a 'driver,' is a software component that facilitates interaction between the computer's OS and a hardware device.

```
+----------+     +----------+     +----------+
|   User   |     |  Kernel  |     | Hardware |
+----------+     +----------+     +----------+
     ^                ^                ^
     |                |                |
User Interface   Kernel Interface   Hardware Interface
```

A device driver essentially serves as the mediator with:
- One side communicating with the kernel
- One side interfacing with the hardware
- One side interacting with the user

## What is a Kernel Module?

Traditionally, to add functionality to the kernel, it had to be recompiled and the system rebooted. Kernel Modules change this by allowing code to be dynamically loaded and unloaded from the kernel on demand.

### Terminology:
- **Loadable Kernel Modules (LKM):** Another name for Kernel Modules.
- **Extension:** `.ko` which stands for Kernel Object.

### Standard Location:
By default, modules are stored in `/lib/modules/<kernel version>` on the root file system.

![](images/1.png)

![](images/Screenshot%20from%202023-08-15%2020-50-48.png)

## Device Driver vs Kernel Modules:
While every device driver is a kernel module, not all kernel modules are device drivers. Kernel modules serve various purposes, including:
1. Device Drivers
2. File Systems
3. System Calls
4. Network drivers (e.g., those implementing TCP/IP)
5. TTY line disciplines (for terminal devices)

## Advantages:
1. Memory efficient as they can be loaded/unloaded on demand.
2. No need to reboot after every modification.
3. Bugs in modules don't necessarily halt the system.
4. Easier debugging and maintenance.
5. Simplified management for multiple machines.

## Disadvantages:
1. Consumes more memory due to module management.
   - Module management consumes unpageable kernel memory.
   - A basic kernel with a number of modules loaded will consume more memory than an equivalent kernel with the drivers compiled into the kernel image itself.
2. Modules load late in the boot process, so essential features must be in the base kernel.
   - All core functionality should be in the base kernel.
3. Dynamic loading widens the attack surface: an attacker who gains root can `insmod` a malicious module, which is one reason some hardened systems disable module loading entirely.

## Configuration:
To support modules, the kernel should have the `CONFIG_MODULES=y` option enabled.

![](images/Screenshot%20from%202023-08-15%2021-10-32.png)

For **Rust** modules, the kernel additionally needs `CONFIG_RUST=y` (available since Linux 6.1, requires `rustc` and LLVM/Clang toolchain support).

### Types:
1. **In-Tree:** Present within the Linux Kernel Source Code.
2. **Out-of-Tree:** Absent from the Linux Kernel Source Code, though they can eventually become in-tree modules.

### Basic Commands:
- **List Modules:** `lsmod` (information derived from `/sys/module`, which wraps the kernel's internal list of loaded modules).

![](images/Screenshot%20from%202023-08-15%2020-45-45.png)

- Gives the size of each module.
- `Used by` explains dependencies (how many other loaded modules/users hold a reference to this module).

- **Module Information:** `modinfo` provides detailed information about a module.

![Module information for module ip6_tables](images/Screenshot%20from%202023-08-15%2021-14-47.png)

![Module information for module cec](images/Screenshot%20from%202023-08-15%2021-16-51.png)

Notes:
- `parm`: acceptable parameter for the module.
- `intree`: whether it is an in-tree module.

## Repository Structure

| Folder | Lesson | C | Rust |
|--------|--------|---|------|
| `001.hello` | Minimal Hello World module | ✔ | ✔ |
| `002.insmod_working` | What happens when you run `insmod` (docs only) | ✖ | ✖ |
| `003.rename_module` | Change the output module name | ✔ | ✔ |
| `004.Multiple_files` | Module built from multiple source files | ✔ | ✔ |
| `005.Multiple_kernel_module` | Building several modules from one Makefile (docs) | ✖ | ✖ |
| `006.dmesg` | Debugging module output with `dmesg` (docs) | ✖ | ✖ |

Each lesson folder contains a `c/` and/or `rust/` sub-folder with its own `Makefile` and README.

## Prerequisites

**C modules (any kernel):**
```bash
sudo apt install build-essential linux-headers-$(uname -r)
```

**Rust modules (Linux 6.1+ with CONFIG_RUST=y):**
```bash
# Recent rustup toolchain and LLVM/clang
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh
sudo apt install clang llvm
```

Rust modules currently require LLVM=1 when building (see the Makefiles inside `*/rust/`).

---

With this foundation, you're ready to delve deeper into the fascinating world of kernel programming.
