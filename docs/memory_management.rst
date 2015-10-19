.. sectionauthor:: Tom Bannink <tom@buurlagewits.nl>

.. highlight:: c

Memory Management
=================

Memory management on the Epiphany platform requires some special care so we discuss it in this separate section. The Epiphany cores have very little local (fast) memory, and access the external (larger) memory space is very slow. Therefore one needs to pay special attention to memory management in order to write good programs for the Epiphany platform.

We provide some functions that aid in memory allocation. These are not part of the offical BSP standard, but meant as a utility library. This page will cover these helper functions. If you are interested in the more technical details (specific for the Parallella), see :ref:`Parallella memory details<memory_details>`.

Introduction
------------

In short, there are two types of memory:

- local memory: 32 KB for each core, fast
- external memory: 32 MB shared for all cores, slow

In principle, all computations should be performed on data in the fast local memory. However 32 KB might not be enough for all your data. In this case you have to store the data in external memory and transfer the required parts to local memory to do the computations. Access to external memory can be a factor 100 slower in some cases so this should be avoided when possible.

How do you know in what type of memory your data is stored? Let us look at the following example::

    int global_var;

    int my_function(int argument) {
        int local_var;
        void* buffer1 = ebsp_malloc(10 * sizeof(int));
        void* buffer2 = ebsp_ext_malloc(10 * sizeof(int));


        // ....

        if (buffer1) ebsp_free(buffer1);
        if (buffer2) ebsp_free(buffer2);
    }

In this example, the following variables will be stored in local memory:
- ``global_var``
- ``local_var``
- ``argument``
- ``buffer1``, as well as the data it points to
- ``my_function`` (the machine code)
However, ``buffer2`` points to data that is stored in the large *external memory*. The pointer itself is stored in local memory.

In general, global and local variables in your C source code will be stored in local memory, unless otherwise specified with some special gcc attributes. Code itself (i.e. the machine code) can also be stored in both types of memory. Normal C code will be stored in local memory, unless specified using gcc attributes. Variables allocated using :cpp:func:`ebsp_ext_malloc` are stored in external memory.

Data copying
------------

Direct memcpy
.............

In C you can copy data using ``memcpy(destination, source, nbytes)``. This function is available on the Epiphany as well as part of the ESDK, but its implementation is not properly optimized for the Epiphany architecture. In particular the function itself is stored in external memory (unless you choose to save the complete C library in local memory) and it also does not perform 8-byte transfers. For this reason we have created :cpp:func:`ebsp_memcpy` which is stored in local memory and does transfers utilizing 8-byte read/write instructions when possible. It is therefore faster than ``memcpy`` and should be preferred.

DMA engine
..........

Each Epiphany processor contains a so-called DMA engine which can be used to transfer data. This DMA engine can be viewed as a separate core that can copy data while the normal Epiphany core does other things. The Epiphany core can simply give the DMA engine a task (a source and destination address along with some other options) and the DMA engine will copy the data so that the Epiphany core can continue with other operations. The advantage of the DMA engine over normal memory access is that the DMA engine is **faster** and can transfer data **while the CPU does other things**. There are **two DMA channels**, meaning that two pairs of source/destination addresses can be set and the Epiphany core can continue while the DMA engine is transfering data. 

We have provided some utility functions to make the use of the DMA engine easier.  If you want to use the DMA engine using the ``e_dma_xxx`` functions from the ESDK you can do so, but only use ``E_DMA_0``. The other DMA channel (``E_DMA_1``) is used internally by the library.

.. warning::
    The DMA engine can not transfer data from the local core to itself (i.e. to another memory location in the same core). Either the source or destination (or both) should point to another core's memory or to external memory.

The Epiphany BSP library provides the functions :cpp:func:`ebsp_dma_push` and :cpp:func:`ebsp_dma_wait`. They implement a queue of DMA tasks that are handled sequentially. With :cpp:func:`ebsp_dma_push` you can push a task to this queue and with :cpp:func:`ebsp_dma_wait` you can wait for the task to complete::

    // A handle identifies the transfer task
    ebsp_dma_handle descriptor_1;
    ebsp_dma_handle descriptor_2;

    // Start two transfers
    ebsp_dma_push(&descriptor_1, destination_1, source_1, data_size_1);
    ebsp_dma_push(&descriptor_2, destination_2, source_2, data_size_2);

    // perform some computations
    // ...

    // Wait for them to finish
    ebsp_dma_wait(&descriptor_1);
    ebsp_dma_wait(&descriptor_2);

Pushing a new task will start the DMA engine if it was not started yet. If it was already running, the library will add the task to an internal queue and automatically point the DMA engine to the next task when it is finished. For those who are interested, this is implemented using interrupts.

In order to use the DMA engine to write data to another core, one needs a memory address that points to the local memory of another core. For this we provide the function :cpp:func:`ebsp_get_direct_address`::

    // Some buffer
    float data[16];

    // Register it in the BSP system
    bsp_push_reg(&data, sizeof(data));
    bsp_sync();

    // Get an address for the data buffer on the core with pid 3
    float* remote_data = ebsp_get_direct_address(3, &incoming_data);

    // Now we can pass 'remote_data' to the DMA engine, or use it directly
    *remote_data = 1.0f;

The above example shows how to obtain an address of a variable on another core. This address can then be passed as source or destination to :cpp:func:`ebsp_dma_push`.

Example
-------

Memory allocation
.................

The memory allocation functions work analogously to the normal C function ``malloc``. Memory allocated by :cpp:func:`ebsp_ext_malloc` and by :cpp:func:`ebsp_malloc` can both be freed with the same function :cpp:func:`ebsp_free`, as in the following example::

    // Allocate local memory
    float* local_data = (float*)ebsp_malloc(16 * sizeof(float));
    if (!local_data) {
        ebsp_message("Memory allocation failed!");
    } else {
        for (int i = 0; i < 16; i++)
            local_data[i] = 2.0f;
        // Free the memory
        ebsp_free(local_data);
    }

    // Allocate external memory
    float* external_data = (float*)ebsp_ext_malloc(16 * sizeof(float));
    if (external_data) {
        do_computation();
        // Free the memory
        ebsp_free(external_data);
    }

Note that calling :cpp:func:`ebsp_free` with a null pointer results in undefined behaviour, so the following is **NOT** allowed::

    float* local_data = (float*)ebsp_malloc(16 * sizeof(float));
    if (local_data) {
        for (int i = 0; i < 16; i++)
            local_data[i] = 2.0f;
    }
    // !!! WRONG: This will crash if local_data is NULL
    ebsp_free(local_data);


External memory DMA transfers
.............................

The following example demonstrates the use of :cpp:func:`ebsp_dma_push` to write a buffer of local data to external memory and read a buffer from external memory using the DMA engine.::

    // Allocate buffers
    float* external_data_1 = (float*)ebsp_ext_malloc(16 * sizeof(float));
    float* external_data_2 = (float*)ebsp_ext_malloc(16 * sizeof(float));
    float* local_data_1 = (float*)ebsp_malloc(16 * sizeof(float));
    float* local_data_2 = (float*)ebsp_malloc(16 * sizeof(float));

    // Fill local buffer 1 with data
    for (int i = 0; i < 16; i++)
        local_data_1[i] = 2.0f;

    // Fill external buffer 2 with data
    // Note that this is slow
    for (int i = 0; i < 16; i++)
        external_data_2[i] = 2.0f;
    
    // To 'tasks' for the DMA engine:
    // Copy local_data_1 to external_data_1 (write to external memory)
    // Copy external_data_2 to local_data_2 (read from external memory)

    // This corresponds to two handles
    ebsp_dma_handle descriptor_1;
    ebsp_dma_handle descriptor_2;

    // Start the DMA with the writing task
    ebsp_dma_push(&descriptor_1, external_data_1, local_data_1, sizeof(local_data_1));

    // We can 'push' the next task while the DMA already works on the first task
    ebsp_dma_push(&descriptor_2, local_data_2, external_data_2, sizeof(local_data_2));
    
    // Do lengthy computation in the mean time
    do_computations();
    
    // Wait for the DMA to finish the second task
    ebsp_dma_wait(&descriptor_2);

    // Because the DMA performs the tasks in order,
    // we can be assured that the first task is completed as well

Core to core DMA transfers
..........................

To use the DMA to transfer data to another core, we need to get the address that points to another core. This can be done using :cpp:func:`ebsp_get_direct_address`. In the following example we have to arrays: ``my_data`` and ``incoming_data``. The idea is to copy the contents of ``my_data`` on the local core into ``incoming_data`` on the next core. To do this, we first register ``incoming_data``. After this we can get the address of the corresponding array on a remote core. In this case we take the core with pid ``s + 1`` where ``s`` is the pid of the local core. With this address we can now use :cpp:func:`ebsp_dma_push` to copy data using the DMA engine. During this transfer, other computations can be done. After this we use :cpp:func:`ebsp_dma_wait` which blocks untill the transfer is complete (or returns immediately if already completed).::

    int s = bsp_pid();
    int p = bsp_nprocs();
    
    // Data to be sent
    float my_data[16];

    // Buffer to receive data
    float incoming_data[16];

    // Register it in the BSP system
    bsp_push_reg(&incoming_data, sizeof(incoming_data));
    bsp_sync();
    
    // Get an address for the incoming_data buffer on the core with pid s + 1.
    float* remote_data = ebsp_get_direct_address((s+1)%p, &incoming_data);
    
    // Start the DMA to copy the data from my_data on this core to incoming_data on the next core
    ebsp_dma_handle descriptor;
    ebsp_dma_push(&descriptor, remote_data, &my_data, sizeof(my_data));
    
    // Do lengthy computation
    do_computations();
    
    // Wait for the DMA transfer to finish
    ebsp_dma_wait(&descriptor);
    
    // Done

Interface
---------

Epiphany
^^^^^^^^

.. doxygenfunction:: ebsp_ext_malloc
   :project: ebsp_e

.. doxygenfunction:: ebsp_malloc
   :project: ebsp_e

.. doxygenfunction:: ebsp_free
   :project: ebsp_e

.. doxygenfunction:: ebsp_memcpy
   :project: ebsp_e

.. doxygenfunction:: ebsp_dma_push
   :project: ebsp_e

.. doxygenfunction:: ebsp_dma_wait
   :project: ebsp_e

.. doxygenfunction:: ebsp_get_direct_address
   :project: ebsp_e
