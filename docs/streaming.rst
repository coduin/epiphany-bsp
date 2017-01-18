.. sectionauthor:: Jan-Willem Buurlage <janwillem@buurlagewits.nl>
.. highlight:: c

Data streams
============

Streaming
---------

When dealing with problems that involve a lot of data such as images or large matrices, it is often the case that the data for the problem does not fit on the combined local memory of the Epiphany processor. In order to work with the data we must then use the larger (but much slower) external memory, which slows the programs down tremendously.

For these situations we provide a *streaming* mechanism. When writing your program to use streams, it will work on smaller tokens of the problem at any given time -- such that the data currently being treated is always local to the core. The EBSP library prepares the next token to work on while the previous token is being processed such that there is minimal downtime because the Epiphany cores are waiting for the slow external memory.

Making and using down streams
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

A stream contains data to be processed by an Epiphany core, and can also be used to obtain results from computations performed by the Epiphany core. Every stream has a *total size* and a *token size*. The total size is the total number of bytes of the entire set of data. This set of data then gets partitioned into tokens consisting of the number of bytes set by the token size. This size need not be constant (i.e. it may vary over a single stream), but for our discussion here we will assume that it is constant.

A stream is created before the call to ``ebsp_spmd`` on the host processor. The host prepares the data to be processed by the Epiphany cores, and the EBSP library then performs the necessary work needed for each core to receives its token. Note that this data is copied efficiently to the external memory upon creation of the stream, so that the user data should be stored in the ordinary RAM, e.g. allocated by a call to ``malloc``. A stream is created as follows::

    // (on the host)
    int count = 256;
    int count_in_token = 32;
    float* data = malloc(count * sizeof(float));
    // ... fill data
    bsp_stream_create(count * sizeof(float), count_in_token * sizeof(float), data);

This will create a stream containing user data. This stream is chopped up in ``256/32 = 8`` tokens. If you want to use this streams in the kernel of a core you need to *open* it and *move tokens* from a stream to the local memory. Every stream you create on the host gets is identified by the order in which they are created, starting from index ``0``. For example, the stream we created above will obtain the id ``0``. A second stream (regardless of whether it is up or down) will be identified with ``1``, etc. *These identifiers are shared between cores*. Opening a stream is done by using this identifier, for example, to open a stream with identifier ``3``::

    bsp_stream mystream;
    if(bsp_stream_open(&mystream, 3)) {
        // ...
    }

After this call, the stream will start copying data to the core, but the data is not necessarily there yet (it might still be copying). To access this data we *move* a token::

    // Get some data
    void* buffer = NULL;
    bsp_stream_move_down(&mystream, &buffer, 0);
    // The data is now in buffer

The first argument is the stream object that was filled using ``bsp_stream_open``. The second argument is a pointer to a pointer that will be set to the data location. The final ``double_buffer`` argument, gives you the option to start writing the next token to local memory (using the DMA engine), while you process the current token that you just moved down. This can be done simultaneously to your computations, but will take up twice as much memory. It depends on the specific situation whether double buffered mode should be turned on or off. Subsequent blocks are obtained using repeated calls to ``bsp_stream_move_down``.

If you want to use a token multiple times at different stages of your algorithm, you need to be able to instruct EBSP to change which token you want to obtain. Internally the EBSP system has a *cursor* for each stream which points to the next token that should be obtained. You can modify this cursor using the following two functions::

    // move the cursor of the stream forward by 5 tokens
    bsp_stream_seek(&mystream, 5);

    // move the cursor of the stream back by 3 tokens
    bsp_stream_seek(&mystream, -3);

When you exceed the bounds of the stream, it will be set to the final or first token respectively. Note that this gives you random access inside your streams. Therefore our streaming approach should actually be called *pseudo-streaming*, because formally streaming algorithms only process tokens in a stream a constant number of times. However on the Epiphany we can provide random-access in our streams, opening the door to different semantics such as moving the cursor.

Moving results back up
^^^^^^^^^^^^^^^^^^^^^^

A stream can also be used to move results back up, for example::

    int* buffer1 = ebsp_malloc(100 * sizeof(int));
    int* buffer2 = ebsp_malloc(100 * sizeof(int));
    int* curbuffer = buffer1;
    int* otherbuffer = buffer2;

    ebsp_stream s;
    bsp_stream_open(&s, 0); // open stream 0
    while (...) {
        // Fill curbuffer
        for (int i = 0; i < 100; i++)
            curbuffer[i] = 5;

        // Send up
        bsp_stream_move_up(&s, curbuffer, 100 * sizeof(int), 0);
        // Use other bufferfer
        swap(curbuffer, otherbuffer);
    }
    ebsp_free(buffer1);
    ebsp_free(buffer2);

Here, we have two buffers containing data. While filling one of the buffers with data, we move the other buffer up. We do this using the ``bsp_stream_move_up`` function which has as arguments respectively: the stream handle, the data to send up, the size of the data to send up, and a flag that indicates whether we want to *wait for completion*. In this case, we do not wait, but use two buffers to perform computations and to send data up to the host simulatenously.

Closing streams
^^^^^^^^^^^^^^^

The EBSP stream system allocates buffers for you on the cores. When you are done with a stream you should tell the EBSP system by calling::

    bsp_stream_close(&my_stream);

which will free the buffers for other use, and allow other cores to use the streams.

Interface
------------------

Host
^^^^

.. doxygenfunction:: bsp_stream_create
   :project: ebsp_host

Epiphany
^^^^^^^^

.. doxygenfunction:: bsp_stream_open
   :project: ebsp_e

.. doxygenfunction:: bsp_stream_close
   :project: ebsp_e

.. doxygenfunction:: bsp_stream_move_up
   :project: ebsp_e

.. doxygenfunction:: bsp_stream_move_down
   :project: ebsp_e

.. doxygenfunction:: bsp_stream_seek
   :project: ebsp_e
