#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/of_device.h>
#include <linux/i2c.h>
#include "lcd_16x2.h"

static int lcd_16x2_probe_new(struct i2c_client *client)
{
	lcd_init(client);

	lcd_send_str(client, "Hello, World!!!");
	pr_info("Huu Thinh:	%s\t%d\n", __func__, __LINE__);
	return 0;
}

static int lcd_16x2_remove(struct i2c_client *client)
{
	lcd_clean(client);
	pr_info("Huu Thinh:     %s\t%d\n", __func__, __LINE__);
	return 0;
}

static struct i2c_device_id lcd_16x2_dev_id[] = {
	{.name = "lcd_16x2", },
	{ }
};

static struct of_device_id lcd_16x2_of_id[] = {
	{.name = "lcd_16x2", },
	{ }
};

static struct i2c_driver lcd_16x2_driver = {
	.probe_new	= lcd_16x2_probe_new,
	.remove		= lcd_16x2_remove,
	.id_table	= lcd_16x2_dev_id,
	.driver		= {
		.name = "lcd_16x2_driver",
		.of_match_table = lcd_16x2_of_id,
	},
};

module_i2c_driver(lcd_16x2_driver);

MODULE_LICENSE("GPL");
