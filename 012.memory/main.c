// SPDX-License-Identifier: GPL-2.0
/*
 * 012.memory - kmalloc vs kzalloc vs vmalloc, and GFP flags
 *
 * Four allocations, each logged to dmesg, all freed on module unload:
 *
 *   kmalloc(64, GFP_KERNEL)      physical contiguous, NOT zeroed
 *   kzalloc(64, GFP_KERNEL)      kmalloc + zeroed -> safe defaults
 *   vmalloc(256*1024)            virtually contiguous, big, NOT zeroed
 *   kmalloc(32, GFP_ATOMIC)      never-sleeps variant
 *
 * Why vmalloc exists: kmalloc can only get physically contiguous
 * memory, which maxes out around a few MB (page-alignment pressure).
 * vmalloc maps together scattered physical pages into one contiguous
 * *virtual* range, so it can allocate much larger chunks - at the cost
 * of slower access (page-table jumps) and no DMA use.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>   /* kmalloc / kzalloc / kfree */
#include <linux/vmalloc.h> /* vmalloc / vfree */

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Gopal");
MODULE_DESCRIPTION("012: kmalloc vs kzalloc vs vmalloc + GFP flags");

/* Pointers start as NULL: kfree(NULL) and vfree(NULL) are both legal
 * no-ops, so the error path can free "whatever succeeded" safely. */
static char *km_mem;
static char *kz_mem;
static void *vm_mem;
static void *atomic_mem;

static int __init mymemory_init(void)
{
	pr_info("%s: allocating four buffers\n", __func__);

	/* 1) kmalloc: fast, physically contiguous, contents = garbage.
	 * We write a pattern so the "we used it" step is real. */
	km_mem = kmalloc(64, GFP_KERNEL);
	if (!km_mem)
		goto fail;
	memset(km_mem, 'K', 64);
	pr_info("%s: kmalloc(64, GFP_KERNEL)      = %px, byte[0]=0x%02x (contents NOT guaranteed zero!)\n",
		__func__, km_mem, km_mem[0]);

	/* 2) kzalloc: identical to kmalloc but zero-initialized.
	 * Use it whenever you would have kmalloc + memset(0). */
	kz_mem = kzalloc(64, GFP_KERNEL);
	if (!kz_mem)
		goto fail;
	pr_info("%s: kzalloc(64, GFP_KERNEL)       = %px, byte[0]=0x%02x (always zeroed)\n",
		__func__, kz_mem, kz_mem[0]);

	/* 3) vmalloc: big buffer, virtually contiguous (physically
	 * scattered pages glued together by the MMU). Do NOT use it for
	 * DMA, and do not free it with kfree! */
	vm_mem = vmalloc(256 * 1024); /* 256 KB */
	if (!vm_mem)
		goto fail;
	memset(vm_mem, 'V', 256 * 1024);
	pr_info("%s: vmalloc(256*1024)             = %px (virtual range, physically scattered pages)\n",
		__func__, vm_mem);

	/* 4) GFP_ATOMIC: allocation that is NOT allowed to sleep, for
	 * interrupt/timer/tasklet context. It uses a smaller emergency
	 * pool, so it fails more often - callers MUST handle failure.
	 * (Here we only demo it; real ATOMIC use is in IRQ handlers.) */
	atomic_mem = kmalloc(32, GFP_ATOMIC);
	if (!atomic_mem) {
		/* a failure here is normal and must not be fatal */
		pr_warn("%s: kmalloc(32, GFP_ATOMIC) failed - that is allowed, callers must cope\n",
			__func__);
	} else {
		pr_info("%s: kmalloc(32, GFP_ATOMIC)       = %px (never sleeps)\n",
			__func__, atomic_mem);
	}

	pr_info("%s: GFP_KERNEL may sleep & reclaim; GFP_ATOMIC never sleeps\n",
		__func__);
	return 0;

fail:
	pr_err("%s: allocation failed, freeing what succeeded\n", __func__);
	mymemory_exit(); /* safe: kfree/vfree accept NULL */
	return -ENOMEM;
}

static void __exit mymemory_exit(void)
{
	/* The golden rule: free with the MATCHING function and never
	 * double-free. kfree() for kmalloc/kzalloc, vfree() for vmalloc. */
	kfree(km_mem);
	kfree(kz_mem);
	vfree(vm_mem);
	kfree(atomic_mem);
	km_mem = kz_mem = NULL;
	vm_mem = atomic_mem = NULL;
	pr_info("%s: freed everything - no leaks\n", __func__);
}

module_init(mymemory_init);
module_exit(mymemory_exit);
