// SPDX-License-Identifier: GPL-2.0
//! A minimal "Hello, World" Linux kernel module written in Rust.
//!
//! Requires Linux 6.1+ built with `CONFIG_RUST=y`.

use kernel::prelude::*;

module! {
    type: RustHello,
    name: "rust_hello",
    author: "AlphaCortex-03",
    description: "A minimal hello world kernel module written in Rust",
    license: "GPL",
}

struct RustHello;

impl kernel::Module for RustHello {
    fn init(_module: &'static ThisModule) -> Result<Self> {
        pr_info!("{} : In init\n", "RustHello");
        Ok(RustHello)
    }
}

impl Drop for RustHello {
    fn drop(&mut self) {
        pr_info!("{} : In exit\n", "RustHello");
    }
}
