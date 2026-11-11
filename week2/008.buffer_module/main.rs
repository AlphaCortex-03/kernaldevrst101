// SPDX-License-Identifier: GPL-2.0
//! 008.buffer_module - Rust twin of main.c.
//!
//! Same data flow as the C version: userspace bytes are copied into a
//! kernel-side buffer on write, and back out on read.
//!
//! Rust differences to the C version:
//! - No raw `char kbuf[1024]`: we use `KVec<u8>` (a kernel-heap vector)
//!   guarded by a `Mutex`, because the device can be opened by several
//!   processes at the same time.
//! - There is no copy_from_user()/copy_to_user() to misuse: the
//!   `UserSlicePtrReader`/`Writer` ARE the safe copies, enforced by the
//!   type system.
//!
//! NOTE: targets the ~6.8 rust-next API; names may differ between trees.

use kernel::{
    fs::File,
    miscdevice::{MiscDevice, UserSlicePtrReader, UserSlicePtrWriter},
    prelude::*,
    sync::{new_mutex, Mutex},
};

module_misc_device! {
    type: MyBuf,
    name: "mybuf_rs",
    author: "Gopal",
    description: "008: kernel buffer behind the device (Rust)",
    license: "GPL",
}

struct MyBuf {
    // Kernel-side storage. Rust note: KVec allocates from the kernel
    // heap with GFP flags, like kmalloc does in C.
    data: Mutex<KVec<u8>>,
}

impl MiscDevice for MyBuf {
    type OpenData = ();

    fn open(_file: &File, _data: &Self::OpenData) -> Result<Pin<KBox<Self>>> {
        pr_info!("my_open: device opened\n");
        Ok(KBox::pin_init(
            try_pin_init!(MyBuf {
                data <- new_mutex!(KVec::new(), "MyBuf::data"),
            }),
            GFP_KERNEL,
        ))
    }

    // C: copy_from_user() + store
    fn write(&self, _file: &File, reader: &mut UserSlicePtrReader) -> Result<usize> {
        // Safe copy_from_user() equivalent: validates the userspace
        // address and copies everything into a kernel vector.
        let mut incoming = KVec::new();
        reader.read_all(&mut incoming, GFP_KERNEL)?;
        let len = incoming.len();

        // Store, replacing whatever was there before (like the C version).
        let mut data = self.data.lock();
        *data = incoming;

        match core::str::from_utf8(&data) {
            Ok(s) => pr_info!("my_write: stored {:?} ({} bytes)\n", s, len),
            Err(_) => pr_info!("my_write: stored {} bytes\n", len),
        }
        Ok(len)
    }

    // C: copy_to_user() + advance pos
    fn read(&self, _file: &File, writer: &mut UserSlicePtrWriter) -> Result<usize> {
        let data = self.data.lock();
        if data.is_empty() {
            return Ok(0); // nothing stored: EOF, `cat` stops
        }
        // Safe copy_to_user() equivalent.
        writer.write_slice(&data)?;
        pr_info!("my_read: returned {} bytes\n", data.len());
        Ok(data.len())
    }

    fn release(_this: Pin<KBox<Self>>, _file: &File) {
        pr_info!("my_release: device closed\n");
    }
}
