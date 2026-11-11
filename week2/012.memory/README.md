# 012.memory

A tour through the kernel's allocators: `kmalloc`, `kzalloc`,
`vmalloc`, and the two most important GFP flags. The module allocates
four buffers on load, logs each one to dmesg, and frees everything on
unload - zero leaks.

## What

- `kmalloc(64, GFP_KERNEL)` - fast, physically contiguous, garbage
  contents (we write a `'K'` pattern into it)
- `kzalloc(64, GFP_KERNEL)` - kmalloc + zeroed (byte 0 is 0x00)
- `vmalloc(256*1024)` - 256 KB, virtually contiguous, physically
  scattered pages glued by the MMU (we write a `'V'` pattern)
- `kmalloc(32, GFP_ATOMIC)` - the never-sleeps variant
- `kfree`/`vfree` for each in `exit`

## What you learn

- kmalloc vs kzalloc: same speed, only difference is zeroing - use
  kzalloc instead of kmalloc + memset(0)
- kmalloc vs vmalloc: kmalloc = physically contiguous but small
  (a few MB max); vmalloc = large but slow, and never for DMA
- GFP_KERNEL: may sleep (wait/reclaim memory) - process context only
- GFP_ATOMIC: never sleeps, smaller emergency pool, fails more often -
  for interrupt/timer context; failures MUST be handled
- The matching-free rule: `kfree()` for kmalloc/kzalloc, `vfree()`
  for vmalloc

## Files

| File       | Purpose                                          |
| ---------- | ------------------------------------------------ |
| `main.c`   | C module doing the four allocations              |
| `main.rs`  | Rust twin (`KBox` / `KVBox` + GFP flags)         |
| `Makefile` | Wrapper that invokes the kernel build            |
| `Kbuild`   | The real build rules (`obj-m`)                   |
| `README.md`| This file                                        |

## How to build

```bash
cd 012.memory
make
```

## How to test

```bash
sudo insmod mymemory.ko
dmesg | tail -n 8
sudo rmmod mymemory
dmesg | tail -n 2
```

## Expected output

```
mymemory_init: allocating four buffers
mymemory_init: kmalloc(64, GFP_KERNEL)      = ffff88800xxxxxxx, byte[0]=0x4b (contents NOT guaranteed zero!)
mymemory_init: kzalloc(64, GFP_KERNEL)      = ffff88800xxxxxxx, byte[0]=0x00 (always zeroed)
mymemory_init: vmalloc(256*1024)            = ffffc90000xxxxxx (virtual range, physically scattered pages)
mymemory_init: kmalloc(32, GFP_ATOMIC)      = ffff88800xxxxxxx (never sleeps)
mymemory_init: GFP_KERNEL may sleep & reclaim; GFP_ATOMIC never sleeps
mymemory_exit: freed everything - no leaks
```

Note the kmalloc byte[0]: we wrote `0x4b` ('K') into it, but the point
is that *before writing*, its contents were garbage - unlike the
kzalloc buffer, which is guaranteed 0x00.

## Common bug + fix

**Bug:** freeing the vmalloc buffer with `kfree(vm_mem)` (or the
reverse: `vfree()` on kmalloc memory). On VMware this instantly gives a
kernel BUG/oops like "BUG: unable to handle page fault" or an
`objfree` warning, because kfree expects a slab object, not a
page-table-built virtual range.

**Fix:** free with the matching function - `kfree()` for anything from
`kmalloc`/`kzalloc`/`kzalloc`-family, `vfree()` for anything from
`vmalloc`/`vzalloc`. Keep the pointer's "who allocated me" obvious
(different variable names, as in this lesson). Bonus rule: `kfree(NULL)`
and `vfree(NULL)` are legal no-ops, which is why the error path here
can free everything unconditionally.

## Rust version (main.rs)

- `KBox::new(value, flags)` = kmalloc-backed allocation; taking an
  initial value means kzalloc is just `KBox::new(0, ...)`, and
  uninitialized garbage cannot leak into your program by accident
- `KVBox` = vmalloc-backed allocation
- `flags::GFP_KERNEL` / `flags::GFP_ATOMIC` - with `?` on the call, a
  failing GFP_ATOMIC allocation is a *handled* error, not a NULL check
  you can forget
- No `kfree`/`vfree` anywhere: the module struct owns the allocations
  and the kernel drops them automatically on unload (leak-free by
  design)
- Same API-version caveat as lesson 007 (targets ~6.8 rust-next, needs
  a `CONFIG_RUST=y` kernel such as `linux-rust` on Arch)
