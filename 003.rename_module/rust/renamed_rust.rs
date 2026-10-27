// SPDX-License-Identifier: GPL-2.0
//! Lesson 003: renaming a Rust kernel module.
//!
//! In Rust for Linux the module's registered name comes from the `name:`
//! field of the `module!` macro below, and is registered as
//! `my_renamed_module`. The compiled object is derived from this file's
//! name (`renamed_rust.rs` -> `renamed_rust.o` -> `renamed_rust.ko`).

use kernel::prelude::*;

module! {
    type: RustRenamed,
    name: "my_renamed_module",
    author: "AlphaCortex-03",
    description: "Lesson 003: a Rust kernel module demonstrating module naming",
    license: "GPL",
}

struct RustRenamed;

impl kernel::Module for RustRenamed {
    fn init(_module: &'static ThisModule) -> Result<Self> {
        pr_info!("{} : In init\n", "RustRenamed");
        Ok(RustRenamed)
    }
}

impl Drop for RustRenamed {
    fn drop(&mut self) {
        pr_info!("{} : In exit\n", "RustRenamed");
    }
}
