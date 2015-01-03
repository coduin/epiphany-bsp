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

HOST_LIB_NAMES = -lhost-bsp -le-hal -le-loader

E_LIB_NAMES = -le-bsp -le-lib

########################################################

all: hello e_hello dot_product puttest

########################################################

hello: bin/host_hello bin/e_hello.elf bin/e_hello.srec

bin/host_hello: hello/host_hello.c
	gcc $(INCLUDES) -o $@ $< $(LIBS) $(HOST_LIBS) $(HOST_LIB_NAMES)
	
bin/e_hello.elf: hello/e_hello.c
	e-gcc -T ${ELDF} $(INCLUDES) -o $@ $< $(LIBS) $(E_LIBS) $(E_LIB_NAMES)

bin/e_hello.srec: bin/e_hello.elf
	e-objcopy --srec-forceS3 --output-target srec $< $@

########################################################

e_hello: bin/host_e_hello bin/e_e_hello.elf bin/e_e_hello.srec

bin/host_e_hello: e_hello/host_e_hello.c
	gcc $(INCLUDES) -o $@ $< $(LIBS) $(HOST_LIBS) $(HOST_LIB_NAMES)
	
bin/e_e_hello.elf: e_hello/e_e_hello.c
	e-gcc -T ${ELDF} $(INCLUDES) -o $@ $< $(LIBS) $(E_LIBS) $(E_LIB_NAMES)

bin/e_e_hello.srec: bin/e_e_hello.elf
	e-objcopy --srec-forceS3 --output-target srec $< $@

########################################################

dot_product: bin/host_dot_product bin/e_dot_product.elf bin/e_dot_product.srec

bin/host_dot_product: dot_product/host_dot_product.c
	gcc $(INCLUDES) -o $@ $< $(LIBS) $(HOST_LIBS) $(HOST_LIB_NAMES)
	
bin/e_dot_product.elf: dot_product/e_dot_product.c
	e-gcc -T ${ELDF} $(INCLUDES) -o $@ $< $(LIBS) $(E_LIBS) $(E_LIB_NAMES)

bin/e_dot_product.srec: bin/e_dot_product.elf
	e-objcopy --srec-forceS3 --output-target srec $< $@

########################################################

puttest: bin/host_puttest bin/e_puttest.elf bin/e_puttest.srec

bin/host_puttest: puttest/host_puttest.c
	gcc $(INCLUDES) -o $@ $< $(LIBS) $(HOST_LIBS) $(HOST_LIB_NAMES)
	
bin/e_puttest.elf: puttest/e_puttest.c
	e-gcc -T ${ELDF} $(INCLUDES) -o $@ $< $(LIBS) $(E_LIBS) $(E_LIB_NAMES)

bin/e_puttest.srec: bin/e_puttest.elf
	e-objcopy --srec-forceS3 --output-target srec $< $@

########################################################


clean:
	rm bin/*
