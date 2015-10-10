.. sectionauthor:: Tom Bannink <tom@buurlagewits.nl>

.. Epiphany BSP documentation master file, created by
   sphinx-quickstart on Thu Sep 17 21:08:04 2015.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

.. highlight:: c

Memory Management
=================

To deal with external memory

Introduction
------------

...

Example
-------

...

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

.. doxygenfunction:: ebsp_dma_start
   :project: ebsp_e

.. doxygenfunction:: ebsp_dma_wait
   :project: ebsp_e

.. doxygenfunction:: ebsp_get_raw_address
   :project: ebsp_e
