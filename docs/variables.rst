.. Epiphany BSP documentation master file, created by
   sphinx-quickstart on Thu Sep 17 21:08:04 2015.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

.. highlight:: c

BSP Variables
=============

Introduction
------------

If we want to write more interesting EBSP programs, we need to have a way to communicate between the different Epiphany cores. In EBSP communication happens in one of two ways: using message passing, which we will introduce later, or via *registered variables*. An EBSP variable exists on every processor, but it does not have to have the same size on every Epiphany core.

Variable registration
^^^^^^^^^^^^^^^^^^^^^

We register a variable by calling `bsp_push_reg`:::

    int a = 0;
    bsp_push_reg(&a, sizeof(int));
    bsp_sync();

Here we declare an integer `a`, and initialize it with zero. Next we *register* the variable with BSP system, by passing its local location, and its size.

To ensure that all cores have registered a variable, we perform a barrier synchronisation after the registration. The Epiphany cores will halt execution until *every other core* reaches this point in the program, so it *synchronizes* the program execution between the Epiphany cores. Only *one variable may be declared between calls to `bsp_sync`*!

Putting and getting values
^^^^^^^^^^^^^^^^^^^^^^^^^^

Registered variables can be written to or read from by other cores. In BSP this is refered to as *putting* something in a variable, or *getting* the value of a variable. To write for example our processor ID to the *next core* we can write:::

    int b = s;
    bsp_put((s + 1) % p, &b, &a, 0, sizeof(int));
    bsp_sync();

Let us explain this code line by line. As in the *Hello World* example, here we define `s` and `p` to be the processor id and the number of processors respectively. In our call to `bsp_put` we pass the following arguments (in order):

1. The `pid` of the target (i.e. the receiving) processor.
2. A pointer to the source data that we want to copy to the target processor.
3. A pointer representing a *registered variable*. Note that this pointer refers to the registered variable on the *sending* processor -- the EBSP system can identify these processors such that it knows which *remote address* to write to.
4. The offset (in bytes) from which we want to start writing at the target processor.
5. The number of bytes to copy.

Before we want to use a communicated value on the target processor, we need to again perform a barrier synchronisation by calling `bsp_sync`. This ensures that all the outstanding communication gets resolved. After the call to `bsp_sync` returns, we can use the result of `bsp_put` on the target processor. The code between two consecutive calls to `bsp_sync` is called a *superstep*.

When we receive the data, we can for example write the result to the standard output. Below we give the complete program which makes use of `bsp_put` to communicate with another processor. Here, and in the remainder of this post we will only write the code in between the calls to `bsp_begin` and `bsp_end`, the other code is identical to the code in the *Hello World* example.::

    int s = bsp_pid();
    int p = bsp_nprocs();

    int a = 0;
    bsp_push_reg(&a, sizeof(int));
    bsp_sync();

    int b = s;
    bsp_put((s + 1) % p, &b, &a, 0, sizeof(int));
    bsp_sync();

    ebsp_message("received: %i", a);

This results in the following output:::

    $01: received: 0
    $02: received: 1
    $07: received: 6
    $00: received: 15
    ...

Where we have suppressed the output from the other cores. As we see we are correctly receiving the processor id of the previous cores!

An alternative way of communication is *getting* the value of a registered variable from a remote core. The syntax is very similar:::

    a = s;
    int b = 0;
    bsp_get((s + 1) % p, &a, 0, &b, sizeof(int));
    bsp_sync();

The arguments for `bsp_get` are:

1. The `pid` of processor we want to *get* the value from.
2. The pointer representing a registed variable.
3. The offset (in bytes) at the remote processor from which we want to start reading.
4. A pointer to the local destination.
5. The number of bytes to copy.

And again, we perform a barrier synchronisation to ensure the data has been transferred. If you are familiar with concurrent programming, then you might think we are at risk of a `race condition <https://en.wikipedia.org/wiki/Race_condition>`_! What if processor `s` reaches the `bsp_get` statement before processor `(s + 1) % p` has set the value for `a` equal to its process number? Do we then obtain zero? In this case, we do not have to worry -- no data transfer is initialized until each core has reached `bsp_sync`. Indeed we receive the correct output:::

    $01: received: 2
    $03: received: 4
    $11: received: 12
    $14: received: 15
    ...

Unbuffered communication
^^^^^^^^^^^^^^^^^^^^^^^^

So far we have discussed writing to, and reading from variables using `bsp_put` and `bsp_get`. These two functions are *buffered*. When calling `bsp_put` for example, the *current source value* at the time of the function call is guarenteed to be sent to the target processor, but it does not get sent until the next barrier synchronisation -- so behind the scenes the EBSP library stores a copy of the data. The BSP standard was originally designed for distributed memory systems with very high latency, in which this design makes a lot of sense. On the Epiphany platform this gives a lot of unnecessary overhead since data is copied to *external memory*.

This problem is not unique to the Epiphany platform however. Together with the `MulticoreBSP <http://www.multicorebsp.com/>`_ which target modern multicore processors, two additional BSP primitives were introduced that provide *unbuffered* variable communication, `bsp_hpput` and `bsp_hpget`. Here the `hp...` prefix stands for *high performance*.

However, although their function signatures are completely identical, these are not meant as a drop-in replacements for `bsp_put` and `bsp_get`. They are unsafe in the sense that data transfer happens *at once*. This means that when using these functions you should be aware of possible race conditions -- which can notoriously lead to mistakes that can be very hard to debug.

To facilitate writing code using only unbuffered communication we will expose an `ebsp_barrier` function in the next EBSP release that performs a barrier synchronisation without transferring any outstanding communication that has arisen from calls to `bsp_put` and `bsp_get`. Let us look at an example program using these unbuffered variants.::

    int s = bsp_pid();
    int p = bsp_nprocs();

    int a = 0;
    bsp_push_reg(&a, sizeof(int));
    bsp_sync();

    int b = s;
    // barrier ensures b has been written to on each core
    bsp_sync();

    bsp_hpput((s + 1) % p, &b, &a, 0, sizeof(int));

    // barrier ensures data has been received
    bsp_sync();
    ebsp_message("received: %i", a);

When writing or reading large amounts of data in between different `bsp_sync` calls, the `hp...` functions are much more efficient in terms of local memory usage (which is very valuable because of the small size) as well as running speed. However, extra care is needed to effectively synchronize between threads. For example, if we remove any of the two `bsp_sync` calls in the previous example program, there will be a race condition.

We test the program, and see that the output is indeed identical to before:::

    $01: received: 0
    $08: received: 7
    $02: received: 1
    $10: received: 9
    ...


Interface
---------

Epiphany
^^^^^^^^

.. doxygenfunction:: bsp_push_reg
   :project: ebsp

.. doxygenfunction:: bsp_put
   :project: ebsp

.. doxygenfunction:: bsp_get
   :project: ebsp

.. doxygenfunction:: bsp_sync
   :project: ebsp

.. doxygenfunction:: bsp_hpput
   :project: ebsp

.. doxygenfunction:: bsp_hpget
   :project: ebsp

.. doxygenfunction:: ebsp_barrier
   :project: ebsp
