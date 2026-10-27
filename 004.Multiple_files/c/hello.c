// SPDX-License-Identifier: GPL-2.0
/*
 * hello.c - Lesson 004: a kernel module built from multiple source files.
 *
 * func.c is linked into the same module object; see the Makefile.
 */

#include <linux/kernel.h>
#include <linux/module.h>

#include "func.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("AlphaCortex-03");
MODULE_DESCRIPTION("Lesson 004: kernel module with multiple source files");

static int __init test_hello_init(void)
{
	pr_info("%s : In init\n", __func__);
	func();
	return 0;
}

static void __exit test_hello_exit(void)
{
	pr_info("%s : In exit\n", __func__);
}

module_init(test_hello_init);
module_exit(test_hello_exit);
