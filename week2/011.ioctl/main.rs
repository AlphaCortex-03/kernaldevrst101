// SPDX-License-Identifier: GPL-2.0
//! 011.ioctl - Rust twin of main.c.
//!
//! In C you build command numbers with _IO/_IOR/_IOW macros. Those
//! macros are just bit packing, so in Rust we compute the same numbers
//! as `const` values (the ~6.8-era bindings also ship `kernel::ioctl`
//! helpers on some trees; constants here work everywhere):
//!
//!   _IO(magic, nr)         = (0 << 30) | (0 << 16) | (magic << 8) | nr
//!   _IOR(magic, nr, int)   = (2 << 30) | (4 << 16) | (magic << 8) | nr
//!   _IOW(magic, nr, int)   = (1 << 30) | (4 << 16) | (magic << 8) | nr
//!
//! NOTE: targets the ~6.8 rust-next API; names may differ between trees.

use kernel::{
    fs::File,
    miscdevice::{MiscDevice, UserSlice, UserSlicePtrReader},
    prelude::*,
    sync::{new_mutex, Mutex},
};

const IOCTL_MAGIC: u32 = b'g' as u32;
const IOCTL_CLEAR: u32 = (IOCTL_MAGIC << 8) | 1;
const IOCTL_GET_SIZE: u32 = (2 << 30) | (4 << 16) | (IOCTL_MAGIC << 8) | 2;
const IOCTL_SET_VALUE: u32 = (1 << 30) | (4 << 16) | (IOCTL_MAGIC << 8) | 3;

module_misc_device! {
    type: MyIoctl,
    name: "myioctl_rs",
    author: "Gopal",
    description: "011: ioctl control commands (Rust)",
    license: "GPL",
}

struct MyIoctl {
    data: Mutex<KVec<u8>>, // the buffer written through normal write()
    value: Mutex<i32>,     // set via IOCTL_SET_VALUE
}

impl MiscDevice for MyIoctl {
    type OpenData = ();

    fn open(_file: &File, _data: &Self::OpenData) -> Result<Pin<KBox<Self>>> {
        pr_info!("my_open: device opened\n");
        Ok(KBox::pin_init(
            try_pin_init!(MyIoctl {
                data <- new_mutex!(KVec::new(), "MyIoctl::data"),
                value <- new_mutex!(0, "MyIoctl::value"),
            }),
            GFP_KERNEL,
        ))
    }

    // Normal write, like lesson 008 - ioctl sits ON TOP of read/write.
    fn write(&self, _file: &File, reader: &mut UserSlicePtrReader) -> Result<usize> {
        let mut incoming = KVec::new();
        reader.read_all(&mut incoming, GFP_KERNEL)?;
        let len = incoming.len();
        let mut data = self.data.lock();
        *data = incoming;
        pr_info!("my_write: stored {} bytes\n", len);
        Ok(len)
    }

    // C: unlocked_ioctl. `arg` is the raw userspace value/pointer.
    fn ioctl(&self, _file: &File, cmd: u32, arg: usize) -> Result<i32> {
        match cmd {
            IOCTL_CLEAR => {
                self.data.lock().clear();
                pr_info!("my_ioctl: buffer cleared\n");
                Ok(0)
            }
            IOCTL_GET_SIZE => {
                let size = self.data.lock().len() as i32;
                // arg is a pointer to a userspace int. Rust note: there
                // is no `int __user *` cast here - UserSlice is the
                // enforced-safe copy_to_user().
                UserSlice::new(arg, core::mem::size_of::<i32>())
                    .writer()
                    .write(&size.to_ne_bytes())?;
                pr_info!("my_ioctl: told userspace size={}\n", size);
                Ok(0)
            }
            IOCTL_SET_VALUE => {
                let mut buf = [0u8; core::mem::size_of::<i32>()];
                // enforced-safe copy_from_user()
                UserSlice::new(arg, buf.len()).reader().read(&mut buf)?;
                let value = i32::from_ne_bytes(buf);
                *self.value.lock() = value;
                pr_info!("my_ioctl: stored value={}\n", value);
                Ok(0)
            }
            // Rust note: ENOTTY is the same traditional "unknown ioctl"
            // error code the C version returns.
            _ => Err(ENOTTY),
        }
    }

    fn release(_this: Pin<KBox<Self>>, _file: &File) {
        pr_info!("my_release: device closed\n");
    }
}
