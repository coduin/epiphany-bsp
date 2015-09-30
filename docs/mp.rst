.. Epiphany BSP documentation master file, created by
   sphinx-quickstart on Thu Sep 17 21:08:04 2015.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

.. highlight:: c

Message Passing
================

Introduction
------------

The next subject we will discuss is *passing messages* between Epiphany cores. Message passing provides a way to communicate between cores, without having to register variables. This relies on a *message queue*, which is available to every processor. Using message passing, you can communicate to other cores without registering variables. This can be very useful when the amount of data varies from core to core, and it is not clear beforehand how the data will be distributed. It is good to keep in mind that message passing is a lot slower than alternative communication methods since it utilizes the *external memory*.

A BSP message has a *tag* and a *payload*. The *tag* identifies the message, and the *payload* contains the acutal data. The size (in bytes) of a tag is universal, i.e. it is the same across all Epiphany cores (as well as the host). The tagsize can be set using `bsp_set_tagsize`:::

    int tagsize = sizeof(int);
    bsp_set_tagsize(&tagsize);
    bsp_sync();

The tagsize must be set on each core simultaneously, that is to say in the same *superstep*. Alternatively, The tagsize can also be set on the host before issuing `ebsp_spmd`. For compatibility reasons, the call to `bsp_set_tagsize` writes the old value for the tagsize to its argument. We also provide an alternative way to obtain the tagsize, by simply calling `ebsp_get_tagsize`.::

    int tagsize = ebsp_get_tagsize();

After setting the tagsize (and synchronizing), we are ready to start sending messages. We can send a message using `bsp_send`:::

    int tag = 1;
    int payload = 42 + s;
    bsp_send((s + 1) % p, &tag, &payload, sizeof(int));
    bsp_sync();

We first need to declare variables holding the tag and the payload. In our case these are integers, but in general you can use any data type. In order, the arguments of `bsp_send` are:

1. The `pid` of processor we want to *send* the message to.
2. A pointer to the tag data.
3. A pointer to the payload data.
4. The size of the payload. Note that you can vary this size between every send call, contrary to the tagsize.

After synchronizing, the target processor can receive the message. To receive messages, we must first inspect the queue:::

    int packets = 0;
    int accum_bytes = 0;
    bsp_qsize(&packets, &accum_bytes);

The call to `bsp_qsize` writes the *number of packets* to the first argument, and the *total number of bytes in the queue* to the second argument. Next, we can loop over each packet, *moving* the packages to the local core:::

    int payload_in = 0;
    int payload_size = 0;
    int tag_in = 0;
    for (int i = 0; i < packets; ++i) {
        bsp_get_tag(&payload_size, &tag_in);
        bsp_move(&payload_in, sizeof(int));
        ebsp_message("payload: %i, tag: %i", payload_in, tag_in);
    }

We use two new primitives here. First we obtain for each packet (note that here we only have a single packet) the payload size and the incoming tag, using `bsp_get_tagsize`. The payload itself is *moved* using `bsp_move`. The first argument should point to a buffer large enough to store the payload data, and the second argument is the number of bytes to move. Note that we could use our obtained payload size to allocate a buffer large enough to hold the payload, and we could pass it to the second argument of `bsp_move`. It is good to keep in mind that if less bytes are moved than the size of the payload, the remaining data is thrown away. Here we know all messages contain a single integer, such that we can just write the payload into a local variable directly.

This code results in the following output:::

    $02: payload: 43, tag: 1
    $08: payload: 49, tag: 1
    $00: payload: 57, tag: 1
    $13: payload: 54, tag: 1
    ...

Message passing is a very general and powerful technique when using variables to communicate proves to restrictive. However, the flexibility of message passing comes with performance penalty, because the buffers that are involved are too large to store on a single core. As before, `bsp_hpput` and `bsp_hpget` should be your preferred way of communicating if you are optimizing for speed.


Example
-------

We finish our discussion of inter-core BSP message passing by providing a complete program that sends messages around:::

    int s = bsp_pid();
    int p = bsp_nprocs();

    int tagsize = sizeof(int);
    bsp_set_tagsize(&tagsize);
    bsp_sync();

    int tag = 1;
    int payload = 42 + s;
    bsp_send((s + 1) % p, &tag, &payload, sizeof(int));
    bsp_sync();

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

Interface
---------

Host
^^^^

.. doxygenfunction:: ebsp_qsize
   :project: ebsp

.. doxygenfunction:: bsp_set_tagsize
   :project: ebsp

Epiphany
^^^^^^^^

.. doxygenfunction:: bsp_set_tagsize
   :project: ebsp

.. doxygenfunction:: ebsp_set_tagsize
   :project: ebsp

.. doxygenfunction:: bsp_move
   :project: ebsp
