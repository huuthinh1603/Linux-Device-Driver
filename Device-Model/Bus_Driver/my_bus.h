#ifndef __MY_BUS_H__
#define __MY_BUS_H__

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/list.h>

#define dev_to_my_device(ptr)	container_of(ptr, struct my_device, dev)
#define drv_to_my_driver(ptr)	container_of(ptr, struct my_driver, drv)

extern struct bus_type my_bus_type;
extern struct idr adap_idr;

struct my_adapter {
	uint32_t id;
	void *data;
	struct device dev;
};

struct my_device {
	unsigned char name[32];
	struct my_device_id *id_entry;
	struct my_adapter *adap;
	struct device dev;
};

struct my_device_id {
	unsigned char name[32];
	void *data;
};

struct my_driver {
	int (*probe)(struct my_device *);
	int (*remove)(struct my_device *);
	struct my_device_id *id_table;
	struct device_driver drv;
};

#endif /* __MY_BUS_H__ */
