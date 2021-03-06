# Raspberry pi specific makefile

##################################################
# Definition for the location of the modules
include ./tools/libs.mk

##################################################
# Fonctions

# For every dir provided, provide the complete path to the library file
_add_lib_suffix=$(foreach _dir,$(1),$(addsuffix /lib/lib$(notdir $(_dir)).a, $(_dir)))
# Get all objects file for one lib
_get_object_files=$(subst $(2),.o,$(subst /src/,/obj/,$(wildcard $(subst /lib/,/src,$(dir $(1)))/*$(2))))
# Get all prereq files for one directory
_get_prereq_files=$(subst $(2),.d,$(subst /src/,/pre/,$(wildcard $(1)/src/*$(2))))
# Get source from object file
_get_src_obj_file=$(subst /obj/,/src,$(dir $(1)))/$(subst .o,$(2),$(notdir $(1)))
# Get source from prerequisite file
_get_src_pre_file=$(subst /pre/,/src,$(dir $(1)))/$(subst .d,$(2),$(notdir $(1)))
# Get object file from source
_get_obj_src_file=$(subst /src/,/obj,$(dir $(1)))/$(subst $(2),.o,$(notdir $(1)))

##################################################
# Environment variables

PARALLEL = -j6
MAKEFLAGS = --no-print-directory

BIN = sw_local.bin
INTEG_LOG_LEVEL = -DINTEGRATION_LOG_LEVEL=4

PWD = $(shell pwd)
SUBDIR_MAIN = env/MAIN
SUBDIR_BASE = env/BASE
SUBDIRS_ENV = $(filter-out $(SUBDIR_MAIN) $(SUBDIR_BASE), $(wildcard env/* ))
SUBDIRS_MOD = $(wildcard modules/*)
SUBDIR_DATA = DATA
SUBDIR_KERN = tools/modules

# List of the libraries needed to compile the binary
LIST_LIBENV = $(call _add_lib_suffix,$(SUBDIRS_ENV))
LIST_LIBMOD = $(call _add_lib_suffix,$(SUBDIRS_MOD))

# Create a list of rules to clean all the libs
LIST_CLEAN_ENV = $(addprefix clean_, $(notdir $(SUBDIRS_ENV)) $(notdir $(SUBDIR_MAIN)))
LIST_CLEAN_MOD = $(addprefix clean_, $(notdir $(SUBDIRS_MOD)))

DIST_CLEAN_ENV = $(addprefix distclean_, $(notdir $(SUBDIRS_ENV)) $(notdir $(SUBDIR_MAIN)))
DIST_CLEAN_MOD = $(addprefix distclean_, $(notdir $(SUBDIRS_MOD)))

# Get the list of all the objects file needed to compile the file
OBJ_FILES := $(foreach lib,$(LIST_LIBENV) $(LIST_LIBMOD),$(call _get_object_files,$(lib),.c))
OBJ_FILES += $(foreach lib,$(LIST_LIBENV) $(LIST_LIBMOD),$(call _get_object_files,$(lib),.cpp))

PREREQ_FILES := $(foreach _dir, env/MAIN $(SUBDIRS_ENV) $(SUBDIRS_MOD), $(call _get_prereq_files,$(_dir),.c))
PREREQ_FILES += $(foreach _dir, env/MAIN $(SUBDIRS_ENV) $(SUBDIRS_MOD), $(call _get_prereq_files,$(_dir),.cpp))

OBJ_FILES_MAIN = $(patsubst $(SUBDIR_MAIN)/src/%.cpp, $(SUBDIR_MAIN)/obj/%.o, $(wildcard $(SUBDIR_MAIN)/src/*.cpp))

PATH_BINARY = $(SUBDIR_MAIN)/bin/$(BIN)
PATH_STRIP  = $(PATH_BINARY).stripped
PATH_MAP    = $(PATH_BINARY:.bin=.map)

OUTPUT_FILES = $(PATH_BINARY) $(PATH_STRIP) $(PATH_MAP)

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

INCLUDES += $(patsubst %, -I$(PWD)/%/api, $(SUBDIR_BASE))
INCLUDES += $(patsubst %, -I$(PWD)/%/api, $(SUBDIRS_MOD))
INCLUDES += $(patsubst %, -I$(PWD)/%/api, $(SUBDIRS_ENV))
INCLUDES += $(patsubst %, -I$(PWD)/%/api, $(SUBDIR_MAIN))
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

all: $(OUTPUT_FILES) $(SUBDIR_DATA) | /tftpboot/
	@echo "   CP $<"
	@echo "   CP $(SUBDIR_DATA)"
	@cp    $(SUBDIR_MAIN)/bin/$(BIN) /tftpboot/
	@cp    $(SUBDIR_MAIN)/bin/$(BIN) /media/sf_partage_vm/
	@cp -r $(SUBDIR_DATA)/			/tftpboot/
	@echo "$(shell date)"
	@echo "-----------------------------------"
	@echo " Done"

# Include all the file defining the prerequisites for each source file
-include $(PREREQ_FILES)

.SECONDARY: $(addsuffix /lib/, $(SUBDIRS_ENV) $(SUBDIRS_MOD)) \
    $(addsuffix /obj/, $(SUBDIRS_ENV) $(SUBDIRS_MOD) $(SUBDIR_MAIN)) \
    $(SUBDIR_MAIN)/bin \
    $(addsuffix /pre/, $(SUBDIRS_ENV) $(SUBDIRS_MOD) $(SUBDIR_MAIN))

# Get the source file associated with the object file being compiled.
# One rule for C and another for C++
.SECONDEXPANSION:

$(PATH_BINARY): $(LIST_LIBENV) $(LIST_LIBMOD) $(OBJ_FILES) $(OBJ_FILES_MAIN) | $(SUBDIR_MAIN)/bin
	@echo "   LD $@"
	@#$(MAKE) $(PARALLEL) -C $(SUBDIR_MAIN) -f module.mk bin
	@$(CROSS_COMPILE)$(CXX) $(OBJ_FILES_MAIN) $(LIBS_PATH) $(LIBS) -Xlinker -Map=$(PATH_MAP) -o $@

$(PATH_STRIP): $(PATH_BINARY)
	@echo "STRIP $@"
	@$(CROSS_COMPILE)$(STRIP) -s -o $@ $<

$(PATH_MAP): $(PATH_BINARY)
	@echo "  MAP $@"

# Rule to generate prerequisite files
%.d: $$(call _get_src_pre_file,$$@,.c) | $$(dir $$@)
	@echo "  PRE $@"
	@$(RM) $@
	@$(CROSS_COMPILE)$(CC) $(INCLUDES) -I$(subst /pre,/inc/,$(dir $@)) $(C_FLAGS) -MM $< > $@.$$$$; \
	sed 's#$(notdir $(call _get_obj_src_file, $<,.c))[ :]*#$(call _get_obj_src_file, $<,.c) $@: #g' < $@.$$$$ > $@; \
	$(RM) $@.$$$$

%.d: $$(call _get_src_pre_file,$$@,.cpp) | $$(dir $$@)
	@echo "  PRE $@"
	@$(RM) $@
	@$(CROSS_COMPILE)$(CXX) $(INCLUDES) -I$(subst /pre,/inc/,$(dir $@)) $(CPLUS_FLAGS) -MM $< > $@.$$$$; \
	sed 's#$(notdir $(call _get_obj_src_file, $<,.cpp))[ :]*#$(call _get_obj_src_file, $<,.cpp) $@: #g' < $@.$$$$ > $@; \
	$(RM) $@.$$$$

# Rule to generate object files
%.o: $$(call _get_src_obj_file,$$@,.c) | $$(dir $$@)
	@echo "   CC $@"
	@#$(MAKE) $(PARALLEL) -C $(subst /obj/,/,$(dir $@)) -f module.mk obj/$(notdir $@)
	@$(CROSS_COMPILE)$(CC) $(INCLUDES) -I$(subst /obj,/inc/,$(dir $@)) $(C_FLAGS) $(DEBUG) -c $< -o $@

%.o: $$(call _get_src_obj_file,$$@,.cpp) | $$(dir $$@)
	@echo "  CXX $@"
	@#$(MAKE) $(PARALLEL) -C $(subst /obj/,/,$(dir $@)) -f module.mk obj/$(notdir $@)
	@$(CROSS_COMPILE)$(CXX) $(INCLUDES) -I$(subst /obj,/inc/,$(dir $@)) $(CPLUS_FLAGS) $(DEBUG) -c $< -o $@

# Get all the object files associated with the library being compiled
%.a: $$(call _get_object_files,$$@,.c) $$(call _get_object_files,$$@,.cpp) | $$(dir $$@)
	@echo "   AR $@"
	@#$(MAKE) $(PARALLEL) -C $(subst /lib,,$(dir $@)) -f module.mk lib/$(notdir $@)
	@$(CROSS_COMPILE)$(AR) rcs $@ $^

%/pre/ %/pre::
	@echo "MKDIR $@"
	@mkdir $@

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
	@echo "   RM $</obj"
	@$(RM) $</obj

clean_%: env/%
	@echo "   RM $</obj"
	@$(RM) $</obj

distclean_%: modules/% clean_%
	@echo "   RM $</pre"
	@$(RM) $</pre
	@echo "   RM $</lib"
	@$(RM) $</lib

distclean_%: env/% clean_%
	@echo "   RM $</pre"
	@$(RM) $</pre
	@echo "   RM $</lib"
	@$(RM) $</lib

clean: $(LIST_CLEAN_MOD) $(LIST_CLEAN_ENV)

distclean: $(DIST_CLEAN_MOD) $(DIST_CLEAN_ENV)
	@echo "   RM $(SUBDIR_MAIN)/bin"
	@$(RM) $(SUBDIR_MAIN)/bin
	@$(MAKE) $(PARALLEL) -C $(SUBDIR_KERN) clean

##################################################
# Compilation du module kernel
kmodules:
	@$(MAKE) $(PARALLEL) -C $(SUBDIR_KERN)

print-%:
	@echo $* = $($(*))

.PHONY: all main modules env clean distclean kmodules

