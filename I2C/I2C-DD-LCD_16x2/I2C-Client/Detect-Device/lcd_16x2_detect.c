#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/of_device.h>
#include <linux/i2c.h>

#define I2C_CLASS_LCD_16x2	(1 << 1)
#define ADAP_NR			2
static int i2c_lcd_detect(struct i2c_client *client, 
			struct i2c_board_info *info)
{
	pr_info("Huu Thinh:	%s\t%d\n", __func__, __LINE__);

	strncpy(info->type, "lcd_16x2", I2C_NAME_SIZE);
	info->flags = 0;
	return 0;
}

unsigned short i2c_lcd_address_list[] = {0x27, 0x28, 0x29, I2C_CLIENT_END};

static struct i2c_driver lcd_16x2_driver = {
	.class		= I2C_CLASS_LCD_16x2,
	.detect		= i2c_lcd_detect,
	.address_list	= i2c_lcd_address_list,
	.driver		= {
		.name = "lcd_detect",
	},
};

static struct i2c_adapter *adap;

static int __init lcd_16x2_init(void)
{
	int retval;

	adap = i2c_get_adapter(ADAP_NR);
	if(!adap) {
		pr_err("Getting the adap failed\n");
		return -EPERM;
	}
	
	if(adap->nr == ADAP_NR)
		adap->class = I2C_CLASS_LCD_16x2;

	retval = i2c_add_driver(&lcd_16x2_driver);
	if(retval) {
		pr_err("Registerring the i2c client failed\n");
		goto out;
	}
	
	return 0;

out:
	i2c_put_adapter(adap);
	return retval;

}

static void __exit lcd_16x2_exit(void)
{
	if(adap->nr == ADAP_NR)
		adap->class = I2C_CLASS_DEPRECATED;

	i2c_put_adapter(adap);	
	i2c_del_driver(&lcd_16x2_driver);
}

module_init(lcd_16x2_init);
module_exit(lcd_16x2_exit);

MODULE_LICENSE("GPL");
