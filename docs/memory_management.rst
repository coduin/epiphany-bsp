.. sectionauthor:: Tom Bannink <tom@buurlagewits.nl>

.. highlight:: c

Memory Management
=================

Memory management on the Epiphany platform requires some special care so we discuss it in this separate section.
The Epiphany cores have very little local (fast) memory, and access the external (larger) memory space is very slow. Therefore one needs to pay special attention to memory management in order to write good programs for the Epiphany platform.

We provide some functions that aid in memory allocation. These are not part of the offical BSP standard, but meant as a utility library. This page will cover these helper functions. If you are interested in the more technical details (specific for the Parallella), see :ref:`Parallella memory details<memory_details>`.

Introduction
------------

In short, there are two types of memory:

- local memory: 32 KB for each core, fast
- external memory: 32 MB shared for all cores, slow

In principle, all computations should be performed on data in the fast local memory. However 32 KB might not be enough for all your data. In this case you have to store the data in external memory and transfer the required parts to local memory to do the computations. Access to external memory can be a factor 100 slower in some cases so this should be avoided when possible.

How do you know in what type of memory your data is stored? Let us look at the following example:::

    int global_var;

    int my_function(int argument) {
        int local_var;
        void* buffer1 = ebsp_malloc(10 * sizeof(int));
        void* buffer2 = ebsp_ext_malloc(10 * sizeof(int));

        // In fast local memory:
        // - global_var
        // - local_var
        // - argument
        // - buffer1
        // - my_function (the machine code)
        //
        // In large external memory:
        // - buffer2

        if (buffer1) ebsp_free(buffer1);
        if (buffer2) ebsp_free(buffer2);
    }

Global and local variables in your C source code will be stored in local memory, unless otherwise specified with some special gcc attributes. Code itself (i.e. the machine code) can also be stored in both types of memory. Normal C code will be stored in local memory, unless specified using gcc attributes. Variables allocated using :cpp:func:`ebsp_ext_malloc` are stored in external memory.

Data copying
------------

memcpy
......

TODO 1

DMA engine
..........

TODO 2

TODO: specify that users should not use DMA1 explicitly, only dma0 if wanted.

.. warning::
    Do not use DMA for local to local (only to other cores)

note that this requires ebsp_get_direct_address()

TODO: clarify that its a queue, and multiple tasks can be pushed.

Example
-------

Memory allocation
.................

The memory allocation functions work analogously to the normal C function ``malloc``. Memory allocated by :cpp:func:`ebsp_ext_malloc` and by :cpp:func:`ebsp_malloc` can both be freed with the same function :cpp:func:`ebsp_free`, as in the following example:::

    // Allocate local memory
    float* localdata = (float*)ebsp_malloc(16 * sizeof(float));
    if (!localdata) {
        ebsp_message("Memory allocation failed!");
    } else {
        for (int i = 0; i < 16; i++)
            local_data[i] = 2.0f;
        // Free the memory
        ebsp_free(local_data);
    }

    // Allocate external memory
    float* externaldata = (float*)ebsp_ext_malloc(16 * sizeof(float));
    if (externaldata) {
        do_computation();
        // Free the memory
        ebsp_free(externaldata);
    }

Note that calling :cpp:func:`ebsp_free` with a null pointer results in undefined behaviour, so the following is **NOT** allowed:::

    float* localdata = (float*)ebsp_malloc(16 * sizeof(float));
    if (localdata) {
        for (int i = 0; i < 16; i++)
            local_data[i] = 2.0f;
    }
    // !!! WRONG: This will crash if localdata is NULL
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

To use the DMA to transfer data to another core, we need to get the address that points to another core. This can be done using :cpp:func:`ebsp_get_direct_address`. In the following example we have to arrays: ``mydata`` and ``incomingdata``. The idea is to copy the contents of ``mydata`` on the local core into ``incomingdata`` on the next core. To do this, we first register ``incomingdata``. After this we can get the address of the corresponding array on a remote core. In this case we take the core with pid ``s+1`` where ``s`` is the pid of the local core. With this address we can now use :cpp:func:`ebsp_dma_push` to copy data using the DMA engine. During this transfer, other computations can be done. After this we use :cpp:func:`ebsp_dma_wait` which blocks untill the transfer is complete (or returns immediately if already completed).::

    int s = bsp_pid();
    int p = bsp_nprocs();
    
    // Data to be sent
    float mydata[16];

    // Buffer to receive data
    float incomingdata[16];

    // Register it in the BSP system
    bsp_push_reg(&incomingdata, sizeof(incomingdata));
    bsp_sync();
    
    // Get an address for the incomingdata buffer on the core with pid s + 1.
    float* remotedata = ebsp_get_direct_address((s+1)%p, &incomingdata);
    
    // Start the DMA to copy the data from mydata on this core to incomingdata on the next core
    ebsp_dma_handle descriptor;
    ebsp_dma_push(&descriptor, remotedata, &mydata, sizeof(mydata));
    
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
