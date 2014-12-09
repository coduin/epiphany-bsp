ESDK=${EPIPHANY_HOME}
ELDF=${ESDK}/bsps/current/fast.ldf

INCLUDES = \
		   -I../include\
		   -I${ESDK}/tools/host/include

LIBS = \
	 -L../bin/lib

HOST_LIBS = \
	 -L${ESDK}/tools/host/lib

E_LIBS = \
	 -L${ESDK}/tools/host/lib

########################################################

all: hello e_hello

########################################################

hello: bin/hello_host bin/hello.elf bin/hello.srec

bin/hello_host: hello/hello.c
	gcc $(INCLUDES) -o $@ $< $(LIBS) $(HOST_LIBS) -lhost-bsp -le-hal -le-loader
	
bin/hello.elf: hello/e-hello.c
	e-gcc -T ${ELDF} $(INCLUDES) -o $@ $< $(LIBS) $(E_LIBS) -le-bsp -le-lib

bin/hello.srec: bin/hello.elf
	e-objcopy --srec-forceS3 --output-target srec bin/hello.elf bin/hello.srec

########################################################

e_hello: bin/e_hello_host bin/e_hello.elf bin/e_hello.srec

bin/e_hello_host: e_hello/e_hello.c
	gcc $(INCLUDES) -o $@ $< $(LIBS) $(HOST_LIBS) -le-hal -le-loader
	
bin/e_hello.elf: e_hello/e_ehello.c
	e-gcc -T ${ELDF} $(INCLUDES) -o $@ $< $(LIBS) $(E_LIBS) -le-lib

bin/e_hello.srec: bin/e_hello.elf
	e-objcopy --srec-forceS3 --output-target srec bin/e_hello.elf bin/e_hello.srec

########################################################

clean:
	rm bin/*
