#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/of_device.h>
#include <linux/i2c.h>

#define adap_nr	2

static struct i2c_board_info lcd_16x2_info = {
	I2C_BOARD_INFO("lcd_16x2", 0x27),
	.flags = 0,
};

static struct i2c_client *client;
static struct i2c_adapter *adap;

static int __init lcd_16x2_init(void)
{

	adap = i2c_get_adapter(adap_nr);
	if(!adap) {
		pr_err("Getting the adapter failed\n");
		return -EINVAL;
	}

	client = i2c_new_device(adap, &lcd_16x2_info);
	if(!client) {
		pr_err("Registerring the device failed\n");
		goto out;
	}

	return 0;

out:
	i2c_put_adapter(adap);
	return -EPERM;
}

static void __exit lcd_16x2_exit(void)
{
	i2c_unregister_device(client);
	i2c_put_adapter(adap);
}

module_init(lcd_16x2_init);
module_exit(lcd_16x2_exit);

MODULE_LICENSE("GPL");
