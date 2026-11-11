// SPDX-License-Identifier: GPL-2.0
//! 007.char_device - Rust twin of main.c.
//!
//! The C version builds a `struct file_operations` table by hand and
//! registers a `struct miscdevice`. The Rust bindings wrap both into
//! one trait: `MiscDevice`. The kernel calls your trait methods in
//! exactly the same places it would call the C callbacks.
//!
//! NOTE: Rust-for-Linux is not in mainline yet and its API changes
//! between kernel releases. This file targets the ~6.8 rust-next API
//! (`module_misc_device!` + `UserSlicePtrReader/Writer`); on older or
//! newer trees some names may differ slightly.

use kernel::{
    fs::File,
    miscdevice::{MiscDevice, UserSlicePtrReader, UserSlicePtrWriter},
    prelude::*,
};

// The macro does what misc_register() + module_init/module_exit do in C.
module_misc_device! {
    type: MyDevice,
    name: "mydevice_rs", // Rust note: on 6.6-era trees write this as b"mydevice_rs"
    author: "Gopal",
    description: "007: minimal char device (Rust)",
    license: "GPL",
}

// No per-open state in this lesson, so the struct is empty.
// (Lesson 008 adds real data, which needs a Mutex in Rust.)
struct MyDevice;

impl MiscDevice for MyDevice {
    type OpenData = ();

    // C: .open
    fn open(_file: &File, _data: &Self::OpenData) -> Result<Pin<KBox<Self>>> {
        pr_info!("my_open: device opened\n");
        // Rust note: state is heap-allocated by the kernel and pinned -
        // the kernel keeps the object at a stable address.
        Ok(KBox::pin_init(MyDevice {}, GFP_KERNEL))
    }

    // C: .write (userspace -> kernel)
    fn write(&self, _file: &File, reader: &mut UserSlicePtrReader) -> Result<usize> {
        pr_info!("my_write: userspace sent {} bytes (ignored for now)\n", reader.len());
        // Rust note: there is NO raw __user pointer access here. The
        // `reader` IS the safe copy_from_user(): its memory can only be
        // read through methods that validate the userspace address.
        // Lesson 008 actually stores the data.
        Ok(reader.len())
    }

    // C: .read (kernel -> userspace)
    fn read(&self, _file: &File, _writer: &mut UserSlicePtrWriter) -> Result<usize> {
        pr_info!("my_read: nothing to give back yet\n");
        Ok(0) // 0 == EOF, same convention as C
    }

    // C: .release
    fn release(_this: Pin<KBox<Self>>, _file: &File) {
        pr_info!("my_release: device closed\n");
    }
}
