// SPDX-License-Identifier: GPL-2.0
/*
 * 010.sysfs - expose state as a file: /sys/kernel/my_mod/value
 *
 * sysfs is "one value per file". Reading the file calls our show()
 * function, writing it calls our store() function. That is the whole
 * idea - no read/write data streams, no commands, just values.
 *
 *   sysfs   -> one value per file, for config/state (this lesson)
 *   procfs  -> process info + some legacy kernel state (streams)
 *   debugfs -> developer dumping ground, no stability guarantees
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/kobject.h>
#include <linux/sysfs.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Gopal");
MODULE_DESCRIPTION("010: sysfs attribute under /sys/kernel/my_mod");

/* The state we expose. */
static int value = 42;

/* ------------------------------------------------------------------
 * show(): called when the file is READ (e.g. cat).
 * Fill the page-sized `buf` and return how many bytes you wrote.
 * sysfs_emit() is the safe way to fill it (replaces sprintf).
 * ------------------------------------------------------------------ */
static ssize_t value_show(struct kobject *kobj, struct kobj_attribute *attr,
			  char *buf)
{
	return sysfs_emit(buf, "%d\n", value);
}

/* ------------------------------------------------------------------
 * store(): called when the file is WRITTEN (e.g. echo 99 > ...).
 * `buf` holds what the user wrote, `count` its length.
 * Return the number of bytes consumed (usually count).
 * ------------------------------------------------------------------ */
static ssize_t value_store(struct kobject *kobj, struct kobj_attribute *attr,
			   const char *buf, size_t count)
{
	int new_value;
	int ret;

	/* kstrtoint is the kernel's safe atoi: it refuses junk input
	 * ("abc") and understands the trailing '\n' echo sends. */
	ret = kstrtoint(buf, 0, &new_value);
	if (ret) {
		pr_err("%s: invalid input, keeping value=%d\n", __func__, value);
		return ret;
	}

	value = new_value;
	pr_info("%s: value set to %d\n", __func__, value);
	return count;
}

/* __ATTR(name, mode, show, store) - mode 0644 so users may read and
 * root may write. (DEVICE_ATTR is for struct device in the driver
 * model; we attach to a plain kobject, so we use __ATTR.) */
static struct kobj_attribute value_attr = __ATTR(value, 0644, value_show,
						 value_store);

static struct kobject *my_kobj;

static int __init mysysfs_init(void)
{
	int ret;

	/* Creates the directory /sys/kernel/my_mod
	 * (kernel_kobj is the kobject behind /sys/kernel). */
	my_kobj = kobject_create_and_add("my_mod", kernel_kobj);
	if (!my_kobj) {
		pr_err("%s: failed to create kobject\n", __func__);
		return -ENOMEM;
	}

	/* Creates the file value inside that directory. */
	ret = sysfs_create_file(my_kobj, &value_attr.attr);
	if (ret) {
		pr_err("%s: sysfs_create_file failed: %d\n", __func__, ret);
		kobject_put(my_kobj);
		return ret;
	}

	pr_info("%s: /sys/kernel/my_mod/value ready (value=%d)\n", __func__,
		value);
	return 0;
}

static void __exit mysysfs_exit(void)
{
	sysfs_remove_file(my_kobj, &value_attr.attr);
	kobject_put(my_kobj); /* drop our reference; the dir disappears */
	pr_info("%s: /sys/kernel/my_mod removed\n", __func__);
}

module_init(mysysfs_init);
module_exit(mysysfs_exit);
