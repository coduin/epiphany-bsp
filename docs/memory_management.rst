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

How do you know in what type of memory your data is stored?
Global and local variables in your C source code will be stored in local memory, unless otherwise specified with some special gcc attributes. 
Code itself (i.e. the machine code) can also be stored in both types of memory. Normal C code will be stored in local memory, unless specified using gcc attributes.
Variables allocated using ``ebsp_ext_malloc`` are stored in external memory.

TODO: Introduce DMA


DMA
---

TODO: specify that users should not use DMA1 explicitly, only dma0 if wanted.

...

Example
-------

Memory allocation
......

TODO: IMPORTANT: ext_malloc exists on host: can we alloc and send_message the address? nope

DMA transfers
......

note that this requires ebsp_get_direct_address()


Interface
------------------

.. doxygenfunction:: ebsp_ext_malloc
   :project: ebsp_e

.. doxygenfunction:: ebsp_malloc
   :project: ebsp_e

.. doxygenfunction:: ebsp_free
   :project: ebsp_e

.. doxygenfunction:: ebsp_dma_push
   :project: ebsp_e

.. doxygenfunction:: ebsp_dma_wait
   :project: ebsp_e

.. doxygenfunction:: ebsp_get_direct_address
   :project: ebsp_e
