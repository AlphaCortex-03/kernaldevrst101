# What happens when we run `insmod` on a module

## What is a kernel module?

A kernel module is a segment of kernel code that can be dynamically added to the running kernel upon loading. Similarly, it can be removed when its functionality is no longer needed.

## The `insmod` process

When we run `insmod` on a module, the system goes through several steps:

1. **Initial call**: The process starts with a syscall. Classic `insmod` used the `init_module()` syscall; modern loaders (and current `insmod`/`modprobe`) prefer `finit_module()`, which reads the module from a file descriptor instead of copying a user-space buffer. Both notify the kernel that there is a pending module load request.

2. **In-kernel operations**: Once in the kernel, the corresponding syscall handler (`sys_init_module` / `sys_finit_module`, i.e. `__do_sys_init_module` in modern kernels) is invoked. It:

   - First verifies the permissions (capabilities; `CAP_SYS_MODULE`) of the user trying to load the module.

   - Upon successful verification, the internal `load_module()` function is invoked. This function:

     - Allocates temporary memory and transfers the ELF module image into kernel memory (using `copy_from_user` for `init_module`, or the directly mapped file for `finit_module`).

     - Validates the integrity of the ELF file, ensuring it meets the expected structure and standards (architecture, version magic, signature policy…).

     - Based on the interpretation of the ELF file, layout offsets are generated within the allocated memory. These are termed the "convenience variables" of the load process.

     - User-supplied module arguments are also transferred to kernel memory and processed.

     - The process then performs symbol resolution against the running kernel's symbol table.

     - Upon completion, `load_module()` yields a reference to the kernel module structure.

   - This reference is integrated into a doubly linked list (the kernel's global module list) which chronicles all loaded modules in the system. The module also appears under `/sys/module/<name>`.

   - Lastly, the function registered via `module_init` inside the module code is invoked.

3. **On success**: `insmod` prints nothing and exits 0; the module is listed by `lsmod`.

# What Happens if the Init Function Returns an Error (e.g. -1)

The init function of a kernel module, registered via `module_init`, is the function that gets called when you insert the module into the kernel. It is used to set up any resources the module needs, such as allocating memory, initializing data structures, registering a device driver, etc.

Returning a value from the init function indicates the success or failure of the module initialization. In the Linux kernel, a return value of `0` indicates success, while a negative errno value indicates failure.

If the init function returns a negative error such as `-1` (which corresponds to `-EPERM`, "Operation not permitted"):

1. **Kernel module insertion fails**: The kernel rolls back the load and does not keep the module inserted (unlike `insmod`'s behaviour on some old kernels where a partially initialized module could linger, modern kernels tear it down). The module's functionality will not be available.

2. **Resource cleanup**: Resources that were allocated in the init function up to the point of failure should be released by the init function *itself* before returning the error — the kernel does not undo all of your steps for you. Failing to do so causes leaks that persist until reboot (well, until the load attempt's teardown frees the module skeleton).

3. **Kernel logging**: The failure is visible in the kernel log. When you run `dmesg`, you may see messages like `module: failed to load` / `insmod: ERROR: could not insert module ...: Operation not permitted`. The exact error text (`Operation not permitted` here) is the `strerror` of the errno returned by the init function — e.g. returning `-EINVAL` prints `Invalid module format`, `-ENOMEM` prints "Cannot allocate memory", etc.

4. **Dependencies**: Any subsequent modules that depend on the failed module will also fail to load or function correctly (if it were to have been loaded).

## Good practice

It is good practice to propagate meaningful errno codes (`-EINVAL`, `-ENOMEM`, ...) rather than a bare `-1`, and to log a detailed diagnostic with the appropriate log level (e.g. `pr_err(...)`) in your init function, to make the cause of failure easy to diagnose from `dmesg`.
