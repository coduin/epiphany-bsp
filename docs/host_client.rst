.. Epiphany BSP documentation master file, created by
   sphinx-quickstart on Thu Sep 17 21:08:04 2015.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

.. highlight:: c

Communicating with the Epiphany
===============================

Communication: up and down
--------------------------

Writing kernels for the Epiphany is only useful when you can provide them with data to process. The easiest way to send data from the host program running on the host processor to the Epiphany cores is completely analogous to message passing between cores. So far the code we have written on the host only initializes the BSP system, starts the SPMD program on the Epiphany, and finalizes the system afterwards. Before the call to `ebsp_spmd` we can prepare messages containing e.g. initial data for the Epiphany cores. This works completely identically to inter-core message passing, using `ebsp_set_tagsize` instead of `bsp_set_tagsize`, and `ebsp_send_down` instead of `bsp_send`::

    // file: host_code.c
    int n = bsp_nprocs();

    int tagsize = sizeof(int);
    ebsp_set_tagsize(&tagsize);

    int tag = 1;
    int payload = 0;
    for (int s = 0; s < n; ++s) {
        payload = 1000 + s;
        ebsp_send_down(s, &tag, &payload, sizeof(int));
    }

These messages are available like any other on the Epiphany cores, but only between the call to `bsp_begin` and the first call to `bsp_sync`. So on the Epiphany cores we can read the messages using::

    // file: ecore_code.c

    bsp_begin();

    // here the messages from the host are available
    int packets = 0;
    int accum_bytes = 0;
    bsp_qsize(&packets, &accum_bytes);

    int payload_in = 0;
    int payload_size = 0;
    int tag_in = 0;
    for (int i = 0; i < packets; ++i) {
        bsp_get_tag(&payload_size, &tag_in);
        bsp_move(&payload_in, sizeof(int));
        ebsp_message("payload: %i, tag: %i", payload_in, tag_in);
    }

    // after this call the messages are invalidated
    bsp_sync();
    ... // remainder of the program, see below

Resulting in the output::

    $00: payload: 1000, tag: 1
    $03: payload: 1003, tag: 1
    $02: payload: 1002, tag: 1
    $13: payload: 1013, tag: 1
    ...

A similar method can be used to send data up (from the Epiphany cores to the host). If you have followed along with our discussion so far the second half of the kernel code should be self explanatory::

    // file: ecore_code.c
    ... // obtain initial data, see above

    int payload = payload_in + 1000;
    int tag = s;
    ebsp_send_up(&tag, &payload, sizeof(int));

    bsp_end();

Note that now we are using our processor number as the tag, such that the host can use the tag to differentiate between messages coming from different cores. Usage of `ebsp_send_up` is limited to the final superstep, i.e. between the last call to `bsp_sync` and the call to `bsp_end`. In the host program we can read the resulting messages similarly to how we read them on the Epiphany processor::

    // file: host_code.c

    ...
    ebsp_spmd();

    int packets = 0;
    int accum_bytes = 0;
    ebsp_qsize(&packets, &accum_bytes);

    int payload_in = 0;
    int payload_size = 0;
    int tag_in = 0;
    for (int i = 0; i < packets; ++i) {
        ebsp_get_tag(&payload_size, &tag_in);
        ebsp_move(&payload_in, sizeof(int));
        printf("payload: %i, tag: %i", payload_in, tag_in);
    }

    ebsp_end();

This gives the output:

.. code-block:: none

    payload: 2001, tag: 1
    payload: 2013, tag: 13
    payload: 2003, tag: 3
    payload: 2002, tag: 2
    ...

For the first time we have written data to the cores, applied a transformation to the data using the Epiphany cores, and sent it back up to the host program.

Message passing is a nice way to get initial data to the Epiphany cores, and to get the results of computations back to the host. However, it is very restrictive, and does not give the user a lot of control over the way the data gets sent down. An alternative approach is given by `ebsp_write` and `ebsp_read`. These calls require manually addressing the local memory on each core. Every core has 32kb of local memory, corresponding to addresses `0x0000` to `0x8000`. The default settings of EBSP put the program data at the start of this space (i.e. at `0x0000`), and the stack moves downwards from the end (i.e. at `0x8000`). Using `ebsp_write` from the host program, you can prepare data at specific spaces on the local cores::

    int data[4] = { 1, 2, 3, 4 };
    for (int s = 0; s < n; ++s) {
        ebsp_write(0, &data, (void*)0x5000, 4 * sizeof(int));
    }

This would write 4 integers to each core starting at `0x5000`. Similarly, `ebsp_read` can be used to retrieve data from the cores. We would not recommend this approach for users just beginning with the Parallella and EBSP in particular. A better approach to move large amounts of data from and to the Epiphany processor uses *data streams*, which will be introduced in the next EBSP release. This allows data to be moved in predetermined *chunks*, which are acted upon independently. We will explain this approach in detail in a future blogpost.


Interface (Vertical communication)
----------------------------------

Host
^^^^

.. doxygenfunction:: ebsp_qsize
   :project: ebsp

.. doxygenfunction:: ebsp_set_tagsize
   :project: ebsp

.. doxygenfunction:: ebsp_get_tagsize
   :project: ebsp

.. doxygenfunction:: ebsp_send_down
   :project: ebsp

.. doxygenfunction:: ebsp_write
   :project: ebsp

.. doxygenfunction:: ebsp_read
   :project: ebsp

Epiphany
^^^^^^^^

.. doxygenfunction:: ebsp_send_up
   :project: ebsp
