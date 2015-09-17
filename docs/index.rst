.. Epiphany BSP documentation master file, created by
   sphinx-quickstart on Thu Sep 17 21:08:04 2015.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

Welcome to Epiphany BSP's documentation!
========================================

Contents:

.. toctree::
    :maxdepth: 2

    basic
    variables
    mp
    host_client
    streaming

    bsp

.. highlight:: c

Introduction
------------

The [Bulk Synchronous Parallel (BSP)](http://en.wikipedia.org/wiki/Bulk_synchronous_parallel)
computing model is a model for designing parallel algorithms. It provides
a description of how parallel computation should be 
carried out. Programs written with this model consist of
a number of supersteps, which in turn consist of local 
computations and non-blocking communication. A (barrier)
synchronisation at the end of such a step is used to guarantee
occurance of all communications within a step.

This library (EBSP) provides an implementation of the model on top of the Epiphany SDK (ESDK). 
This allows the BSP computing model to be used with the Epiphany
architecture developed by [Adapteva](http://www.adapteva.com).
In particular this library has been implemented and tested on the 
[Parallella](http://www.parallella.org) board. Our goal is to
allow current BSP programs to be run on the Epiphany architecture
with minimal modifications. We believe the BSP model is a good starting point
for anyone new to parallel algorithms, and our goal for this library is to provide an easy way
to implement parallel programs on the Epiphany architecture, without having to resort directly
to the Epiphany SDK.


Resources
---------

- \link bsp_model Information on the BSP model \endlink
- \link getting_started Getting started with EBSP \endlink
- **API Reference**: See e_bsp.h and host_bsp.h

Minimal example
---------------

Code for the host:::

    #include <host_bsp.h>
    #include <stdio.h>

    int main(int argc, char **argv)
    {
        // initialize the BSP system
        bsp_init("e_hello.srec", argc, argv);

        // show the number of processors available
        printf("bsp_nprocs(): %i\n", bsp_nprocs());

        // initialize the epiphany system, and load Epiphany binary
        bsp_begin(bsp_nprocs());

        // run the program on the Epiphany cores
        ebsp_spmd();

        // finalize
        bsp_end();

        return 0;
    }

Code for the Epiphany cores:::

    #include <e_bsp.h>

    int main()
    {
        bsp_begin();

        int n = bsp_nprocs(); 
        int p = bsp_pid();

        ebsp_message("Hello world from core %d/%d", p, n);

        bsp_end();

        return 0;
    }

Indices and tables
==================

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`

