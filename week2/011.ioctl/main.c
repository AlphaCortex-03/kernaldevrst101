// SPDX-License-Identifier: GPL-2.0
/*
 * 011.ioctl - control commands from userspace
 *
 * read/write only move a stream of bytes. When you need the program to
 * send *commands* ("clear the buffer", "tell me the size", "store this
 * value") you use ioctl: one function that receives a command number
 * plus one argument.
 *
 * Command numbers are built with _IO / _IOR / _IOW so that kernel and
 * userspace always agree on the exact number and on the direction:
 *   _IO(magic, nr)         no data                -> IOCTL_CLEAR
 *   _IOR(magic, nr, type)  kernel READS... no:    -> IOCTL_GET_SIZE
 *                          (type is copied OUT to userspace)
 *   _IOW(magic, nr, type)  data flows INTO kernel -> IOCTL_SET_VALUE
 * R = Read from the device's point of view (kernel -> user),
 * W = Write to the device (user -> kernel). Easy to confuse!
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/uaccess.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Gopal");
MODULE_DESCRIPTION("011: ioctl control commands on /dev/mydevice");

#define BUF_SIZE 1024

/* The magic number is an arbitrary byte that identifies our device.
 * Use the same one in kernel and userspace (user_app.c repeats it). */
#define IOCTL_MAGIC 'g'

#define IOCTL_CLEAR	_IO(IOCTL_MAGIC, 1)	/* no data */
#define IOCTL_GET_SIZE	_IOR(IOCTL_MAGIC, 2, int) /* int sent out */
#define IOCTL_SET_VALUE	_IOW(IOCTL_MAGIC, 3, int) /* int sent in */

static char kbuf[BUF_SIZE];
static size_t data_len;		/* bytes stored in kbuf */
static int stored_value;	/* set via IOCTL_SET_VALUE */

static int my_open(struct inode *inode, struct file *file)
{
	pr_info("%s: device opened\n", __func__);
	return 0;
}

static int my_release(struct inode *inode, struct file *file)
{
	pr_info("%s: device closed\n", __func__);
	return 0;
}

/* Normal read/write still work - ioctl is ON TOP of them. */
static ssize_t my_write(struct file *file, const char __user *ubuf,
			size_t count, loff_t *pos)
{
	size_t to_copy = count;

	if (to_copy > BUF_SIZE - 1)
		to_copy = BUF_SIZE - 1;

	if (copy_from_user(kbuf, ubuf, to_copy))
		return -EFAULT;

	kbuf[to_copy] = '\0';
	data_len = to_copy;
	pr_info("%s: stored %zu bytes\n", __func__, data_len);
	return count;
}

/* ------------------------------------------------------------------
 * The ioctl handler. `cmd` is the command number, `arg` is whatever
 * the user passed - for data-carrying commands it is a pointer to a
 * userspace int, so it arrives as an unsigned long we must cast.
 * ------------------------------------------------------------------ */
static long my_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	int __user *argp = (int __user *)arg;
	int val;

	pr_info("%s: cmd=%#x arg=%#lx\n", __func__, cmd, arg);

	switch (cmd) {
	case IOCTL_CLEAR:
		memset(kbuf, 0, sizeof(kbuf));
		data_len = 0;
		pr_info("%s: buffer cleared\n", __func__);
		return 0;

	case IOCTL_GET_SIZE:
		/* copy_to_user: send data_len OUT to the user's int. */
		val = (int)data_len;
		if (copy_to_user(argp, &val, sizeof(val)))
			return -EFAULT;
		pr_info("%s: told userspace size=%d\n", __func__, val);
		return 0;

	case IOCTL_SET_VALUE:
		/* copy_from_user: pull an int IN from the user's int. */
		if (copy_from_user(&val, argp, sizeof(val)))
			return -EFAULT;
		stored_value = val;
		pr_info("%s: stored value=%d\n", __func__, stored_value);
		return 0;

	default:
		/* ENOTTY is the traditional "unknown ioctl" error.
		 * Yes, it literally means "not a tty" - historical. */
		pr_err("%s: unknown cmd %#x\n", __func__, cmd);
		return -ENOTTY;
	}
}

static const struct file_operations my_fops = {
	.owner = THIS_MODULE,
	.open = my_open,
	.write = my_write,
	.release = my_release,
	.unlocked_ioctl = my_ioctl, /* the modern ioctl hook */
};

static struct miscdevice my_misc = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "mydevice", /* /dev/mydevice */
	.fops = &my_fops,
};

static int __init myioctl_init(void)
{
	int ret;

	ret = misc_register(&my_misc);
	if (ret) {
		pr_err("%s: misc_register failed: %d\n", __func__, ret);
		return ret;
	}
	pr_info("%s: /dev/mydevice ready (ioctl commands 0x%02x*)\n",
		__func__, IOCTL_MAGIC);
	return 0;
}

static void __exit myioctl_exit(void)
{
	misc_deregister(&my_misc);
	pr_info("%s: /dev/mydevice removed\n", __func__);
}

module_init(myioctl_init);
module_exit(myioctl_exit);
