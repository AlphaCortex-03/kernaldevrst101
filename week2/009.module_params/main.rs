// SPDX-License-Identifier: GPL-2.0
//! 009.module_params - Rust twin of main.c.
//!
//! In C you declare a variable + module_param() + MODULE_PARM_DESC().
//! In Rust all three live in one place: the `params` section of the
//! `module!` macro. Each parameter also appears under
//! /sys/module/myparams_rs/parameters/, exactly like the C version.
//!
//! Rust note: int-array parameters (myarray) have no Rust binding in
//! the ~6.8-era trees yet - see main.c for that part.
//!
//! NOTE: targets the ~6.8 rust-next API; parameter accessor paths have
//! changed between RfL releases.

use kernel::prelude::*;

module! {
    type: MyParams,
    name: "myparams_rs",
    author: "Gopal",
    description: "009: module parameters (Rust)",
    license: "GPL",
    params: {
        // charp in C == `str` here.
        myname: {
            type: str,
            default: b"nobody",
            permissions: 0o444, // read-only, like 0444 in C
            description: "Name to greet (string)",
        },
        count: {
            type: i32,
            default: 1,
            permissions: 0o444,
            description: "How many times to greet",
        },
        // bool in C == `bool` here (accepts debug=1 / =y / =true).
        debug: {
            type: bool,
            default: false,
            permissions: 0o444,
            description: "Enable extra debug output",
        },
    },
}

struct MyParams;

impl KernelModule for MyParams {
    fn init(_module: &'static ThisModule) -> Result<Self> {
        pr_info!("myparams_init: loaded with parameters:\n");
        pr_info!("myparams_init:   myname  = \"{}\"\n", module_params::myname.read());
        pr_info!("myparams_init:   count   = {}\n", module_params::count.read());
        pr_info!("myparams_init:   debug   = {}\n", module_params::debug.read());

        // Rust note: accessors are version-sensitive; on some trees you
        // read a guard and deref it: *module_params::count.read()
        let count = module_params::count.read();
        for i in 0..count {
            pr_info!("myparams_init: hello #{}, {}!\n", i, module_params::myname.read());
        }

        if module_params::debug.read() {
            pr_info!("myparams_init: debug output enabled\n");
        }

        Ok(MyParams)
    }
}
