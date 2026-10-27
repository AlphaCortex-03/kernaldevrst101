// SPDX-License-Identifier: GPL-2.0
//! Lesson 004: a multi-file Rust kernel module.
//!
//! `func.rs` is pulled in through the `mod func;` declaration below.
//! The kernel build system compiles the whole crate (rust_multi.rs plus
//! its submodules) into the single rust_multi object that `obj-m` asks for.

mod func;

use kernel::prelude::*;

module! {
    type: RustMulti,
    name: "rust_multi",
    author: "AlphaCortex-03",
    description: "Lesson 004: a multi-file kernel module written in Rust",
    license: "GPL",
}

struct RustMulti;

impl kernel::Module for RustMulti {
    fn init(_module: &'static ThisModule) -> Result<Self> {
        pr_info!("{} : In init\n", "RustMulti");
        func::greet();
        Ok(RustMulti)
    }
}

impl Drop for RustMulti {
    fn drop(&mut self) {
        pr_info!("{} : In exit\n", "RustMulti");
    }
}
