# Makefile pour compiler pour raspberry pi

# Definition des emplacements des modules
include ./tools/libs.mk

# Variables d'environnement
CROSS_COMPILE = /opt/arm-bcm2708/arm-linux-gnueabihf/bin/arm-linux-gnueabihf-
ARCH = arm
LIB_ROOTFS = /opt/rootfs/lib
BIN_ROOTFS = /opt/rootfs/usr

BIN = sw_local.bin

PWD = $(shell pwd)
SUBDIR_MAIN = env/MAIN
SUBDIRS_ENV = $(filter-out $(SUBDIR_MAIN), $(wildcard env/* ))
SUBDIRS_MOD = $(wildcard modules/*)
SUBDIR_DATA = DATA

export PWD
export SUBDIRS_ENV
export SUBDIRS_MOD
export SUBDIR_DATA

export LIB_ROOTFS
export BIN_ROOTFS

# Mode de compilation
all: main
	@echo "###################################"
	@echo "Copie des fichiers dans le serveur TFTP..."
	@echo "-----------------------------------\n"
	cp    $(SUBDIR_MAIN)/bin/$(BIN) /tftpboot/
	cp -r $(SUBDIR_DATA)/			/tftpboot/
	@echo ""
	@echo "$(shell date)"
	@echo "-----------------------------------"
	@echo " Done"

clean:
	@echo "Cleaning env..."
	@for directory in ${SUBDIRS_ENV} ; do \
		$(MAKE) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $$directory -f module.mk clean ; \
	done
	@echo "Cleaning modules...."
	@for directory in ${SUBDIRS_MOD} ; do \
		$(MAKE) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $$directory -f module.mk clean ; \
	done
	@$(MAKE) -C $(SUBDIR_MAIN) -f module.mk clean

distclean: clean
	@echo "Cleaning env..."
	@for directory in ${SUBDIRS_ENV} ; do \
		$(MAKE) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $$directory -f module.mk libclean ; \
	done
	@echo "Cleaning modules...."
	@for directory in ${SUBDIRS_MOD} ; do \
		$(MAKE) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $$directory -f module.mk libclean ; \
	done
	$(MAKE) -C $(SUBDIR_MAIN) -f module.mk distclean

# Compilation des modules d'environnement sauf le main
env:
	@echo "###################################"
	@echo "Compiling environment..."
	@echo "-----------------------------------\n"
	@for directory in ${SUBDIRS_ENV} ; do \
		$(MAKE) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $$directory -f module.mk ; \
	done

# Compilation des modules utilisateur
modules:
	@echo "###################################"
	@echo "Compiling modules..."
	@echo "-----------------------------------\n"
	@for directory in ${SUBDIRS_MOD} ; do \
		$(MAKE) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $$directory -f module.mk ; \
	done

# Compilation du main puis executable
main: env modules
	@echo "###################################"
	@echo "Linking..."
	@echo "-----------------------------------\n"
	$(MAKE) ARCH=$(ARCH) CROSS_COMPILE=$(CROSS_COMPILE) -C $(SUBDIR_MAIN) -f module.mk bin

print-%:
	@echo $* = $($(*))

.PHONY: all main modules env clean

