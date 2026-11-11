# 010: sysfs

sysfs is the kernel's "one value per file" configuration/state tree. This module creates `/sys/kernel/my_mod/value`: reading the file calls your `show()` function, writing it calls your `store()` function.

## What the module does

- Creates the kobject directory `/sys/kernel/my_mod`
- Creates the attribute file `value` (default 42, mode 0644)
- `cat` reads the value via `show()`, `echo 99` sets it via `store()`

## sysfs vs procfs vs debugfs

| FS       | Purpose                              | Example                       |
| -------- | ------------------------------------ | ----------------------------- |
| `sysfs`  | **one value per file**, for config and state; the stable, documented interface | `/sys/kernel/my_mod/value`, `/sys/class/*` |
| `procfs` | process information and some legacy kernel state; often human-readable multi-line streams | `/proc/cpuinfo`, `/proc/meminfo` |
| `debugfs`| developer debugging, no rules, no stability guarantees, usually mounted at `/sys/kernel/debug` | `/sys/kernel/debug/*` |

Rule of thumb: exporting a knob or a status number -> **sysfs**. Reporting processes -> procfs. Quick and dirty dumps for yourself -> debugfs (never for production interfaces).

## How show() and store() work

```c
static ssize_t value_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
        return sysfs_emit(buf, "%d\n", value);   /* fill the page buffer */
}

static ssize_t value_store(struct kobject *kobj, struct kobj_attribute *attr,
                           const char *buf, size_t count)
{
        int ret = kstrtoint(buf, 0, &new_value); /* safe string->int */
        if (ret)
                return ret;                      /* refuse junk input */
        value = new_value;
        return count;                            /* bytes consumed */
}
```

`__ATTR(name, mode, show, store)` wires the functions to the file. (`DEVICE_ATTR` is for `struct device` in the driver model; we attach to a plain kobject, so we use `__ATTR`.)

## How to build

```bash
cd 010.sysfs
make
```

## How to test

```bash
sudo insmod mysysfs.ko
cat /sys/kernel/my_mod/value         # 42
echo 99 > /sys/kernel/my_mod/value   # store() runs
cat /sys/kernel/my_mod/value         # 99
dmesg | tail
sudo rmmod mysysfs                   # directory disappears
```

## Expected output

```
mysysfs_init: /sys/kernel/my_mod/value ready (value=42)
value_store: value set to 99
mysysfs_exit: /sys/kernel/my_mod removed
```

## Common bug + fix

**Bug:** `echo abc > /sys/kernel/my_mod/value` crashes the kernel, or `store()` silently accepts garbage.

**Cause:** parsing with `sscanf`/`atoi`-style code that trusts the input, or returning the wrong length from `store()`.

**Fix:** parse with `kstrtoint()` and check its return value; on failure return the error so the write fails cleanly. Always return the number of bytes consumed (`count`) on success - returning 0 makes the shell retry the write forever.

## Rust version (main.rs)

The ~6.8-era Rust bindings have no raw kobject/sysfs attribute API yet, so the Rust version exposes the same `value` as a module parameter with `permissions: 0o644`, which creates `/sys/module/mysysfs_rs/parameters/value` - readable and (for root) writable at runtime, the same "one value per file" idea. For the kobject-based file use `main.c`. Same API-version caveat as lesson 007.
