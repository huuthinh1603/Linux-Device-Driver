#ifndef __LCD_16X2_H__
#define __LCD_16X2_H__

int lcd_send_cmd(unsigned char);
int lcd_send_data(unsigned char);
void lcd_send_str(const char *);
void lcd_clean(void);
void lcd_init(void);

#endif /* __LCD_16X2_H__ */
