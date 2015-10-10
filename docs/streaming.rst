.. sectionauthor:: Jan-Willem Buurlage <janwillem@buurlagewits.nl>
.. highlight:: c

Data streams
============

Streaming
---------

When dealing with problems that involve a lot of data such as images or large matrices, it is often the case that the data for the problem does not fit on the combined local memory of the Epiphany processor. In order to work with the data we must then use the larger (but much slower) external memory, which slows the programs down tremendously.

For these situations we provide a *streaming* mechanism. When writing your program to use streams, it will work on smaller chunks of the problem at any given time -- such that the data currently being treated is always local to the core. The EBSP library prepares the next chunk to work on while the previous chunk is being processed such that there is minimal downtime because the Epiphany cores are waiting for the slow external memory.

Making and using down streams
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

There are two types of streams, *up* and *down* streams. A *down* stream contains data to be processed by an Epiphany core, while an *up* stream contains results from computations performed by the Epiphany core. Every stream (both up and down) has a *target processor*, *total size* and a *chunk size*. The target processor is simply the processor id of the core that should receive the content of the stream. The total size is the total number of bytes of the entire set of data. This set of data then gets partitioned into chunks consisting of the number of bytes set by the chunk size. This size need not be constant (i.e. it may vary over a single stream), but for our discussion here we will assume that it is constant.

A stream is created before the call to `ebsp_spmd` on the host processor. The host prepares the data to be processed by the Epiphany cores, and the EBSP library then performs the necessary work needed for each core to receives its chunk. A stream is created as follows::

    // on the host
    int count = 256;
    int count_in_chunk = 32;
    float* data = malloc(count * sizeof(float));
    // ... fill data
    for (int s = 0; s < bsp_nprocs(); s++) {
        ebsp_create_down_stream(&data, s, count * sizeof(float),
                                    count_in_chunk * sizeof(float));
    }

This will create `bsp_nprocs()` identical streams containing user data, one for each core. These streams are chopped up in `256/32 = 8` chunks. If you want to use these streams in the kernel you need to *open* them and *move chunks* from a stream to the local memory. Every stream you create on the host gets is identified by the order in which they are created. For example, the stream we created above will obtain the id `0` on every core. A second stream (regardless of whether it is up or down) will be identified with `1`, etc. *These identifiers are shared between up and down streams, but not between cores*. Opening a stream is done by using this identifier::

    // in the kernel
    float* address = NULL;
    ebsp_open_down_stream(&(void*)address, // a pointer to the address store
                          0);              // the stream identifier

After this call, address will contain the location in the local memory of the first chunk, but the data is not necessarily there yet (it might still be copying). To ensure that the data has been received we *move* a chunk::

    int double_buffer = 1;
    ebsp_move_chunk_down(&(void*)address, 0, double_buffer);

The first two arguments are identical to those of `ebsp_open_down_stream`. The `double_buffer` argument gives you the option to start writing the next chunk to local memory (using the DMA engine), while you process the current chunk that just moved down. This can be done simultaneously to your computations, but will take up twice as much memory. It depends on the specific situation whether double_buffered mode should be turned on or off. Subsequent blocks are obtained using repeated calls to `ebsp_move_chunk_down`.

If you want to use a chunk multiple times at different stages of your algorithm, you need to be able to instruct EBSP to change which chunk you want to obtain. Internally the EBSP system has a *cursor* for each stream which points to the next chunk that should be obtained. You can modify this cursor using the following two functions::

    // reset the cursor of the first stream to its first chunk
    ebsp_reset_down_cursor(0);

    // move the cursor of the first stream forward by 5 chunks
    ebsp_move_down_cursor(0, 5);

    // move the cursor of the first stream back by 3 chunks
    ebsp_move_down_cursor(0, -3);

Note that this gives you random access inside your streams. Therefore our streaming approach should actually be called *pseudo-streaming*, because formally streaming algorithms only process chunks in a stream a constant number of times. However on the Epiphany we can provide random-access in our streams, leading to different semantics such as moving the cursor.

Moving results back up
^^^^^^^^^^^^^^^^^^^^^^

Up streams work very similar to down streams, however no data has to be supplied by the host since it is generated by the Epiphany. We construct an up stream in the following way::

    // on the host
    // .. create down stream (see above)
    void* upstream_data = malloc(sizeof(void*) * bsp_nprocs());
    for (int s = 0; s < bsp_nprocs(); s++) {
        upstream_data[s] = ebsp_create_down_stream(
            s, count * sizeof(float), count_in_chunk * sizeof(float));
    }

The array `upstream_data` holds pointers to the generated data by each processor. In the kernel you can *open* these streams similarly to down streams::

    // in the kernel
    float* up_address = NULL;
    ebsp_open_up_stream(&(void*)up_address, // a pointer to the address store
                        1);                 // the stream identifier

Note that this stream has the identifier `1` on each core. The up_address now points to a portion of *local memory* that you can fill with data from the kernel. To move a chunk of results up we use::

    int double_buffer = 1;
    ebsp_move_chunk_up(&(void*)up_address, 1, double_buffer);

If we use a double buffer, then after this call `up_address` will point to a new portion of memory, such that you can continue your operations while the previous chunk is being copied up. Again, this uses more local memory, but does allow you to continue processing the next chunk.

Closing streams
^^^^^^^^^^^^^^^

The EBSP stream system allocates buffers for you on the cores. When you are done with a stream you should tell the EBSP system by calling::

    ebsp_close_down_stream(0);
    ebsp_close_up_stream(0);

which will free the buffers for other use.

Interface
------------------

Host
^^^^

.. doxygenfunction:: ebsp_create_down_stream
   :project: ebsp_host

.. doxygenfunction:: ebsp_create_up_stream
   :project: ebsp_host

Epiphany
^^^^^^^^

.. doxygenfunction:: ebsp_open_down_stream
   :project: ebsp_e

.. doxygenfunction:: ebsp_open_up_stream
   :project: ebsp_e

.. doxygenfunction:: ebsp_close_down_stream
   :project: ebsp_e

.. doxygenfunction:: ebsp_close_up_stream
   :project: ebsp_e

.. doxygenfunction:: ebsp_move_chunk_up
   :project: ebsp_e

.. doxygenfunction:: ebsp_move_chunk_down
   :project: ebsp_e

.. doxygenfunction:: ebsp_move_down_cursor
   :project: ebsp_e

.. doxygenfunction:: ebsp_reset_down_cursor
   :project: ebsp_e

.. doxygenfunction:: ebsp_set_up_chunk_size
   :project: ebsp_e
