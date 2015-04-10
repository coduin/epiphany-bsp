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

all: dirs hello e_hello primitives dot_product memtest bspbench lu_decomposition 

dirs:
	mkdir -p bin

########################################################

bin/%.srec: bin/%.elf
	e-objcopy --srec-forceS3 --output-target srec $< $@

########################################################

hello: bin/host_hello bin/e_hello.elf bin/e_hello.srec

bin/host_hello: hello/host_hello.c
	$(PLATFORM_PREFIX)gcc $(CFLAGS) $(INCLUDES) -o $@ $< $(LIBS) $(HOST_LIBS) $(HOST_LIB_NAMES)
	
bin/e_hello.elf: hello/e_hello.c
	e-gcc $(CFLAGS) -T ${ELDF} $(INCLUDES) -o $@ $< $(LIBS) $(E_LIBS) $(E_LIB_NAMES)

########################################################

e_hello: bin/host_e_hello bin/e_e_hello.elf bin/e_e_hello.srec

bin/host_e_hello: e_hello/host_e_hello.c
	$(PLATFORM_PREFIX)gcc $(CFLAGS) $(INCLUDES) -o $@ $< $(LIBS) $(HOST_LIBS) $(HOST_LIB_NAMES)
	
bin/e_e_hello.elf: e_hello/e_e_hello.c
	e-gcc $(CFLAGS) -T ${ELDF} $(INCLUDES) -o $@ $< $(LIBS) $(E_LIBS) $(E_LIB_NAMES)

########################################################

dot_product: bin/host_dot_product bin/e_dot_product.elf bin/e_dot_product.srec

bin/host_dot_product: dot_product/host_dot_product.c
	$(PLATFORM_PREFIX)gcc $(CFLAGS) $(INCLUDES) -o $@ $< $(LIBS) $(HOST_LIBS) $(HOST_LIB_NAMES)
	
bin/e_dot_product.elf: dot_product/e_dot_product.c
	e-gcc $(CFLAGS) -T ${ELDF} $(INCLUDES) -o $@ $< $(LIBS) $(E_LIBS) $(E_LIB_NAMES)

########################################################

lu_decomposition: bin/host_lu_decomposition bin/e_lu_decomposition.elf bin/e_lu_decomposition.srec

bin/host_lu_decomposition: lu_decomposition/host_lu_decomposition.c
	$(PLATFORM_PREFIX)gcc $(CFLAGS) $(INCLUDES) -o $@ $< $(LIBS) $(HOST_LIBS) $(HOST_LIB_NAMES)
	
bin/e_lu_decomposition.elf: lu_decomposition/e_lu_decomposition.c
	e-gcc $(CFLAGS) -T ${ELDF} $(INCLUDES) -o $@ $< $(LIBS) $(E_LIBS) $(E_LIB_NAMES)

########################################################

memtest: bin/host_memtest bin/e_memtest.elf bin/e_memtest.srec

bin/host_memtest: memtest/host_memtest.c
	$(PLATFORM_PREFIX)gcc $(CFLAGS) $(INCLUDES) -o $@ $< $(LIBS) $(HOST_LIBS) $(HOST_LIB_NAMES)
	
bin/e_memtest.elf: memtest/e_memtest.c
	e-gcc $(CFLAGS) -T ${ELDF} $(INCLUDES) -o $@ $< $(LIBS) $(E_LIBS) $(E_LIB_NAMES)

########################################################

bspbench: bin/host_bspbench bin/e_bspbench.elf bin/e_bspbench.srec

bin/host_bspbench: bspbench/host_bspbench.c
	$(PLATFORM_PREFIX)gcc $(CFLAGS) $(INCLUDES) -o $@ $< $(LIBS) $(HOST_LIBS) $(HOST_LIB_NAMES)
	
bin/e_bspbench.elf: bspbench/e_bspbench.c
	e-gcc $(CFLAGS) -T ${ELDF} $(INCLUDES) -o $@ $< $(LIBS) $(E_LIBS) $(E_LIB_NAMES)

########################################################

primitives: bin/host_primitives bin/e_primitives.elf bin/e_primitives.srec

bin/host_primitives: primitives/host_primitives.c
	$(PLATFORM_PREFIX)gcc $(CFLAGS) $(INCLUDES) -o $@ $< $(LIBS) $(HOST_LIBS) $(HOST_LIB_NAMES)
	
bin/e_primitives.elf: primitives/e_primitives.c
	e-gcc $(CFLAGS) -T ${ELDF} $(INCLUDES) -o $@ $< $(LIBS) $(E_LIBS) $(E_LIB_NAMES)

########################################################

clean:
	rm bin/*
