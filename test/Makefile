ESDK=${EPIPHANY_HOME}
ELDF=${ESDK}/bsps/current/fast.ldf
ELDF=../ebsp_fast.ldf

# ARCH will be either x86_64, x86, or armv7l (parallella)
ARCH=$(shell uname -m)

ifeq ($(ARCH),x86_64)
ARM_PLATFORM_PREFIX=arm-linux-gnueabihf-
E_PLATFORM_PREFIX  =epiphany-elf-
else
ARM_PLATFORM_PREFIX=
E_PLATFORM_PREFIX  =e-
endif

# no-tree-loop-distribute-patters makes sure the compiler
# does NOT replace loops with calls to memcpy, residing in external memory
CFLAGS=-std=c99 -Wall -O3 -ffast-math -fno-tree-loop-distribute-patterns

#First include directory is only for cross-compiling
INCLUDES = -I/usr/include/esdk \
		   -I../include\
		   -I${ESDK}/tools/host/include

LIBS = \
	 -L../lib

HOST_LIBS = \
	 -L /usr/arm-linux-gnueabihf/lib \
	 -L${ESDK}/tools/host/lib

E_LIBS = \
	 -L${ESDK}/tools/host/lib

HOST_LIB_NAMES = -lhost-bsp -le-hal -le-loader

E_LIB_NAMES = -le-bsp -le-lib

########################################################

all: dirs tests

tests: bsp_time bsp_nprocs bsp_pid bsp_init bsp_hpput bsp_local_mp bsp_vertical_mp bsp_variables bsp_hp_variables bsp_utility bsp_streams bsp_dma bsp_memory bsp_abort

dirs:
	@mkdir -p bin

########################################################

cfiles := $(shell find -iname '*.c')

vpath %.c $(dir $(cfiles))

bin/%: %.c
	@echo "CC $<"
	@$(ARM_PLATFORM_PREFIX)gcc $(CFLAGS) $(INCLUDES) -o $@ $< $(LIBS) $(HOST_LIBS) $(HOST_LIB_NAMES)

bin/%.elf: %.c
	@echo "CC $<"
	@$(E_PLATFORM_PREFIX)gcc $(CFLAGS) -T ${ELDF} $(INCLUDES) -o $@ $< $(LIBS) $(E_LIBS) $(E_LIB_NAMES)

bin/%.s: %.c
	@echo "CC $<"
	@$(E_PLATFORM_PREFIX)gcc $(CFLAGS) -T $(ELDF)  $(INCLUDES) -fverbose-asm -S $< -o $@ $(LIBS) $(E_LIBS) $(E_LIB_NAMES)

########################################################

bsp_time:               bin/e_bsp_time.elf          bin/host_bsp_time
bsp_nprocs:             bin/e_bsp_nprocs.elf        bin/host_bsp_nprocs
bsp_pid:                bin/e_bsp_pid.elf           bin/host_bsp_pid
bsp_init:               bin/e_bsp_init.elf          bin/host_bsp_init
bsp_hpput:              bin/e_bsp_hpput.elf         bin/host_bsp_hpput
bsp_local_mp:           bin/e_bsp_local_mp.elf      bin/host_bsp_local_mp
bsp_vertical_mp:        bin/e_bsp_vertical_mp.elf   bin/host_bsp_vertical_mp
bsp_variables:          bin/e_bsp_variables.elf     bin/host_bsp_variables
bsp_hp_variables:       bin/e_bsp_hp_variables.elf  bin/host_bsp_hp_variables
bsp_utility:            bin/e_bsp_utility.elf       bin/host_bsp_utility
bsp_streams:            bin/e_bsp_streams.elf       bin/host_bsp_streams
bsp_dma:                bin/e_bsp_dma.elf           bin/host_bsp_dma
bsp_memory:             bin/e_bsp_memory.elf        bin/host_bsp_memory
bsp_abort:              bin/e_bsp_abort.elf         bin/host_bsp_abort

########################################################

clean:
	rm bin/*
