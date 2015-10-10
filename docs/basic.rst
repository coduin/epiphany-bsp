.. sectionauthor:: Jan-Willem Buurlage <janwillem@buurlagewits.nl>

.. highlight:: c

Getting started: The Basics
===========================

EBSP programs are written in single-program multiple-data *(SPMD)* style. This means that each core runs the same code, but obtains different data. Later we will see how we can transfer data to and from the Epiphany cores, but for now our first step will be to get the cores to output their designated core number (called `pid` for *processor identifier*). Like all programs written for the Parallella, an EBSP program consists of two parts. One part contains the code that runs on the *host processor*, the ARM chip that hosts the Linux OS. The other part contains the code that runs on each Epiphany core. In heterogeneous computing it is common to call this second part the *kernel*.

Hello World!
------------

A host program consists of at least four EBSP functions, which are generally used as in the following example:::

    // file: host_code.c

    #include <host_bsp.h>

    int main(int argc, char **argv)
    {
        bsp_init("ecore_program.srec", argc, argv);
        bsp_begin(16);
        ebsp_spmd();
        bsp_end();

        return 0;
    }

The first call to `bsp_init` initializes the EBSP library. The first argument is the filename of the (compiled) kernel program, and the second and third arguments are the program arguments. Next we tell the EBSP system how many cores we would like to use (in this case; all 16 cores for a standard Parallella board) by calling `bsp_begin` passing `16` as its first argument. The call to `ebsp_spmd` starts the execution of the kernel program on the 16 cores. When the execution has finished we finalize the EBSP system by calling `bsp_end` without arguments.

Next we write the kernel for our Hello World program. Besides outputting "Hello World" we also show the processor number. The code looks like this:::

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

Let us also go over the kernel code line by line. First we initialize the EBSP system on the core, by calling `bsp_begin`. In a kernel program this call does not require any arguments, since there is no additional program to run! Next we obtain information about our own designated processor number (commonly called `s`) using `bsp_pid`, and the total number of processors (commonly called `p`) by calling `bsp_nprocs`. We then output a message to host using `ebsp_message`. This function can be used completely identically to `printf` in ordinary C programs. Again we finalize the system with a call to `bsp_end` which cleans up the EBSP system.

You may have noticed that some EBSP functions, which we will refer to as *primitives*, are prefixed with `bsp_` while others are prefixed by `ebsp_`. This is because the EBSP library introduces some functions that are not in the `BSPlib standard <http://www.bsp-worldwide.org/>`_ but that can be very helpful when programming for the Epiphany.

Running this program should result in output similar to the following:::

    $08: Hello World from processor 8 / 16
    $01: Hello World from processor 1 / 16
    $07: Hello World from processor 7 / 16
    $02: Hello World from processor 2 / 16
    $15: Hello World from processor 15 / 16
    $03: Hello World from processor 3 / 16
    $10: Hello World from processor 10 / 16
    $06: Hello World from processor 6 / 16
    $12: Hello World from processor 12 / 16
    $13: Hello World from processor 13 / 16
    $05: Hello World from processor 5 / 16
    $04: Hello World from processor 4 / 16
    $11: Hello World from processor 11 / 16
    $14: Hello World from processor 14 / 16
    $09: Hello World from processor 9 / 16
    $00: Hello World from processor 0 / 16

The output has the form `$[pid]: output`. As we see, indeed the EBSP kernel is being run on every core! Note that there are no guarantees about which core gets to the `ebsp_message` statement first, and therefore the output need not be in order of processor number.

Interface (Basics)
------------------

Host
^^^^

.. doxygenfunction:: bsp_init
   :project: ebsp_host

.. doxygenfunction:: bsp_begin
   :project: ebsp_host

.. doxygenfunction:: ebsp_spmd
   :project: ebsp_host

.. doxygenfunction:: bsp_end
   :project: ebsp_host

Epiphany
^^^^^^^^

.. doxygenfunction:: bsp_begin
   :project: ebsp_e

.. doxygenfunction:: bsp_pid
   :project: ebsp_e

.. doxygenfunction:: bsp_nprocs
   :project: ebsp_e

.. doxygenfunction:: bsp_end
   :project: ebsp_e

.. doxygenfunction:: ebsp_message
   :project: ebsp_e
