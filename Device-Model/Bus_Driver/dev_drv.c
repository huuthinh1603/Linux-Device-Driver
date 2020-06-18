#include <linux/module.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/idr.h>
#include "dev_drv.h"

int my_device_register(struct my_device *mdev, unsigned int id)
{
	struct my_adapter *adap;

	if(!mdev->name[0])
		return -EINVAL;

	adap = idr_find(&adap_idr, id);
	if(!adap)
		return -EINVAL;

	dev_set_name(&mdev->dev, "%s", mdev->name);
	mdev->adap = adap;
	mdev->dev.parent = &adap->dev;
	mdev->dev.bus = &my_bus_type;

	return device_register(&mdev->dev);
}
EXPORT_SYMBOL_GPL(my_device_register);

void my_device_unregister(struct my_device *mdev)
{
	device_unregister(&mdev->dev);
}
EXPORT_SYMBOL_GPL(my_device_unregister);

int my_driver_register(struct my_driver *mdrv)
{
	mdrv->drv.owner = THIS_MODULE;
	mdrv->drv.bus = &my_bus_type;

	return driver_register(&mdrv->drv);
}
EXPORT_SYMBOL_GPL(my_driver_register);

void my_driver_unregister(struct my_driver *mdrv)
{
	driver_unregister(&mdrv->drv);
}
EXPORT_SYMBOL_GPL(my_driver_unregister);















