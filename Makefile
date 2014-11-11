ESDK=${EPIPHANY_HOME}
ELIBS="-L ${ESDK}/tools/host/lib"
ELDF=${ESDK}/bsps/current/fast.ldf

HOST_LIBNAME = host-bsp
E_LIBNAME	= e-bsp
LIBEXT = .a

EXT_LIBS = -Le-lib

INCLUDES = \
		   -I./include\
		   -I${ESDK}/tools/host/include

E_SRCS = \
		 bsp.c

HOST_SRCS = \
		 bsp_host.c

E_OBJS = $(E_SRCS:%.c=bin/e/%.o) 
HOST_OBJS = $(HOST_SRCS:%.c=bin/host/%.o) 

vpath %.c src

bin/host/%.o: %.c
	gcc $(INCLUDES) -c $< -o $@ ${ELIBS} -le-hal
	
bin/e/%.o: %.c
	e-gcc -T ${ELDF} $(INCLUDES) -c $< -o $@ -le-lib

all: host e

host: bin/lib/$(HOST_LIBNAME)$(LIBEXT)

e: bin/lib/$(E_LIBNAME)$(LIBEXT)

bin/lib/$(HOST_LIBNAME)$(LIBEXT): $(HOST_OBJS)
	ar rvs $@ $^ 

bin/lib/$(E_LIBNAME)$(LIBEXT): $(E_OBJS)
	e-ar rvs $@ $^ 

clean:
	rm bin/lib/*.a
	rm bin/*/*.o
