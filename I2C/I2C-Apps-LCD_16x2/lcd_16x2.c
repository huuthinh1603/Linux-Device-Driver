#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdint.h>
#include "lcd_16x2.h"

extern int fd;

int lcd_send_cmd(unsigned char cmd)
{
	int length;
	char data_u, data_l;
	uint8_t data_t[4];

	data_u = (cmd & 0xf0);
	data_l = ((cmd << 4 ) & 0xf0);
	data_t[0] = data_u | 0x0C;
	data_t[1] = data_u | 0x08;  
	data_t[2] = data_l | 0x0C;
	data_t[3] = data_l | 0x08;

	length = write(fd, data_t, 4);
        if(length != 4) {
                perror("The cmd was sended failed\n");
                return -1;
        }
        return 0;
}

int lcd_send_data(unsigned char data)
{
	int length;
	char data_u, data_l;
	uint8_t data_t[4];

	data_u = (data & 0xf0);
	data_l = ((data << 4) & 0xf0);
	data_t[0] = data_u | 0x0D;
	data_t[1] = data_u | 0x09;
	data_t[2] = data_l | 0x0D;
	data_t[3] = data_l | 0x09;

	length = write(fd, data_t, 4);
	if(length != 4) {
		perror("The data was sended failed\n");
		return -1;
	}

	return 0;
}

void lcd_send_str(const char *str)
{
	printf("%s\n", str);

        while(*str) {
		if(lcd_send_data(*str))
			break;
		str++;
	}
}

void lcd_clean(void)
{
        lcd_send_cmd(0x01);
}

void lcd_init(void)
{
        lcd_send_cmd(0x33);
	lcd_send_cmd(0x32);
        usleep(50 * 1000);
        lcd_send_cmd(0x28);
	usleep(50 * 1000);
        lcd_send_cmd(0x01);
	usleep(50 * 1000);
	lcd_send_cmd (0x06);
	usleep(50 * 1000);
	lcd_send_cmd (0x0c);
	usleep(50 * 1000);
	lcd_send_cmd (0x02);
	usleep(50 * 1000);
	lcd_send_cmd (0x80);

}
