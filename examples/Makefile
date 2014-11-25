ESDK=${EPIPHANY_HOME}
ELDF=${ESDK}/bsps/current/fast.ldf

INCLUDES = \
		   -I../include

LIBS = \
	 -L../bin/lib

HOST_LIBS = \
	 -L${ESDK}/tools/host/lib

E_LIBS = \
	 -L${ESDK}/tools/host/lib

########################################################

all: hello

hello: bin/hello_host bin/hello

bin/hello_host: hello/hello.c
	gcc $(INCLUDES) -o $@ $< $(LIBS) $(HOST_LIBS) -lhost-bsp -le-hal
	
bin/hello: hello/e-hello.c
	e-gcc -T ${ELDF} $(INCLUDES) -o $@ $< $(LIBS) $(E_LIBS) -le-bsp -le-lib

########################################################

clean:
	rm bin/*
