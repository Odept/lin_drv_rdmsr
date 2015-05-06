ifeq ($(KERNELRELEASE),)
	#KERNEL_SOURCE := /usr/src/linux
	SOME_PATH := /lib/modules/$(shell uname -r)/build
	#PWD := $(shell pwd)
	APP := rdmsr

default: module app

module:
	#$(MAKE) -C $(KERNEL_SOURCE) SUBDIRS=$(PWD) modules
	$(MAKE) -C $(SOME_PATH) M=$(PWD) modules

app:
	gcc main.c -o $(APP)

clean:
	#$(MAKE) -C $(KERNEL_SOURCE) SUBDIRS=$(PWD) clean
	$(MAKE) -C $(SOME_PATH) M=$(PWD) clean
	$(RM) $(APP)

else
	obj-m += drv.o
endif
