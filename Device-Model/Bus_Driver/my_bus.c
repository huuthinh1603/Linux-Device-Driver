#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/idr.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/mod_devicetable.h>

#include "my_bus.h"
#include "dev_drv.h"

#define AUTHOR	"Huu Thinh <huuthinh1603@gmail.com>"
#define DESC	"This module is a simple bus driver"

DEFINE_IDR(adap_idr);
static DEFINE_MUTEX(idr_mutex);

void my_adap_release(struct device *dev)
{
	struct my_adapter *adap = dev_get_drvdata(dev);

	if(adap->data)
		kfree(adap->data);

	idr_remove(&adap_idr, adap->id);
	kfree(adap);
}

static int my_plf_probe(struct platform_device *pdev)
{
	struct my_adapter *adap;
	int err = -EPERM;
	
	adap = kzalloc(sizeof(*adap), GFP_KERNEL);
	if(!adap) 
		return -ENOMEM;
	
	mutex_lock(&idr_mutex);
	adap->id = idr_alloc(&adap_idr, (void *)adap, 0, 32, GFP_KERNEL);
	if(adap->id == -ENOSPC) {
		mutex_unlock(&idr_mutex);
		err = -EBUSY;
		goto idr_fail;
	}
	mutex_unlock(&idr_mutex);

	dev_set_drvdata(&adap->dev, adap);
	dev_set_name(&adap->dev, "my_dev-%u", adap->id);
	adap->dev.parent = &pdev->dev;
	adap->dev.release = my_adap_release;

	err = device_register(&adap->dev);
	if(err)
		goto register_device_fail;
	
	pr_info("Register the %s device was success\n", dev_name(&adap->dev));

	platform_set_drvdata(pdev, adap);
	
	return 0;

register_device_fail:
	idr_remove(&adap_idr, adap->id);
idr_fail:
	kfree(adap);
	return err;
}

static int my_plf_remove(struct platform_device *pdev)
{
	struct my_adapter *adap = platform_get_drvdata(pdev);

	device_unregister(&adap->dev);
	return 0;
}

static struct of_device_id my_plf_of_id[] = {
	{.compatible = "my_bus", },
	{ }
};

static struct platform_driver my_plf_driver = {
	.probe	= my_plf_probe,
	.remove	= my_plf_remove,
	.driver	= {
		.name	= "Jee",
		.of_match_table	= my_plf_of_id,
	},
};

static int match_id_table(struct my_device_id *id_table, struct my_device *mdev)
{
	while(id_table->name[0]) {
		if(!strcmp(id_table->name, mdev->name)) {
			mdev->id_entry = id_table;
			return 1;
		}

		id_table++;
	}

	return 0;
}

static int my_bus_match(struct device *dev, struct device_driver *drv)
{
	struct my_device *mdev = dev_to_my_device(dev);
	struct my_driver *mdrv = drv_to_my_driver(drv);
	
	if(mdrv->id_table)
		return match_id_table(mdrv->id_table, mdev);

	return !strcmp(mdev->name, drv->name);
}

static int my_bus_probe(struct device *dev)
{
	struct my_device *mdev = dev_to_my_device(dev);
	struct my_driver *mdrv = drv_to_my_driver(dev->driver);

	return mdrv->probe(mdev);
}

static int my_bus_remove(struct device *dev)
{
        struct my_device *mdev = dev_to_my_device(dev);
        struct my_driver *mdrv = drv_to_my_driver(dev->driver);

        return mdrv->remove(mdev);
}

ssize_t adap_data_show(struct device *dev, struct device_attribute *attr,
			char *buf)
{
	struct my_device *mdev = dev_to_my_device(dev);

	if(mdev->adap->data) 
		strncpy(buf, (char *)mdev->adap->data, PAGE_SIZE);
	else
		strncpy(buf, "Hello, World!\n", PAGE_SIZE);

	return strlen(buf);
}

ssize_t adap_data_store(struct device *dev, struct device_attribute *attr,
			 const char *buf, size_t count)
{
	int retval;
	struct my_device *mdev = dev_to_my_device(dev);

	if(!mdev->adap->data) {
		mdev->adap->data = kzalloc(1024, GFP_KERNEL);
		if(!mdev->adap->data)
			return -ENOMEM;
	}
	
	retval = sscanf(buf, "%s", (char *)mdev->adap->data);
	if(!strlen((char *)mdev->adap->data))
		return -EINVAL;

	return strlen((char *)mdev->adap->data);
}
DEVICE_ATTR(adap, 0664, adap_data_show, adap_data_store);

struct attribute *my_dev_attrs[] = {
	&dev_attr_adap.attr,
	NULL
};
ATTRIBUTE_GROUPS(my_dev);

ssize_t new_device_store(struct bus_type *bus, const char *buf, size_t count)
{
	struct my_device *mdev; 
	unsigned char name[32];
	uint32_t id;
	int retval;

	mdev = kzalloc(sizeof(*mdev), GFP_KERNEL);
	if(!mdev)
		return -ENOMEM;

	retval = sscanf(buf, "%s %d", name, &id);
	if(retval != 2) {
		kfree(mdev);
		return -EINVAL;
	}

	strncpy(mdev->name, name, sizeof(mdev->name));
	
	retval = my_device_register(mdev, id);
	if(retval) {
		kfree(mdev);
		return retval;
	}

	return count;
}
BUS_ATTR(new_device, 0664, NULL, new_device_store);

static struct attribute *new_device_attrs[] = {
	&bus_attr_new_device.attr,
	NULL
};
ATTRIBUTE_GROUPS(new_device);

struct bus_type my_bus_type = {
	.name		= "my_bus",
	.dev_name	= "my_dev-",
	.dev_groups	= my_dev_groups,
	.bus_groups	= new_device_groups,
	.match		= my_bus_match,
	.probe		= my_bus_probe,
	.remove		= my_bus_remove,
};

static int __init my_bus_init(void)
{
	int err = -1;

	err = bus_register(&my_bus_type);
	if (err) {
		pr_err("Register the bus was failed\n");
		return err;
	}
	
	err = platform_driver_register(&my_plf_driver);
	if (err) {
		pr_err("Register the platform driver was failed\n");
		goto platform_driver_fail;
	}
	return 0;

platform_driver_fail:
	bus_unregister(&my_bus_type);
	return err;
}

static void __exit my_bus_exit(void)
{
	platform_driver_unregister(&my_plf_driver);
	bus_unregister(&my_bus_type);
}

module_init(my_bus_init);
module_exit(my_bus_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR(AUTHOR);
MODULE_DESCRIPTION(DESC);

