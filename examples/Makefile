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
	 -L../lib

HOST_LIBS = \
	 -L /usr/arm-linux-gnueabihf/lib \
	 -L${ESDK}/tools/host/lib

E_LIBS = \
	 -L${ESDK}/tools/host/lib

HOST_LIB_NAMES = -lhost-bsp -le-hal -le-loader

E_LIB_NAMES = -le-bsp -le-lib

########################################################

all: hello e_hello primitives dot_product memtest bspbench lu_decomposition 

########################################################

bin/%: %.c
	@echo "CC $<"
	@$(PLATFORM_PREFIX)gcc $(CFLAGS) $(INCLUDES) -o $@ $< $(LIBS) $(HOST_LIBS) $(HOST_LIB_NAMES)
	
bin/%.elf: %.c
	@echo "CC $<"
	@e-gcc $(CFLAGS) -T ${ELDF} $(INCLUDES) -o $@ $< $(LIBS) $(E_LIBS) $(E_LIB_NAMES)

bin/%.srec: bin/%.elf
	@e-objcopy --srec-forceS3 --output-target srec $< $@

########################################################

hello: bin/hello bin/hello/host_hello bin/hello/e_hello.elf bin/hello/e_hello.srec

bin/hello:
	@mkdir -p bin/hello

########################################################

e_hello: bin/e_hello bin/e_hello/host_e_hello bin/e_hello/e_e_hello.elf bin/e_hello/e_e_hello.srec

bin/e_hello:
	@mkdir -p bin/e_hello

########################################################

dot_product: bin/dot_product bin/dot_product/host_dot_product bin/dot_product/e_dot_product.elf bin/dot_product/e_dot_product.srec

bin/dot_product:
	@mkdir -p bin/dot_product

########################################################

lu_decomposition: bin/lu_decomposition bin/lu_decomposition/host_lu_decomposition bin/lu_decomposition/e_lu_decomposition.elf bin/lu_decomposition/e_lu_decomposition.srec

bin/lu_decomposition:
	@mkdir -p bin/lu_decomposition

########################################################

memtest: bin/memtest bin/memtest/host_memtest bin/memtest/e_memtest.elf bin/memtest/e_memtest.srec

bin/memtest:
	@mkdir -p bin/memtest

########################################################

bspbench: bin/bspbench bin/bspbench/host_bspbench bin/bspbench/e_bspbench.elf bin/bspbench/e_bspbench.srec

bin/bspbench:
	@mkdir -p bin/bspbench

########################################################

primitives: bin/primitives bin/primitives/host_primitives bin/primitives/e_primitives.elf bin/primitives/e_primitives.srec

bin/primitives:
	@mkdir -p bin/primitives

########################################################

clean:
	rm -r bin
