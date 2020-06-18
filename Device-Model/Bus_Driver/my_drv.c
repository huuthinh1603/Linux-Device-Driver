#include <linux/module.h>
#include <linux/device.h>
#include "dev_drv.h"

static int my_drv_probe(struct my_device *mdev)
{
	pr_info("Jee:	%s\t%d\n", __func__, __LINE__);
	return 0;
}

static int my_drv_remove(struct my_device *mdev)
{
	pr_info("Jee:   %s\t%d\n", __func__, __LINE__);
        return 0;
}

static struct my_driver mdrv = {
	.probe	= my_drv_probe,
	.remove	= my_drv_remove,
	.drv	= {
		.name	= "my_drv",
	},
};

static int __init my_drv_init(void)
{
	return my_driver_register(&mdrv);	
}

static void __exit my_drv_exit(void)
{
	my_driver_unregister(&mdrv);
}

module_init(my_drv_init);
module_exit(my_drv_exit);

MODULE_LICENSE("GPL");
