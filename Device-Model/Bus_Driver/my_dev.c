#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include "dev_drv.h"

static struct my_device mdev = {
	.name = "my_dev",
};

static int __init my_dev_init(void)
{
	return my_device_register(&mdev, 0);
}

static void __exit my_dev_exit(void)
{
	my_device_unregister(&mdev);
}

module_init(my_dev_init);
module_exit(my_dev_exit);

MODULE_LICENSE("GPL");



