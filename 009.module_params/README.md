# 009: Module Parameters

A module can receive values when you load it, instead of hardcoding them in the source. `insmod` takes `key=value` pairs after the `.ko` file. Every declared parameter also appears as a file under `/sys/module/<module>/parameters/`.

## What the module does

- Accepts 4 parameters at load time:
  - `myname` (string), `count` (int), `myarray` (comma-separated int array), `debug` (bool)
- Greets `myname` `count` times in dmesg
- Registers them read-only (mode 0444) in sysfs

## How the parameters are declared

```c
static char *myname = "nobody";   /* charp = pointer to a C string */
static int count = 1;
static int myarray[8];
static int myarray_len;
static bool debug = false;

module_param(myname, charp, 0444);
MODULE_PARM_DESC(myname, "Name to greet (string)");
module_param_array(myarray, int, &myarray_len, 0444);
```

The last argument of `module_param()` is the mode of the sysfs file:

- `0` - no file in `/sys` at all
- `0444` - file exists, read-only for normal users (this lesson)
- `0644` - root may write the file to change the value at runtime

`MODULE_PARM_DESC` is documentation; it shows up in `modinfo`.

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

Tip: `modinfo -p myparams.ko` lists every parameter with its description.

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

**Bug:** `sudo insmod myparams.ko myname=Gopal count=5` fails with `Invalid module format` or silently keeps the default values.

**Cause:** typo in the parameter name. The kernel matches names exactly, and `Count=5` is not `count=5`.

**Fix:** check exact names with `modinfo -p myparams.ko`. Array params must be comma-separated (`myarray=1,2,3`), not space-separated, and a wrong value type (`count=abc`) also makes insmod fail.

## Rust version (main.rs)

All parameter declaration lives in the `params` section of the `module!` macro: `charp` maps to `str` and the file mode is written in octal (`0o444`). The int-array parameter has no Rust binding yet in the ~6.8-era trees, so that part stays C-only. Same API-version caveat as lesson 007 (needs a `CONFIG_RUST=y` kernel such as `linux-rust` on Arch).
