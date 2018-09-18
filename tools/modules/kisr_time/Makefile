# Makefile pour le module d'interruption
ARCH := arm
CROSS_COMPILE:=/opt/arm-bcm2708/arm-linux-gnueabihf/bin/arm-linux-gnueabihf-

KERNEL_DIR := /home/martin/raspi-kernel/lib/modules/4.14.70/build/
MODULE_DIR := $(shell pwd)

K_SRC := $(filter-out %.mod.c, $(wildcard *.c))
#obj-m := $(K_SRC:%.c=%.o)
obj-m := kisr_time.o

all: modules

modules:
	${MAKE} ARCH=${ARCH} CROSS_COMPILE=${CROSS_COMPILE} -C ${KERNEL_DIR} M=${MODULE_DIR} modules

clean:
	rm -f *.o *.ko *.mod.c .*.o .*.ko .*.mod.c .*.cmd *~
	rm -f Module.symvers Module.markers modules.order
	rm -rf .tmp_versions

print-%:
	@echo $* = $($(*))

.PHONY: all modules
