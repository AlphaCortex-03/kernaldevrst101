// SPDX-License-Identifier: GPL-2.0
/*
 * 009.module_params - parameters given at insmod time
 *
 * Modules can receive values at load time:
 *   sudo insmod myparams.ko myname=Gopal count=5 myarray=1,2,3 debug=1
 *
 * Each parameter also shows up as a file under
 *   /sys/module/myparams/parameters/
 * The file mode we pass (0444) makes them read-only: normal users can
 * `cat` them, but they cannot `echo` new values in - changing a 0444
 * parameter needs a module reload.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Gopal");
MODULE_DESCRIPTION("009: module parameters");

/* ------------------------------------------------------------------
 * Step 1: variables that hold the values. Defaults are used when
 * insmod does not get the parameter.
 * ------------------------------------------------------------------ */
static char *myname = "nobody"; /* charp = pointer to a C string        */
static int count = 1;		/* plain int                            */
static int myarray[8];		/* int array, filled from myarray=1,2,3 */
static int myarray_len;		/* how many elements the user passed    */
static bool debug = false;	/* true/false, accepts debug=1 or =y    */

/* ------------------------------------------------------------------
 * Step 2: declare each one as a module parameter.
 * The last argument (0444) is the mode of the sysfs file:
 *   0    -> no file in /sys at all
 *   0444 -> file exists, read-only for normal users
 *   0644 -> root may write the file to change the value at runtime
 * MODULE_PARM_DESC is just documentation (shows in modinfo).
 * ------------------------------------------------------------------ */
module_param(myname, charp, 0444);
MODULE_PARM_DESC(myname, "Name to greet (string)");

module_param(count, int, 0444);
MODULE_PARM_DESC(count, "How many times to greet");

module_param_array(myarray, int, &myarray_len, 0444);
MODULE_PARM_DESC(myarray, "Demo int array, e.g. myarray=1,2,3");

module_param(debug, bool, 0444);
MODULE_PARM_DESC(debug, "Enable extra debug output");

static int __init myparams_init(void)
{
	int i;

	pr_info("%s: loaded with parameters:\n", __func__);
	pr_info("%s:   myname  = \"%s\"\n", __func__, myname);
	pr_info("%s:   count   = %d\n", __func__, count);
	pr_info("%s:   myarray = [%d, %d, %d] (count=%d)\n", __func__,
		myarray[0], myarray[1], myarray[2], myarray_len);
	pr_info("%s:   debug   = %s\n", __func__, debug ? "true" : "false");

	for (i = 0; i < count; i++)
		pr_info("%s: hello #%d, %s!\n", __func__, i, myname);

	if (debug)
		pr_info("%s: debug output enabled\n", __func__);

	return 0;
}

static void __exit myparams_exit(void)
{
	pr_info("%s: goodbye\n", __func__);
}

module_init(myparams_init);
module_exit(myparams_exit);
