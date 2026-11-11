// SPDX-License-Identifier: GPL-2.0
//! 012.memory - Rust twin of main.c.
//!
//! Rust differences to the C version:
//! - There are no raw kmalloc/kzalloc calls: `KBox::new(value, flags)`
//!   allocates from the kernel heap with GFP flags (kmalloc-backed).
//! - Every Rust allocation takes an initial value, so "uninitialized
//!   garbage" (the kmalloc lesson) cannot happen by accident -
//!   kzalloc is simply `KBox::new(0, ...)`.
//! - `KVBox` is the vmalloc-backed counterpart: a big, virtually
//!   contiguous allocation.
//! - There is nothing to free: the allocations are owned by the module
//!   struct, and the kernel frees them automatically when the module
//!   is unloaded (no kfree/vfree to forget = no leaks by design).
//!
//! NOTE: targets the ~6.8 rust-next API; names may differ between trees.

use kernel::{alloc::flags, prelude::*};

module! {
    type: MyMemory,
    name: "mymemory_rs",
    author: "Gopal",
    description: "012: kmalloc vs kzalloc vs vmalloc (Rust)",
    license: "GPL",
}

struct MyMemory {
    kmalloc_mem: KBox<u8>,
    kzalloc_mem: KBox<u8>,
    vmalloc_mem: KVBox<u8>,
    atomic_mem: KBox<u8>,
}

impl KernelModule for MyMemory {
    fn init(_module: &'static ThisModule) -> Result<Self> {
        pr_info!("mymemory_init: allocating four buffers\n");

        // 1) kmalloc equivalent (GFP_KERNEL, physically contiguous).
        let kmalloc_mem = KBox::new(0x41, flags::GFP_KERNEL)?;
        pr_info!(
            "mymemory_init: kmalloc : Rust KBox + GFP_KERNEL (value=0x{:02x})\n",
            *kmalloc_mem
        );

        // 2) kzalloc equivalent: same allocator, zeroed value.
        let kzalloc_mem = KBox::new(0, flags::GFP_KERNEL)?;
        pr_info!(
            "mymemory_init: kzalloc : KBox + GFP_KERNEL, value=0x{:02x} (zeroed)\n",
            *kzalloc_mem
        );

        // 3) vmalloc equivalent: big allocation backed by scattered
        //    physical pages, contiguous in virtual address space.
        let vmalloc_mem = KVBox::new(0, flags::GFP_KERNEL)?;
        pr_info!("mymemory_init: vmalloc : Rust KVBox (vmalloc-backed)\n");

        // 4) GFP_ATOMIC: never sleeps - for interrupt context. It uses
        //    a smaller emergency pool and fails more often; the `?`
        //    makes callers handle that failure instead of ignoring it.
        let atomic_mem = KBox::new(0, flags::GFP_ATOMIC)?;
        pr_info!("mymemory_init: GFP_ATOMIC: never sleeps (IRQ/timer context)\n");

        Ok(MyMemory {
            kmalloc_mem,
            kzalloc_mem,
            vmalloc_mem,
            atomic_mem,
        })
    }
    // Rust note: no exit hook needed - dropping MyMemory frees all four
    // allocations with the correct allocator automatically.
}
