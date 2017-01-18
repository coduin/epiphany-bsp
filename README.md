# Epiphany BSP

Epiphany BSP is a library for developing applications for the Parallella board. It is easy to setup and use, and provides powerful mechanisms for writing optimized parallel programs.

<p align="center">
<img alt="Epiphany BSP is a parallel library for the Parallella board" src="http://codu.in/img/epiphany-bsp-illustration-trimmed-small.jpg" />
</p>

The [Bulk Synchronous Parallel (BSP)](http://en.wikipedia.org/wiki/Bulk_synchronous_parallel)
computing model is a model for designing parallel algorithms.  This library (EBSP) provides an
implementation of the model on top of the Epiphany SDK (ESDK).  This allows the BSP computing model
to be used with the Epiphany architecture developed by [Adapteva](http://www.adapteva.com).
In particular this library has been implemented and tested on the  [Parallella](http://www.parallella.org) board.

## Example usage:

```C
// file: host_code.c

#include <host_bsp.h>

int main(int argc, char **argv)
{
    bsp_init("ecore_program.elf", argc, argv);
    bsp_begin(16);
    ebsp_spmd();
    bsp_end();

    return 0;
}

// file: ecore_code.c

#include <e_bsp.h>

int main()
{
    bsp_begin();
    int s = bsp_pid();
    int p = bsp_nprocs();
    ebsp_message("Hello World from processor %d / %d", s, p);
    bsp_end();

    return 0;
}
```

Running this program results in the output:

    $08: Hello World from processor 8 / 16
    $01: Hello World from processor 1 / 16
    $07: Hello World from processor 7 / 16
    ...

## License

Epiphany BSP is released under the LGPLv3. See the file `COPYING` for details.

## Documentation

Detailed documentation is provided at <http://www.codu.in/ebsp/docs>.

## Installing EBSP

If you want to write EBSP programs you need to have access to a Parallella board with a recent version of the Epiphany SDK (ESDK) installed.

### Cloning example project

An easy way to get started with EBSP is to clone the [EBSP example project](http://www.github.com/coduin/ebsp-empty-project). See the README bundled with the example project for details.

### Using EBSP manually

Alternatively you can download the [latest release](https://github.com/coduin/epiphany-bsp/releases). The EBSP library depends on the ESDK, and uses a custom linker file `ebsp_fast.ldf`. Your host program should link against `-lhost-bsp -le-hal -le-loader`. Your Epiphany kernel should link against `-le-bsp -le-lib` and use the linker script found at `ext/bsp/ebsp_fast.ldf`. For your convience we provide an example Makefile below:

```Makefile
ESDK=${EPIPHANY_HOME}
ELDF=ext/bsp/ebsp_fast.ldf

CFLAGS=-std=c99 -O3 -ffast-math -Wall

INCLUDES = -Iext/bsp/include \
           -I${ESDK}/tools/host/include

HOST_LIBS = -Lext/bsp/lib \
            -L${ESDK}/tools/host/lib \
            -L/usr/arm-linux-gnueabihf/lib

E_LIBS = -Lext/bsp/lib \
         -L${ESDK}/tools/host/lib

HOST_LIB_NAMES = -lhost-bsp -le-hal -le-loader

E_LIB_NAMES = -le-bsp -le-lib

all: bin bin/host_program bin/ecore_program.elf

bin:
    @mkdir -p bin

bin/host_program: src/host_code.c
    @echo "CC $<"
    @gcc $(CFLAGS) $(INCLUDES) -o $@ $< $(HOST_LIBS) $(HOST_LIB_NAMES)

bin/ecore_program.elf: src/ecore_code.c
    @echo "CC $<"
    @e-gcc $(CFLAGS) -T ${ELDF} $(INCLUDES) -o $@ $< $(E_LIBS) $(E_LIB_NAMES)

clean:
    rm -r bin
```

To run programs built with EBSP you run the host program. The call to `bsp_init()` will load the appropriate Epiphany kernel on the coprocessor.

## Building from source

The `master` branch contains the latest release. An (unstable) snapshot of the current development can be found in the `develop` branch. To manually build the library, issue `make` from the root directory of the library. The library only depends on the ESDK which should come preinstalled on your Parallella board. The examples and tests are built separately.

## About Coduin

Coduin (formerly Buurlage Wits) is a small company based in Utrecht, the Netherlands. Next to our work on software libraries and models for many-core processors in embedded systems, we are also active in the area of data analysis and predictive modelling.

If you are using EBSP, or have any questions, remarks or ideas then please get in touch at info@buurlagewits.nl! We would very much like to hear from you.

## Authors

- Tom Bannink
- Abe Wits
- Jan-Willem Buurlage.

Also thanks to:

- Máté Karácsony

## Issues

 The [issue tracker](https://github.com/coduin/epiphany-bsp/issues) is hosted on GitHub. We welcome pull requests, please pull request against the develop branch and add your name to the authors section of this README. Read [the GitHub flow guide](https://guides.github.com/introduction/flow/) for details.
