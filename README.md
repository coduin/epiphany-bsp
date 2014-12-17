# Introduction

The [Bulk Synchronous Parallel (BSP)](http://en.wikipedia.org/wiki/Bulk_synchronous_parallel)
computing model is a model for designing parallel algorithms. It provides
a description of how parallel computation should be 
carried out. Programs written with this model consist of
a number of supersteps, which in turn consist of a local 
computations and non-blocking communication. A (barrier)
synchronisation at the end of such a step is used to guarantee
occurance of all communications within a step.

This library provides an implementation of the model on top of the Epiphany SDK. 
This allows the BSP computing model to be used with the Epiphany
architecture developed by [Adapteva](http://www.adapteva.com).
In particular this library has been tested on the 
[Parallella](http://www.parallella.org) board. Our goal is to
allow current BSP programs to be run on the Epiphany architecture
with minimal modifications. We believe the BSP model is a good starting point
for anyone new to parallel algorithms, allowing people to get their hands dirty
with the Epiphany architecture, without having to resort on the at times cumbersome
Epiphany SDK.

# Building

Currently this is still a very early work in progress, and
build methods are still undergoing heavy changes. Feel free
to try and get things running yourself. The library and examples
are built seperately. Use:

    $ make

to build the library. Then `cd examples` and run

    $ make

again to build the libraries. The examples are to be run from the
examples folder (required in order to resolve the Epiphany binaries) For example:

    $ ./bin/hello_host

Runs the Hello World example.
