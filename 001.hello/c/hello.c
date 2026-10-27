// SPDX-License-Identifier: GPL-2.0
/*
 * hello.c - A minimal "Hello, World" Linux kernel module.
 *
 * Improvements over the original example:
 *   - SPDX license identifier added
 *   - pr_info() used instead of printk(KERN_INFO ...)
 *   - __init / __exit annotations so the init code is freed after load
 *   - MODULE_AUTHOR / MODULE_DESCRIPTION metadata added
 */

#include <linux/kernel.h>
#include <linux/module.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("AlphaCortex-03");
MODULE_DESCRIPTION("A minimal hello world kernel module");

static int __init test_hello_init(void)
{
	pr_info("%s : In init\n", __func__);
	return 0;
}

static void __exit test_hello_exit(void)
{
	pr_info("%s : In exit\n", __func__);
}

module_init(test_hello_init);
module_exit(test_hello_exit);
