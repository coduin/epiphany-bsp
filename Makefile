# Epiphany compiler
E_COMPILE=epiphany-elf-

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

vpath %.c src

# Build sources
bin/host/%.o: %.c
	gcc -o $@ $<

bin/e/%.o: %.c
	e-gcc -o $@ $<

# Targets
host: bin/lib/$(HOST_LIBNAME)$(LIBEXT)
e: bin/lib/$(E_LIBNAME)$(LIBEXT)

bin/lib/$(HOST_LIBNAME)$(LIBEXT): $(HOST_OBJS)
	ar rvs $@ $^ 

# Link
bin/lib/$(E_LIBNAME)$(LIBEXT): $(E_OBJS)
	e-ar rvs $@ $^ 
