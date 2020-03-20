# Makefile pour le module d'interruption
TOOLCHAIN_DIR=/opt/arm-bcm2708/arm-linux-gnueabihf
TOOLCHAIN_DIR=/opt/armv6-rpi-linux-gnueabihf

ARCH := arm
ARCH_EXT = v6

CROSS_COMPILE = $(TOOLCHAIN_DIR)/bin/$(ARCH)$(ARCH_EXT)-linux-gnueabihf-

#KERNEL_DIR ?= /home/martin/raspi-kernel/lib/modules/4.14.70/build/
KERNEL_DIR ?= /home/martin/buildroot-2019.02.5/output/build/linux-custom/
MODULE_DIR := $(shell pwd)
obj-m := kisr_adc.o

all: modules

modules:
	${MAKE} ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} -C ${KERNEL_DIR} M=${MODULE_DIR}  modules

clean:
	rm -f *.o *.ko *.mod.c .*.o .*.ko .*.mod.c .*.cmd *~
	rm -f Module.symvers Module.markers modules.order
	rm -rf .tmp_versions

.PHONY: all modules
