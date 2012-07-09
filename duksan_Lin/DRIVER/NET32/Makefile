#
# kernel 2.6 driver Makefile
#
obj-m	:= net32_dev.o

KDIR	:= /duksan_app/kernel_3rd/LSU_LINUX_2.6.13
PWD		:= $(shell pwd)

default:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules
	cp net32_dev.ko ./../../OUTPUT/net32_dev.ko
	cp net32_dev.ko /root/AESOP/duksan/MODULE/net32_dev.ko

clean :	
	rm -rf *.ko
	rm -rf *.mod.*
	rm -rf .*.cmd
	rm -rf *.o
	rm -rf Module.*
	rm -rf module.*
