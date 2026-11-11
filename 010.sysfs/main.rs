// SPDX-License-Identifier: GPL-2.0
//! 010.sysfs - Rust twin of main.c.
//!
//! Rust note: the ~6.8-era Rust bindings do not expose a raw
//! kobject/sysfs attribute API yet. The idiomatic Rust way to expose
//! ONE tunable value today is a module parameter with non-zero
//! `permissions`: the kernel then creates a sysfs file for it too, and
//! with 0o644 the value can even be changed at runtime by root -
//! exactly the "one value per file, for config" pattern of sysfs.
//!
//! So this Rust version exposes the same `value` under
//! /sys/module/mysysfs_rs/parameters/value instead of
//! /sys/kernel/my_mod/value. Use main.c when you want the
//! kobject-based file from the test commands.

use kernel::prelude::*;

module! {
    type: MySysfs,
    name: "mysysfs_rs",
    author: "Gopal",
    description: "010: sysfs-exposed state (Rust, via param permissions)",
    license: "GPL",
    params: {
        value: {
            type: i32,
            default: 42,
            permissions: 0o644, // writable through sysfs at runtime
            description: "State exposed under /sys (one value per file)",
        },
    },
}

struct MySysfs;

impl KernelModule for MySysfs {
    fn init(_module: &'static ThisModule) -> Result<Self> {
        pr_info!(
            "mysysfs_init: value = {} (see /sys/module/mysysfs_rs/parameters/value)\n",
            *module_params::value.read()
        );
        Ok(MySysfs)
    }
    // Rust note: no exit hook needed - the sysfs file is owned by the
    // parameter machinery and disappears with the module automatically.
}
