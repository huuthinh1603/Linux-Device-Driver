KERNEL:=/home/huuthinh/Working/BeagleBone-Black/linux

PWD:=$(shell pwd)

all :
	make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- -C $(KERNEL) SUBDIRS=$(PWD) modules

clean :
	make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf- -C $(KERNEL) SUBDIRS=$(PWD) clean
