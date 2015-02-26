ESDK=${EPIPHANY_HOME}
ELDF=${ESDK}/bsps/current/fast.ldf

CFLAGS = -std=c99

HOST_LIBNAME = libhost-bsp
E_LIBNAME	= libe-bsp
LIBEXT = .a

EXT_LIBS = -Le-lib

INCLUDES = \
		   -I./include\
		   -I${ESDK}/tools/host/include
HOST_LIBS=-L${ESDK}/tools/host/lib

E_SRCS = \
		 e_bsp.c

HOST_SRCS = \
		 host_bsp.c \
		 host_bsp_inspector.c

E_OBJS = $(E_SRCS:%.c=bin/e/%.o) 
HOST_OBJS = $(HOST_SRCS:%.c=bin/host/%.o) 

########################################################

vpath %.c src

bin/host/%.o: %.c
	gcc $(CFLAGS) $(INCLUDES) -c $< -o $@ ${HOST_LIBS} -le-hal -lncurses
	
bin/e/%.o: %.c
	e-gcc $(CFLAGS) -T ${ELDF} $(INCLUDES) -c $< -o $@ ${HOST_LIBS} -le-lib

all: host e

host: bin/lib/$(HOST_LIBNAME)$(LIBEXT)

e: bin/lib/$(E_LIBNAME)$(LIBEXT)

bin/lib/$(HOST_LIBNAME)$(LIBEXT): $(HOST_OBJS)
	ar rvs $@ $^ 

bin/lib/$(E_LIBNAME)$(LIBEXT): $(E_OBJS)
	e-ar rvs $@ $^ 

########################################################

clean:
	rm bin/lib/*.a
	rm bin/*/*.o
