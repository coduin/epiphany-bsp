HOST_LIBNAME = host-bsp
E_LIBNAME	= e-bsp
LIBEXT = .a

EXT_LIBS = -Le-lib

INCLUDES = \
		   -I./include \

E_SRCS = \
		 bsp.c

HOST_SRCS = \
		 bsp_host.c

E_OBJS = $(E_SRCS:%.c=bin/e/%.o) 
HOST_OBJS = $(HOST_SRCS:%.c=bin/host/%.o) 

######################################################33

vpath %.c src

bin/host/%.o: %.c
	gcc $(INCLUDES) -c $< -o $@ 
	
bin/e/%.o: %.c
	e-gcc $(INCLUDES) -c $< -o $@

######################################################33

all: host e

host: bin/lib/$(HOST_LIBNAME)$(LIBEXT)

e: bin/lib/$(E_LIBNAME)$(LIBEXT)

bin/lib/$(HOST_LIBNAME)$(LIBEXT): $(HOST_OBJS)
	ar rvs $@ $^ 

bin/lib/$(E_LIBNAME)$(LIBEXT): $(E_OBJS)
	e-ar rvs $@ $^ 

######################################################33
