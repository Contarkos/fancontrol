# Makefile générique pour les modules
TOOLCHAIN_DIR=/opt/arm-bcm2708/arm-linux-gnueabihf
TOOLCHAIN_DIR=/opt/armv6-rpi-linux-gnueabihf

ARCH = arm
ARCH_EXT = v6
DEBUG = -g
LIB_ROOTFS = $(TOOLCHAIN_DIR)/lib
BIN_ROOTFS = $(TOOLCHAIN_DIR)/include

CROSS_COMPILE = $(TOOLCHAIN_DIR)/bin/$(ARCH)$(ARCH_EXT)-linux-gnueabihf-

INCLUDES = \
	-Iinc \
	-Iapi

# Le fichier qui contient les dépendances dans chaque module
include include.mk

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
C_CPLUS_FLAGS += -mcpu=arm1176jzf-s -mfpu=vfp -mfloat-abi=hard -marm -march=armv6kz+fp
C_CPLUS_FLAGS += $(INTEG_LOG_LEVEL)

C_FLAGS 	+= $(C_CPLUS_FLAGS) -Wstrict-prototypes -std=c99
C_FLAGS 	+= -D_POSIX_C_SOURCE=199506L
CPLUS_FLAGS += $(C_CPLUS_FLAGS) -std=c++11

AR = ar
CC = gcc
CXX = g++
STRIP = strip
RM = rm -rf

LIB = lib$(notdir $(shell pwd)).a
OBJ_C = $(patsubst src/%.c, obj/%.o, $(wildcard src/*.c))
OBJ_CXX = $(patsubst src/%.cpp, obj/%.o, $(wildcard src/*.cpp))

BIN = sw_local.bin

LIB_DIR = lib
OBJ_DIR = obj
SRC_DIR = src
BIN_DIR = bin

# On compile une librairie statique
all: lib
	
bin: $(BIN)

lib: $(LIB)

obj_dir:
	@mkdir -p $(OBJ_DIR)

lib_dir:
	@mkdir -p $(LIB_DIR)

bin_dir:
	@mkdir -p $(BIN_DIR)

$(OBJ_C): | obj_dir

$(OBJ_CXX): | obj_dir

$(BIN): $(OBJ_C) $(OBJ_CXX) | bin_dir
	@echo "cc $^ $(LIBS) -o $@ "
	@$(CROSS_COMPILE)$(CXX) $(OBJ_C) $(OBJ_CXX) $(LIBS_PATH) $(LIBS) -o $(BIN_DIR)/$(BIN)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@echo "cc -c $^ -o $@"
	@$(CROSS_COMPILE)$(CC) $(INCLUDES) $(C_FLAGS) $(DEBUG) -c $^ -o $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp
	@echo "cxx -c $^ -o $@"
	@$(CROSS_COMPILE)$(CXX) $(INCLUDES) $(CPLUS_FLAGS) $(DEBUG) -c $^ -o $@

$(LIB): $(OBJ_C) $(OBJ_CXX) | lib_dir
	@echo "Creating $(LIB)..."
	@$(CROSS_COMPILE)$(AR) rcs $(LIB_DIR)/$(LIB) $(OBJ_C) $(OBJ_CXX)

clean:
	$(RM) $(OBJ_C) $(OBJ_CXX)
	@$(RM) $(OBJ_DIR)

libclean: clean
	$(RM) $(LIB_DIR)/$(LIB)
	@$(RM) $(LIB_DIR)

distclean: clean
	$(RM) $(BIN_DIR)/$(BIN)
	@$(RM) $(BIN_DIR)

.PHONY: all lib bin obj_dir lib_dir bin_dir clean libclean distclean
