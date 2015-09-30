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

all: hello e_hello primitives streaming dot_product streaming_dot_product memtest bspbench lu_decomposition memorybench

########################################################

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

streaming_dot_product: bin/streaming_dot_product bin/streaming_dot_product/host_streaming_dot_product bin/streaming_dot_product/e_streaming_dot_product.elf bin/streaming_dot_product/e_streaming_dot_product.srec

bin/streaming_dot_product:
	@mkdir -p bin/streaming_dot_product

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

memorybench: bin/memorybench bin/memorybench/host_memorybench bin/memorybench/e_memorybench.elf bin/memorybench/e_memorybench.srec bin/memorybench/e_memorybench.s

bin/memorybench:
	@mkdir -p bin/memorybench

########################################################

cannon: bin/cannon bin/cannon/host_cannon bin/cannon/e_cannon.elf bin/cannon/e_cannon.srec cannon/common.h

bin/cannon:
	@mkdir -p bin/cannon

########################################################

primitives: bin/primitives bin/primitives/host_primitives bin/primitives/e_primitives.elf bin/primitives/e_primitives.srec

bin/primitives:
	@mkdir -p bin/primitives

########################################################

streaming: bin/streaming bin/streaming/host_streaming bin/streaming/e_streaming.elf bin/streaming/e_streaming.srec

bin/streaming:
	@mkdir -p bin/streaming


########################################################

primes: bin/primes bin/primes/host_primes bin/primes/e_primes.elf bin/primes/e_primes.srec

bin/primes:
	@mkdir -p bin/primes

########################################################

clean:
	rm -r bin

