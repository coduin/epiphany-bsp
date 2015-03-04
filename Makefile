ESDK=${EPIPHANY_HOME}
ELDF=${ESDK}/bsps/current/fast.ldf

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
	mkdir -p bin/host bin/lib
	gcc -O3 -std=c99 $(INCLUDES) -c $< -o $@ ${HOST_LIBS} -le-hal -lncurses
	
bin/e/%.o: %.c
	mkdir -p bin/e bin/lib
	e-gcc -O3 -fno-strict-aliasing -std=c99 -T ${ELDF} $(INCLUDES) -c $< -o $@ ${HOST_LIBS} -le-lib
	#ALL: does work with -Os, but not with -O or -O1
	#e-gcc -O1 -fthread-jumps -falign-labels -fprefetch-loop-arrays -falign-jumps -fcrossjumping -fcse-skip-blocks -fdevirtualize -fgcse -finline-small-functions -fipa-sra -fpartial-inlining -fregmove -freorder-functions -fsched-interblock -falign-functions -falign-loops -fcaller-saves -fcse-follow-jumps -fdelete-null-pointer-checks -fexpensive-optimizations -fgcse-lm -findirect-inlining -foptimize-sibling-calls -fpeephole2 -freorder-blocks -freorder-blocks-and-partition -frerun-cse-after-loop -fsched-spec -fschedule-insns -fstrict-aliasing -ftree-switch-conversion -ftree-vrp -fschedule-insns2 -fstrict-overflow -ftree-pre -ftree-vect-loop-version $(CFLAGS) -T ${ELDF} $(INCLUDES) -c $< -o $@ ${HOST_LIBS} -le-lib
	#NOT fstrict-aliasing, does work with -O, -O1, -Os
	#e-gcc -O1 -fthread-jumps -falign-labels -fprefetch-loop-arrays -falign-jumps -fcrossjumping -fcse-skip-blocks -fdevirtualize -fgcse -finline-small-functions -fipa-sra -fpartial-inlining -fregmove -freorder-functions -fsched-interblock -falign-functions -falign-loops -fcaller-saves -fcse-follow-jumps -fdelete-null-pointer-checks -fexpensive-optimizations -fgcse-lm -findirect-inlining -foptimize-sibling-calls -fpeephole2 -freorder-blocks -freorder-blocks-and-partition -frerun-cse-after-loop -fsched-spec -fschedule-insns -ftree-switch-conversion -ftree-vrp -fschedule-insns2 -fstrict-overflow -ftree-pre -ftree-vect-loop-version $(CFLAGS) -T ${ELDF} $(INCLUDES) -c $< -o $@ ${HOST_LIBS} -le-lib
	#WORKS
	#e-gcc -O -fthread-jumps -falign-labels -fprefetch-loop-arrays -falign-jumps -fcrossjumping -fcse-skip-blocks -fdevirtualize -fgcse -finline-small-functions -fipa-sra -fpartial-inlining -fregmove -freorder-functions -fsched-interblock -falign-functions -falign-loops -fcaller-saves -fcse-follow-jumps -fdelete-null-pointer-checks -fexpensive-optimizations -fgcse-lm -findirect-inlining -foptimize-sibling-calls -fpeephole2 -freorder-blocks -freorder-blocks-and-partition -frerun-cse-after-loop -fsched-spec -fschedule-insns -fstrict-aliasing $(CFLAGS) -T ${ELDF} $(INCLUDES) -c $< -o $@ ${HOST_LIBS} -le-lib
	#NOT WORKING
	#e-gcc -O -fthread-jumps -falign-labels -fprefetch-loop-arrays -falign-jumps -fcrossjumping -fcse-skip-blocks -fdevirtualize -fgcse -finline-small-functions -fipa-sra -fpartial-inlining -fregmove -freorder-functions -fsched-interblock -falign-functions -falign-loops -fcaller-saves -fcse-follow-jumps -fdelete-null-pointer-checks -fexpensive-optimizations -fgcse-lm -findirect-inlining -foptimize-sibling-calls -fpeephole2 -freorder-blocks -freorder-blocks-and-partition -frerun-cse-after-loop -fsched-spec -fschedule-insns $(CFLAGS) -T ${ELDF} $(INCLUDES) -c $< -o $@ ${HOST_LIBS} -le-lib
	#e-gcc -Os -T ${ELDF} $(INCLUDES) -c $< -o $@ ${HOST_LIBS} -le-lib
	#e-gcc $(CFLAGS) -T ${ELDF} $(INCLUDES) -c $< -o $@ ${HOST_LIBS} -le-lib

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
