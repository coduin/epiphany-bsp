# Root directory of epiphany-bsp repository
EBSP=..

ESDK=${EPIPHANY_HOME}
ELDF=${ESDK}/bsps/current/fast.ldf
ELDF=${EBSP}/ebsp_fast.ldf

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
		   -I${EBSP}/include\
		   -I${ESDK}/tools/host/include

LIBS = \
	 -L${EBSP}/lib

HOST_LIBS = \
	 -L /usr/arm-linux-gnueabihf/lib \
	 -L${ESDK}/tools/host/lib

E_LIBS = \
	 -L${ESDK}/tools/host/lib

HOST_LIB_NAMES = -lhost-bsp -le-hal -le-loader

E_LIB_NAMES = -le-bsp -le-lib

########################################################

all: cannon dot_product hello lu_decomposition primitives streaming streaming_dot_product

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

########################################################

hello: bin/hello bin/hello/host_hello bin/hello/e_hello.elf

bin/hello:
	@mkdir -p bin/hello

########################################################

dot_product: bin/dot_product bin/dot_product/host_dot_product bin/dot_product/e_dot_product.elf

bin/dot_product:
	@mkdir -p bin/dot_product

########################################################

streaming_dot_product: bin/streaming_dot_product bin/streaming_dot_product/host_streaming_dot_product bin/streaming_dot_product/e_streaming_dot_product.elf

bin/streaming_dot_product:
	@mkdir -p bin/streaming_dot_product

########################################################

lu_decomposition: bin/lu_decomposition bin/lu_decomposition/host_lu_decomposition bin/lu_decomposition/e_lu_decomposition.elf

bin/lu_decomposition:
	@mkdir -p bin/lu_decomposition

########################################################

cannon: bin/cannon bin/cannon/host_cannon bin/cannon/e_cannon.elf cannon/common.h

bin/cannon:
	@mkdir -p bin/cannon

########################################################

primitives: bin/primitives bin/primitives/host_primitives bin/primitives/e_primitives.elf

bin/primitives:
	@mkdir -p bin/primitives

########################################################

streaming: bin/streaming bin/streaming/host_streaming bin/streaming/e_streaming.elf

bin/streaming:
	@mkdir -p bin/streaming

########################################################

clean:
	rm -r bin

