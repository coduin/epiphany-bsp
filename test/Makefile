ESDK=${EPIPHANY_HOME}
ELDF=${ESDK}/bsps/current/fast.ldf
ELDF=../ebsp_fast.ldf

# ARCH will be either x86_64, x86, or armv7l (parallella)
ARCH=$(shell uname -m)

ifeq ($(ARCH),x86_64)
PLATFORM_PREFIX=arm-linux-gnueabihf-
else
PLATFORM_PREFIX=
endif

CFLAGS=-std=c99 -O3 -ffast-math -Wall

INCLUDES = \
		   -I../include\
		   -I${ESDK}/tools/host/include

LIBS = \
	 -L../bin/lib

HOST_LIBS = \
	 -L /usr/arm-linux-gnueabihf/lib \
	 -L${ESDK}/tools/host/lib

E_LIBS = \
	 -L${ESDK}/tools/host/lib

HOST_LIB_NAMES = -lhost-bsp -le-hal -le-loader

E_LIB_NAMES = -le-bsp -le-lib

########################################################

all: bsp_pid bsp_init

dirs:
	mkdir -p bin

########################################################

bin/%.srec: bin/%.elf
	e-objcopy --srec-forceS3 --output-target srec $< $@

########################################################

bsp_pid: bin/host_bsp_pid bin/e_bsp_pid.elf bin/e_bsp_pid.srec

bin/host_bsp_pid: bsp_pid/host_bsp_pid.c
	$(PLATFORM_PREFIX)gcc $(CFLAGS) $(INCLUDES) -o $@ $< $(LIBS) $(HOST_LIBS) $(HOST_LIB_NAMES)
	
bin/e_bsp_pid.elf: bsp_pid/e_bsp_pid.c
	e-gcc $(CFLAGS) -T ${ELDF} $(INCLUDES) -o $@ $< $(LIBS) $(E_LIBS) $(E_LIB_NAMES)

########################################################

bsp_init: bin/host_bsp_init bin/e_bsp_init.elf bin/e_bsp_init.srec

bin/host_bsp_init: bsp_init/host_bsp_init.c
	$(PLATFORM_PREFIX)gcc $(CFLAGS) $(INCLUDES) -o $@ $< $(LIBS) $(HOST_LIBS) $(HOST_LIB_NAMES)
	
bin/e_bsp_init.elf: bsp_init/e_bsp_init.c
	e-gcc $(CFLAGS) -T ${ELDF} $(INCLUDES) -o $@ $< $(LIBS) $(E_LIBS) $(E_LIB_NAMES)

########################################################

clean:
	rm bin/*
