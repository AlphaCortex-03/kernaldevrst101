// SPDX-License-Identifier: GPL-2.0
//! Lesson 004: helper submodule included by the multi-file Rust kernel module.

use kernel::prelude::*;

pub fn greet() {
    pr_info!("Hello Linux (from func.rs)\n");
}
