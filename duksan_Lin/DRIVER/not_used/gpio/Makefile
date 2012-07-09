#
# kernel 2.6 driver Makefile
#
obj-m	:= dev_gpio.o

KDIR	:= /duksan_app/kernel/LSU_LINUX_2.6.13
PWD		:= $(shell pwd)

default:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules

clean :	
	rm -rf *.ko
	rm -rf *.mod.*
	rm -rf .*.cmd
	rm -rf *.o
	rm -rf Module.*
	rm -rf module.*
