# Raspberry pi specific makefile

##################################################
# Definition for the location of the modules
include ./tools/libs.mk

# Fonctions
_add_lib_suffix=$(foreach _dir,$(1),$(addsuffix /lib/lib$(notdir $(_dir)).a, $(_dir)))
_get_object_file=$(subst $(2),.o,$(subst /src/,/obj/,$(wildcard $(subst /lib/,/src,$(dir $(1)))/*$(2))))
_get_source_file=$(subst /obj/,/src,$(dir $(1)))/$(subst .o,$(2),$(notdir $(1)))

##################################################
# Environment variables

PARALLEL= -j6

BIN = sw_local.bin
INTEG_LOG_LEVEL = -DINTEGRATION_LOG_LEVEL=6

PWD = $(shell pwd)
SUBDIR_MAIN = env/MAIN
SUBDIRS_ENV = $(filter-out $(SUBDIR_MAIN), $(wildcard env/* ))
SUBDIRS_MOD = $(wildcard modules/*)
SUBDIR_DATA = DATA
SUBDIR_KERN = tools/modules

# List of the libraries needed to compile the binary
LIST_LIBENV = $(call _add_lib_suffix,$(SUBDIRS_ENV))
LIST_LIBMOD = $(call _add_lib_suffix,$(SUBDIRS_MOD))

# Create a list of rules to clean all the libs
LIST_CLEAN_ENV = $(addprefix clean_, $(notdir $(SUBDIRS_ENV)))
LIST_CLEAN_MOD = $(addprefix clean_, $(notdir $(SUBDIRS_MOD)))

DIST_CLEAN_ENV = $(addprefix distclean_, $(notdir $(SUBDIRS_ENV)))
DIST_CLEAN_MOD = $(addprefix distclean_, $(notdir $(SUBDIRS_MOD)))

# Get the list of all the objects file needed to compile the file
OBJ_FILES := $(foreach lib,$(LIST_LIBENV) $(LIST_LIBMOD),$(call _get_object_file,$(lib),.c))
OBJ_FILES += $(foreach lib,$(LIST_LIBENV) $(LIST_LIBMOD),$(call _get_object_file,$(lib),.cpp))

OBJ_FILES_MAIN = $(patsubst $(SUBDIR_MAIN)/src/%.cpp, $(SUBDIR_MAIN)/obj/%.o, $(wildcard $(SUBDIR_MAIN)/src/*.cpp))

PATH_BINARY = $(SUBDIR_MAIN)/bin/$(BIN)

##################################################
# Compilation variables

TOOLCHAIN_DIR=/opt/arm-bcm2708/arm-linux-gnueabihf
TOOLCHAIN_DIR=/opt/armv6-rpi-linux-gnueabihf

ARCH = arm
ARCH_EXT = v6
DEBUG = -g
LIB_ROOTFS = $(TOOLCHAIN_DIR)/lib
BIN_ROOTFS = $(TOOLCHAIN_DIR)/include

CROSS_COMPILE = $(TOOLCHAIN_DIR)/bin/$(ARCH)$(ARCH_EXT)-linux-gnueabihf-

LIBS_PATH += $(patsubst %, -L$(PWD)/%/lib, $(SUBDIRS_ENV))
LIBS_PATH += $(patsubst %, -L$(PWD)/%/lib, $(SUBDIRS_MOD))
LIBS_PATH += -L$(LIB_ROOTFS)

LIBS += $(patsubst modules/%, -l%, $(SUBDIRS_MOD))
LIBS += $(patsubst env/%, -l%, $(SUBDIRS_ENV))
LIBS += -lpthread #-lwiringPi -lwiringPiDev

INCLUDES += $(patsubst %, -I$(PWD)/%/api, $(SUBDIRS_MOD))
INCLUDES += $(patsubst %, -I$(PWD)/%/api, $(SUBDIRS_ENV))
INCLUDES += $(patsubst %, -I$(PWD)/%/api, $(SUBDIR_MAIN))
INCLUDES += -I$(PWD)/tools/api
INCLUDES += -I$(BIN_ROOTFS)

C_CPLUS_FLAGS = -Wextra -Wall -Wundef -Wfloat-equal -Wshadow -Wpointer-arith -Wcast-align -Wstrict-overflow=5
C_CPLUS_FLAGS += -O2 -pedantic
C_CPLUS_FLAGS += -Wswitch-default -Wswitch-enum
C_CPLUS_FLAGS += -Wunreachable-code -Wconversion -Wcast-qual
C_CPLUS_FLAGS += -mtune=arm1176jzf-s -mfpu=vfp -mfloat-abi=hard -marm -march=armv6kz+fp
C_CPLUS_FLAGS += $(INTEG_LOG_LEVEL)

C_FLAGS 	+= $(C_CPLUS_FLAGS) -Wstrict-prototypes -std=c99
C_FLAGS 	+= -D_POSIX_C_SOURCE=199506L -D_GNU_SOURCE
CPLUS_FLAGS += $(C_CPLUS_FLAGS) -std=c++11

AR = ar
CC = gcc
CXX = g++
STRIP = strip
RM = rm -rf

export PWD
export SUBDIR_MAIN
export SUBDIRS_ENV
export SUBDIRS_MOD
export SUBDIR_DATA

##################################################
# Compilation rules

all: $(PATH_BINARY) $(SUBDIR_DATA) | /tftpboot/
	@echo "   CP $<"
	@echo "   CP $(SUBDIR_DATA)"
	@cp    $(SUBDIR_MAIN)/bin/$(BIN) /tftpboot/
	@cp -r $(SUBDIR_DATA)/			/tftpboot/
	@echo "$(shell date)"
	@echo "-----------------------------------"
	@echo " Done"

.SECONDARY: $(addsuffix /lib/, $(SUBDIRS_ENV) $(SUBDIRS_MOD)) \
    $(addsuffix /obj/, $(SUBDIRS_ENV) $(SUBDIRS_MOD) $(SUBDIR_MAIN)) \
    $(SUBDIR_MAIN)/bin

# Get the source file associated with the object file being compiled.
# One rule for C and another for C++
.SECONDEXPANSION:
$(PATH_BINARY): $(LIST_LIBENV) $(LIST_LIBMOD) $(OBJ_FILES) $(OBJ_FILES_MAIN) | $(SUBDIR_MAIN)/bin
	@echo "   LD $@"
	@#$(MAKE) $(PARALLEL) -C $(SUBDIR_MAIN) -f module.mk bin
	@$(CROSS_COMPILE)$(CXX) $(OBJ_FILES_MAIN) $(LIBS_PATH) $(LIBS) -o $@

%.o: $$(call _get_source_file,$$@,.c) | $$(dir $$@)
	@echo "   CC $@"
	@#$(MAKE) $(PARALLEL) -C $(subst /obj/,/,$(dir $@)) -f module.mk obj/$(notdir $@)
	@$(CROSS_COMPILE)$(CC) $(INCLUDES) -I$(subst /obj,/inc/,$(dir $@)) $(C_FLAGS) $(DEBUG) -c $^ -o $@

%.o: $$(call _get_source_file,$$@,.cpp) | $$(dir $$@)
	@echo "  CXX $@"
	@#$(MAKE) $(PARALLEL) -C $(subst /obj/,/,$(dir $@)) -f module.mk obj/$(notdir $@)
	@$(CROSS_COMPILE)$(CXX) $(INCLUDES) -I$(subst /obj,/inc/,$(dir $@)) $(CPLUS_FLAGS) $(DEBUG) -c $^ -o $@

# Get all the object files associated with the library being compiled
%.a: $$(call _get_object_file,$$@,.c) $$(call _get_object_file,$$@,.cpp) | $$(dir $$@)
	@echo "   AR $@"
	@#$(MAKE) $(PARALLEL) -C $(subst /lib,,$(dir $@)) -f module.mk lib/$(notdir $@)
	@$(CROSS_COMPILE)$(AR) rcs $@ $^

%/obj/ %/obj::
	@echo "MKDIR $@"
	@mkdir $@

%/lib/ %/lib::
	@echo "MKDIR $@"
	@mkdir $@

%/bin/ %/bin::
	@echo "MKDIR $@"
	@mkdir $@

clean_%: modules/%
	@$(MAKE) $(PARALLEL) -C $< -f module.mk clean

clean_%: env/%
	@$(MAKE) $(PARALLEL) -C $< -f module.mk clean

distclean_%: modules/%
	@$(MAKE) $(PARALLEL) -C $< -f module.mk libclean

distclean_%: env/%
	@$(MAKE) $(PARALLEL) -C $< -f module.mk libclean

clean: $(LIST_CLEAN_MOD) $(LIST_CLEAN_ENV)
	@$(MAKE) $(PARALLEL) -C $(SUBDIR_MAIN) -f module.mk clean

distclean: $(DIST_CLEAN_MOD) $(DIST_CLEAN_ENV)
	@$(MAKE) $(PARALLEL) -C $(SUBDIR_MAIN) -f module.mk distclean
	@$(MAKE) $(PARALLEL) -C $(SUBDIR_KERN) clean

##################################################
# Compilation du module kernel
kmodules:
	@echo "###################################"
	@echo "Compiling kernel modules..."
	@echo "-----------------------------------\n"
	$(MAKE) $(PARALLEL) -C $(SUBDIR_KERN)
	
print-%:
	@echo $* = $($(*))

.PHONY: all main modules env clean distclean kmodules

