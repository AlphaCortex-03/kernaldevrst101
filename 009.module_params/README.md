# 009.module_params

A module can receive values when you load it, instead of hardcoding
them in the source. `insmod` takes `key=value` pairs after the `.ko`
file. Every declared parameter also appears as a file under
`/sys/module/<module>/parameters/`.

## What

- Accepts 4 parameters at load time:
  - `myname` (string), `count` (int), `myarray` (comma-separated int
    array), `debug` (bool)
- Greets `myname` `count` times in dmesg
- Registers them read-only (mode 0444) in sysfs

## What you learn

- `module_param()` / `module_param_array()` / `MODULE_PARM_DESC()`
- What `charp` means (pointer to a userspace-supplied C string)
- The sysfs file mode meanings: `0` = no file, `0444` = read-only,
  `0644` = changeable at runtime by root
- Where `/sys/module/<module>/parameters/` lives

## Files

| File       | Purpose                                |
| ---------- | -------------------------------------- |
| `main.c`   | C module with the four parameters      |
| `main.rs`  | Rust twin (`params` section of `module!`) |
| `Makefile` | Wrapper that invokes the kernel build  |
| `Kbuild`   | The real build rules (`obj-m`)         |
| `README.md`| This file                              |

## How to build

```bash
cd 009.module_params
make
```

## How to test

```bash
sudo insmod myparams.ko myname=Gopal count=5 myarray=1,2,3 debug=1
dmesg | tail

# the same values as read-only sysfs files:
ls /sys/module/myparams/parameters/
cat /sys/module/myparams/parameters/myname   # Gopal
cat /sys/module/myparams/parameters/count    # 5

# read-only for normal users -> changing needs a reload:
echo 99 > /sys/module/myparams/parameters/count   # Permission denied

sudo rmmod myparams
```

Tip: `modinfo myparams.ko` lists every parameter together with its
`MODULE_PARM_DESC` text.

## Expected output

```
myparams_init: loaded with parameters:
myparams_init:   myname  = "Gopal"
myparams_init:   count   = 5
myparams_init:   myarray = [1, 2, 3] (count=3)
myparams_init:   debug   = true
myparams_init: hello #0, Gopal!
myparams_init: hello #1, Gopal!
myparams_init: hello #2, Gopal!
myparams_init: hello #3, Gopal!
myparams_init: hello #4, Gopal!
myparams_init: debug output enabled
```

## Common bug + fix

**Bug:** `sudo insmod myparams.ko myname=Gopal count=5` fails with
`Invalid module format` or silently keeps the default values.

**Cause:** typo in the parameter name. Unknown parameters make insmod
reject the module (or, with a leading dash prefix, get ignored) - the
kernel matches names exactly, and `Count=5` is not `count=5`.

**Fix:** check exact names with `modinfo -p myparams.ko`. Note that
array params must be comma-separated (`myarray=1,2,3`), not
space-separated, and a wrong value type (`count=abc`) also makes insmod
fail with `Invalid module format`.

## Rust version (main.rs)

- All parameter declaration lives in the `params` section of `module!`
- `charp` maps to `str`, and the sysfs file mode is written in octal
  (`0o444`) instead of `0444`
- The int-array parameter has no Rust binding yet in the ~6.8-era
  trees - that part is C-only (see `main.c`)
- Same API-version caveat as lesson 007 (needs a `CONFIG_RUST=y`
  kernel such as `linux-rust` on Arch)
