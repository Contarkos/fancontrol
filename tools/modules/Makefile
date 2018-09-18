# Makefile pour compiler tous les modules kernel

KERNEL_MODULES := $(filter-out Makefile, $(wildcard *))

K_SRC := $(filter-out %.mod.c, $(foreach dir,$(KERNEL_MODULES), $(wildcard $(dir)/*.c)))
K_OBJ := $(patsubst %.c, %.ko, $(K_SRC))

%.ko:
	@$(MAKE) -C $(dir $@)

all: $(K_OBJ)
	cp -r $(K_OBJ) ../../DATA/

clean:
	@for directory in ${KERNEL_MODULES} ; do 	\
		$(MAKE) -C $$directory clean ;			\
	done

print-%:
	@echo $* = $($(*))

.PHONY: all clean
