# Introduction

The [Bulk Synchronous Parallel (BSP)](http://en.wikipedia.org/wiki/Bulk_synchronous_parallel)
computing model is a model for designing parallel algorithms. It provides
a description of how parallel computation should be 
carried out. Programs written with this model consist of
a number of supersteps, which in turn consist of local 
computations and non-blocking communication. A (barrier)
synchronisation at the end of such a step is used to guarantee
occurance of all communications within a step.

This library (eBSP) provides an implementation of the model on top of the Epiphany SDK (eSDK). 
This allows the BSP computing model to be used with the Epiphany
architecture developed by [Adapteva](http://www.adapteva.com).
In particular this library has been implemented and tested on the 
[Parallella](http://www.parallella.org) board. Our goal is to
allow current BSP programs to be run on the Epiphany architecture
with minimal modifications. We believe the BSP model is a good starting point
for anyone new to parallel algorithms, and our goal for this library is to provide an easy way
to implement parallel programs on the Epiphany architecture, without having to resort directly
to the Epiphany SDK.

# Usage

Implementations of the BSP model expose a small set of public functions which are refered to as *primitives*. Programs written with the eBSP library contain a *host processor* and a or *co-processor* part also called *guest*. This is indicated using `host_` and `e_` prefixes respectively. The primitives and their status in the eBSP library are summarized below:

Name          | Description                                                                | Implemented
------------- | -------------------------------------------------------------------------- | -----------
`bsp_init`    | Initializes the BSP system (host).                                         | Yes
`bsp_begin`   | Loads the BSP program and sets up . (host / guest)                         | Yes
`bsp_end`     | Finalizes the BSP program. (host / guest)                                  | Yes
`bsp_pid`     | Returns the processor id. (guest)                                          | Yes
`bsp_nprocs`  | Returns the number of processors available or in use. (host / guest)       | Yes
`bsp_time`    | Returns the elapsed (wall)time in seconds. (guest)                         | Planned
`bsp_sync`    | Denotes the end of a superstep, performs all staged communication. (guest) | Yes
`bsp_push_reg`| Registers a new variable to the BSP system. (guest)                        | Yes
`bsp_put`     | Sends data to another processor using registered variables. (guest)        | Planned
`bsp_hpput`   | Unbuffered version of bsp_put, performed immediately. (guest)              | Yes
...           | ...                                                                        | ...

Besides these primitives we have introduced the function `ebsp_smpd` which starts the program on the co-processor. The host part of a minimal eBSP program looks like this:

```C
// on host
int main() {
    bsp_init("e_program.srec", argc, argv);
    bsp_begin(bsp_nprocs());
    ebsp_smpd();
    bsp_end();
}
```
This will run the Epiphany binary `e_program` on all available Epiphany cores. On the co-processor we have access to a subset of BSP primitives. For example can look up information on our processor id and the number of cores in use:
```C
// on co-processor (e_program)
int main() {
    bsp_begin();

    int n = bsp_nprocs(); 
    int p = bsp_pid();

    ...
```
Furthermore cores can communicate by first registering a variable and then using `bsp_put` or its high-performance (unbuffered) counterpart `bsp_hpput`, and `bsp_sync`. For example:
```C
    ...

    int* a = (int*)0x2000;
    (*a) = p;

    bsp_push_reg(a, sizeof(int));
    bsp_sync();
    bsp_hpput((p + 1) % n, a, a, 0, sizeof(int));
    bsp_sync();
```
would overwrite the variable `a` on processor `p`, with the value `p - 1`. Detailed documentation will be added to the library over time. The header files already contain clear descriptions of the BSP primitives that were implemented so far, and a number of examples are available as well.

# Installation

Currently this project still an early work in progress, and the build process is not yet final. 
However it should not be to hard to get things running yourself. The library and examples
are built seperately. We assume that you have set up the Epiphany SDK. Inside the main directory of `epiphany-bsp` use:

    $ make

to build the library. To build the examples:

    $ cd examples
    $ make

Currently the examples are to be run from the examples folder
(required in order to resolve the Epiphany binaries). For example:

    $ ./bin/host_hello

Runs the 'Hello World!' example. Other examples that are available include a *dot product* implementation and a program which performs an *LU decomposition* of a matrix.
