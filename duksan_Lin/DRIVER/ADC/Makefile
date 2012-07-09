#
# kernel 2.6 driver Makefile
#
obj-m	:= dev_adc.o

KDIR	:= /duksan_app/kernel_3rd/LSU_LINUX_2.6.13
PWD		:= $(shell pwd)

default:
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules
	cp dev_adc.ko ./../../OUTPUT/dev_adc.ko
	cp dev_adc.ko /root/AESOP/duksan/MODULE/dev_adc.ko

clean :	
	rm -rf *.ko
	rm -rf *.mod.*
	rm -rf .*.cmd
	rm -rf *.o
	rm -rf Module.*
	rm -rf module.*
