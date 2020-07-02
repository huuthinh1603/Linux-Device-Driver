KERNEL:=/home/huuthinh/Working/BBB/linux

PWD:=$(shell pwd)

all:
	make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- -C $(KERNEL) SUBDIRS=$(PWD) modules

clean :
	make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- -C $(KERNEL) SUBDIRS=$(PWD) clean
