ESDK=${EPIPHANY_HOME}
ELDF=${ESDK}/bsps/current/fast.ldf
ELDF=ebsp_fast.ldf

# ARCH will be either x86_64, x86, or armv7l (parallella)
ARCH=$(shell uname -m)

ifeq ($(ARCH),x86_64)
PLATFORM_PREFIX=arm-linux-gnueabihf-
else
PLATFORM_PREFIX=
endif

HOST_LIBNAME = libhost-bsp
E_LIBNAME	= libe-bsp
LIBEXT = .a

EXT_LIBS = -Le-lib

INCLUDES = -I/usr/arm-linux-gnueabihf/include \
		   -I./include \
		   -I${ESDK}/tools/host/include
HOST_LIBS=-L${ESDK}/tools/host/lib

E_SRCS = \
		 e_bsp.c \
		 e_bsp_drma.c \
		 e_bsp_mp.c \
		 e_bsp_memory.c

HOST_SRCS = \
		 host_bsp.c

E_OBJS = $(E_SRCS:%.c=bin/e/%.o) 
E_ASMS = $(E_SRCS:%.c=bin/e/%.s)
HOST_OBJS = $(HOST_SRCS:%.c=bin/host/%.o) 

########################################################

vpath %.c src

bin/host/%.o: %.c
	mkdir -p bin/host bin/lib
	$(PLATFORM_PREFIX)gcc -O3 -Wall -std=c99 $(INCLUDES) -c $< -o $@ ${HOST_LIBS} -le-hal
	
bin/e/%.o: %.c
	mkdir -p bin/e bin/lib
	e-gcc -Os -fno-strict-aliasing -std=c99 -Wall -T ${ELDF} $(INCLUDES) -c $< -o $@ ${HOST_LIBS} -le-lib

bin/e/%.s: %.c
	mkdir -p bin/e
	e-gcc -fverbose-asm -Os -fno-strict-aliasing -std=c99 -Wall -T ${ELDF} $(INCLUDES) -S $< -o $@ -le-lib

all: host e

host: bin/lib/$(HOST_LIBNAME)$(LIBEXT)

e: bin/lib/$(E_LIBNAME)$(LIBEXT)

assembly: $(E_ASMS)

bin/lib/$(HOST_LIBNAME)$(LIBEXT): $(HOST_OBJS)
	$(PLATFORM_PREFIX)ar rvs $@ $^ 

bin/lib/$(E_LIBNAME)$(LIBEXT): $(E_OBJS)
	e-ar rvs $@ $^ 

sizecheck: src/sizeof_check.cpp
	@echo "-----------------------"
	@echo "Sizecheck using e-g++"
	@echo "-----------------------"
	e-g++ -Wall $(INCLUDES) -c $< -o bin/sizecheck
	@echo "-----------------------"
	@echo "Sizecheck using g++"
	@echo "-----------------------"
	$(PLATFORM_PREFIX)-g++ -Wall $(INCLUDES) -c $< -o bin/sizecheck

########################################################

clean:
	rm bin/lib/*.a
	rm bin/*/*.o
