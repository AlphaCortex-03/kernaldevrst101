// SPDX-License-Identifier: GPL-2.0
/*
 * func.c - Lesson 004: helper source file linked into the multi-file module.
 */

#include <linux/kernel.h>

#include "func.h"

void func(void)
{
	pr_info("Hello Linux\n");
}
