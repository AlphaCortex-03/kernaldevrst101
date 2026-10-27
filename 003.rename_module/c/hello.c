// SPDX-License-Identifier: GPL-2.0
/*
 * hello.c - Lesson 003: the source file name and the output module name
 * do not have to match. This source produces renamed_hello.ko.
 *
 * See the Makefile for how the output name is changed.
 */

#include <linux/kernel.h>
#include <linux/module.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("AlphaCortex-03");
MODULE_DESCRIPTION("Lesson 003: kernel module with a renamed output file");

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
