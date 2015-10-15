.. sectionauthor:: Tom Bannink <tom@buurlagewits.nl>

.. Epiphany BSP documentation master file, created by
   sphinx-quickstart on Thu Sep 17 21:08:04 2015.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

.. highlight:: c

Memory Management
=================

The cores of the Epiphany chip have very little local memory, and access to so-called external memory is very slow. Therefore one needs to pay special attention to memory management in order to write good programs for the Epiphany platform.

In short, there is *local memory* which is fast but only 32 KB in size, and there is *external memory* which is slow (about a factor 100 in some cases) but large.

TODO: 'Note that this is not part of official BSP but meant as utility library'

TODO: This page covers **using** the ebsp memory management functions. See this link TODO for information on the internals of memory access on the Epiphany platform.

Introduction
------------

...

DMA
---

...

Example
-------

Memory allocation
...

DMA transfers
...
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
