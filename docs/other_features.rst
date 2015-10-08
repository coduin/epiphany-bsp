.. Epiphany BSP documentation master file, created by
   sphinx-quickstart on Thu Sep 17 21:08:04 2015.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

.. highlight:: c

Other features
==============

There are two features we did not yet discuss. The first is timers, which are useful for getting information on the running time of your programs. The second is intervening in programs that are currently running on the Epiphany using the host.

Timers
------

We provide two mechanisms for getting running time information. The first is accurate for relatively short time intervals (less than about 5 seconds). It is used in the following manner::

    float t_start = bsp_time();
    // ... perform computation
    float t_end = bsp_time();
    float result = t_end - t_start;

The variable result than holds the time taken for the computation in seconds. If you want access to the number of clockcycles used for the computation we provide a similar function `ebsp_raw_time` which gives the number of clockcycles as an unsigned integer::

    unsigned int t_start = bsp_time();
    // ... perform computation
    unsigned int t_end = bsp_time();
    unsigned int result = t_end - t_start;

Note that the default Epiphany clockfrequency is about 600 MHz, such that 600000000 cycles is equal to one second.

Interupts
---------

The maximum number of cycles that can be counted using the raw timer is `UINT_MAX`. After reaching this maximum value, an interupt will be fired. This interupt is ignored by default, but if you want to handle this interupt set this up *after the initial call to `bsp_begin`*. The only interupt that is explicitely and necessarily handled by the EBSP library is `dma1_ir`. For more information on the using the DMA engine, see the section on memory management.

Callbacks
---------

If you want to use the host processor together with the Epiphany processor, you require some sort of syncing mechanism. In particular you might want to react to data that has been sent to external memory, or use the ARM in a map-reduce kind of setting. For this we provide a callback mechanism using `ebsp_set_sync_callback`. You can provide a function pointer, and this function will get called each time a core calls `ebsp_host_sync`::

    // on the host
    void callback() {
        printf("ebsp_host_sync called on the Epiphany");
        // communicate with cores or react to data
    }

    ..
    bsp_begin(bsp_nprocs());
    ebsp_set_sync_callback(callback);
    ..

Similarly we provide a callback mechanism for `bsp_end`, which can be useful when developing your own library on top of EBSP.

Interface (Timer and callback)
------------------------------

Host
^^^^

.. doxygenfunction:: ebsp_set_sync_callback
   :project: ebsp

.. doxygenfunction:: ebsp_set_end_callback
   :project: ebsp

Epiphany
^^^^^^^^

.. doxygenfunction:: bsp_time
   :project: ebsp

.. doxygenfunction:: ebsp_raw_time
   :project: ebsp

.. doxygenfunction:: ebsp_host_time
   :project: ebsp
