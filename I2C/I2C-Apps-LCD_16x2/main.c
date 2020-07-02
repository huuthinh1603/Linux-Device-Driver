#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c-dev.h>
#include "lcd_16x2.h"

#define NR		3
#define SIZE_NAME	32
#define ADDR		0x27

int fd;

int main(void) {

	char filename[SIZE_NAME];
	int retval;

	snprintf(filename, SIZE_NAME, "/dev/i2c-%d", NR);

	fd = open(filename, O_RDWR);
	if(fd < 0) {
		perror("Could not open file\n");
		exit(1);
	}

	retval = ioctl(fd, I2C_SLAVE, ADDR);
	if(retval) {
		perror("Setting address was failed\n");
		close(fd);
		exit(1);
	}

	lcd_init();

	lcd_send_str("Hello, World!!!");

	close(fd);
	return 0;

}
