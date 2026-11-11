// SPDX-License-Identifier: GPL-2.0
/*
 * 007.char_device - a minimal character device via misc_register()
 *
 * A character device is a file in /dev that user programs can
 * open(), read(), write() and close() like any normal file.
 * Every one of those actions is routed by the kernel into one
 * function of our file_operations table below.
 *
 * We use a "misc device" instead of registering our own major number:
 * all misc devices share major 10, the kernel picks a free minor,
 * and it even creates the /dev/mydevice node for us automatically.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Gopal");
MODULE_DESCRIPTION("007: minimal character device via misc_register");

/* ------------------------------------------------------------------
 * Step 1: the four file_operations callbacks.
 * ------------------------------------------------------------------ */

/* Called when a program does open("/dev/mydevice"). */
static int my_open(struct inode *inode, struct file *file)
{
	pr_info("%s: device opened\n", __func__);
	return 0;
}

/* Called when a program does close() on the file. */
static int my_release(struct inode *inode, struct file *file)
{
	pr_info("%s: device closed\n", __func__);
	return 0;
}

/* Called when a program reads from /dev/mydevice.
 * We have no data to give yet (that is lesson 008), so we just log
 * the call and return 0. Returning 0 from read() means EOF. */
static ssize_t my_read(struct file *file, char __user *buf, size_t count,
		       loff_t *pos)
{
	pr_info("%s: userspace wants to read %zu bytes (we have none yet)\n",
		__func__, count);
	return 0;
}

/* Called when a program writes to /dev/mydevice.
 * `buf` is a USERSPACE pointer - we must never touch it directly!
 * (Lesson 008 shows the safe copy_from_user() way.)
 * For now we only count the bytes and pretend to accept them. */
static ssize_t my_write(struct file *file, const char __user *buf,
			size_t count, loff_t *pos)
{
	pr_info("%s: userspace wrote %zu bytes (ignored for now)\n",
		__func__, count);
	return count; /* tell userspace "all bytes accepted" */
}

/* ------------------------------------------------------------------
 * Step 2: the table that connects the callbacks to the file type.
 * .owner = THIS_MODULE keeps the module loaded while the file is open.
 * ------------------------------------------------------------------ */
static const struct file_operations my_fops = {
	.owner = THIS_MODULE,
	.open = my_open,
	.read = my_read,
	.write = my_write,
	.release = my_release,
};

/* ------------------------------------------------------------------
 * Step 3: the misc device itself.
 * .name becomes the /dev/<name> node - here: /dev/mydevice
 * ------------------------------------------------------------------ */
static struct miscdevice my_misc = {
	.minor = MISC_DYNAMIC_MINOR, /* kernel picks a free minor number */
	.name = "mydevice",	 /* becomes /dev/mydevice              */
	.fops = &my_fops,
};

static int __init mychar_init(void)
{
	int ret;

	ret = misc_register(&my_misc);
	if (ret) {
		pr_err("%s: misc_register failed: %d\n", __func__, ret);
		return ret;
	}
	pr_info("%s: /dev/mydevice registered (minor=%d)\n", __func__,
		my_misc.minor);
	return 0;
}

static void __exit mychar_exit(void)
{
	misc_deregister(&my_misc);
	pr_info("%s: /dev/mydevice removed\n", __func__);
}

module_init(mychar_init);
module_exit(mychar_exit);
