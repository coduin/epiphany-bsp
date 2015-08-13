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

all: bsp_time bsp_nprocs bsp_pid bsp_init bsp_memtest bsp_hpput

dirs:
	mkdir -p bin

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

bin/%.srec: bin/%.elf
	@$(E_PLATFORM_PREFIX)objcopy --srec-forceS3 --output-target srec $< $@

########################################################

bsp_time:   bin/e_bsp_time.elf      bin/e_bsp_time.srec     bin/host_bsp_time
bsp_nprocs: bin/e_bsp_nprocs.elf    bin/e_bsp_nprocs.srec   bin/host_bsp_nprocs
bsp_pid:    bin/e_bsp_pid.elf       bin/e_bsp_pid.srec      bin/host_bsp_pid
bsp_init:   bin/e_bsp_init.elf      bin/e_bsp_init.srec     bin/host_bsp_init
bsp_memtest:bin/e_bsp_memtest.elf   bin/e_bsp_memtest.srec  bin/host_bsp_memtest
bsp_hpput:  bin/e_bsp_hpput.elf     bin/e_bsp_hpput.srec    bin/host_bsp_hpput

########################################################

clean:
	rm bin/*
