// SPDX-License-Identifier: GPL-2.0
/*
 * 008.buffer_module - real data transfer through /dev/mydevice
 *
 * Lesson 007 only *saw* the read/write calls. Here we actually move
 * data: bytes from userspace are copied into a kernel buffer with
 * copy_from_user(), and read back out with copy_to_user().
 *
 * GOLDEN RULE: you NEVER read or write a userspace pointer directly
 * (no strcpy, no memcpy on it!). Userspace memory may be swapped out,
 * unmapped, or malicious. copy_to_user()/copy_from_user() validate the
 * address, handle faults, and return the number of bytes that could
 * NOT be copied (0 == full success).
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/uaccess.h> /* copy_to_user / copy_from_user */

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Gopal");
MODULE_DESCRIPTION("008: kernel buffer behind /dev/mydevice");

#define BUF_SIZE 1024

/* Our kernel-side storage. Only the kernel may touch this memory. */
static char kbuf[BUF_SIZE];
static size_t data_len; /* how many valid bytes are in kbuf */

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

static ssize_t my_read(struct file *file, char __user *ubuf, size_t count,
		       loff_t *pos)
{
	size_t to_copy;

	pr_info("%s: read(count=%zu, pos=%lld)\n", __func__, count,
		(long long)*pos);

	/* Nothing stored, or everything already read: EOF (cat stops). */
	if (*pos >= data_len)
		return 0;

	/* Never copy more than: what the user asked, what is left in the
	 * buffer, and what fits in kbuf. */
	to_copy = count;
	if (to_copy > data_len - *pos)
		to_copy = data_len - *pos;
	if (to_copy > BUF_SIZE - *pos)
		to_copy = BUF_SIZE - *pos;

	/* THE important line: kernel memory -> userspace memory. */
	if (copy_to_user(ubuf, kbuf + *pos, to_copy))
		return -EFAULT; /* some bytes did not make it */

	*pos += to_copy;
	pr_info("%s: returned %zu bytes\n", __func__, to_copy);
	return to_copy;
}

static ssize_t my_write(struct file *file, const char __user *ubuf,
			size_t count, loff_t *pos)
{
	size_t to_copy = count;

	pr_info("%s: write(count=%zu)\n", __func__, count);

	if (to_copy > BUF_SIZE - 1)
		to_copy = BUF_SIZE - 1; /* keep room for the '\0' */

	/* THE important line: the userspace pointer may be bad at any
	 * moment - copy_from_user() validates + copies it safely. */
	if (copy_from_user(kbuf, ubuf, to_copy))
		return -EFAULT;

	/* Drop one trailing newline so `echo hi` does not store the '\n'. */
	if (to_copy > 0 && kbuf[to_copy - 1] == '\n')
		to_copy--;

	kbuf[to_copy] = '\0';
	data_len = to_copy;

	pr_info("%s: stored \"%s\" (%zu bytes)\n", __func__, kbuf, data_len);
	return count; /* claim we accepted everything userspace sent */
}

static const struct file_operations my_fops = {
	.owner = THIS_MODULE,
	.open = my_open,
	.read = my_read,
	.write = my_write,
	.release = my_release,
};

static struct miscdevice my_misc = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = "mydevice", /* /dev/mydevice, like lesson 007 */
	.fops = &my_fops,
};

static int __init mybuf_init(void)
{
	int ret;

	ret = misc_register(&my_misc);
	if (ret) {
		pr_err("%s: misc_register failed: %d\n", __func__, ret);
		return ret;
	}
	pr_info("%s: /dev/mydevice ready (kernel buffer of %d bytes)\n",
		__func__, BUF_SIZE);
	return 0;
}

static void __exit mybuf_exit(void)
{
	misc_deregister(&my_misc);
	pr_info("%s: /dev/mydevice removed\n", __func__);
}

module_init(mybuf_init);
module_exit(mybuf_exit);
