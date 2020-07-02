#ifndef __LCD_16x2_H__
#define __LCD_16x2_H__

#include <linux/i2c.h>

struct lcd_16x2 {
        struct i2c_client *client;
        unsigned char data;
};

int lcd_send_cmd(struct lcd_16x2 *);
int lcd_send_data(struct lcd_16x2 *);
void lcd_send_str(struct i2c_client *, char *);
void lcd_clean(struct i2c_client *);
void lcd_init(struct i2c_client *);

#endif /* __LCD_16x2_H__ */
