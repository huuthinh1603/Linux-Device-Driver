#ifndef __DEV_DRV_H__
#define __DEV_DRV_H__

#include "my_bus.h"

extern int my_device_register(struct my_device *, unsigned int);
extern void my_device_unregister(struct my_device *);

extern int my_driver_register(struct my_driver *);
extern void my_driver_unregister(struct my_driver *);

#endif /* __DEV_DRV_H__ */
