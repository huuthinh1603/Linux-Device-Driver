#include <linux/delay.h>
#include <linux/i2c.h>
#include "lcd_16x2.h"

int lcd_send_cmd(struct lcd_16x2 *lcd)
{
	int length;
	char data_u, data_l;
	uint8_t data_t[4];

	data_u = (lcd->data & 0xf0);
	data_l = ((lcd->data << 4) & 0xf0);
	data_t[0] = data_u | 0x0C;
	data_t[1] = data_u | 0x08;  
	data_t[2] = data_l | 0x0C;
	data_t[3] = data_l | 0x08;

	length = i2c_master_send(lcd->client, data_t, 4);
	if(length != 4) {
		pr_err("Sending the cmd failed\n");
		return -1;
	}

	return 0;
}

int lcd_send_data(struct lcd_16x2 *lcd)
{
	int length;
	char data_u, data_l;
	uint8_t data_t[4];

	data_u = (lcd->data & 0xf0);
	data_l = ((lcd->data << 4) & 0xf0);
	data_t[0] = data_u | 0x0D;
	data_t[1] = data_u | 0x09;
	data_t[2] = data_l | 0x0D;
	data_t[3] = data_l | 0x09;

	length = i2c_master_send(lcd->client, data_t, 4);
	if(length != 4) {
		pr_err("Sending the data failed\n");
		return -1;
	}

	return 0;
}

void lcd_send_str(struct i2c_client *client, char *str)
{
	struct lcd_16x2 lcd;
	lcd.client = client;

	while(*str) {
		lcd.data = *str;
		if(lcd_send_data(&lcd))
			break;

		str++;
	}
}

void lcd_clean(struct i2c_client *client)
{
	struct lcd_16x2 lcd;

        lcd.client = client;
	lcd.data = 0x01;
	lcd_send_cmd(&lcd);

}

void lcd_init(struct i2c_client *client)
{
	struct lcd_16x2 lcd;
	lcd.client = client;

	lcd.data = 0x33;
	lcd_send_cmd(&lcd);

	lcd.data = 0x32;
        lcd_send_cmd(&lcd);
        msleep(50);

	lcd.data = 0x28;
        lcd_send_cmd(&lcd);
        msleep(50);

	lcd.data = 0x01;
        lcd_send_cmd(&lcd);
        msleep(50);

	lcd.data = 0x06;
        lcd_send_cmd(&lcd);
        msleep(50);

	lcd.data = 0x0C;
        lcd_send_cmd(&lcd);
        msleep(50);

	lcd.data = 0x02;
        lcd_send_cmd(&lcd);
        msleep(50);

	lcd.data = 0x80;
        lcd_send_cmd(&lcd);
}
