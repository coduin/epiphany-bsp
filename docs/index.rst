.. Epiphany BSP documentation master file, created by
   sphinx-quickstart on Thu Sep 17 21:08:04 2015.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

.. sectionauthor:: Jan-Willem Buurlage <janwillem@buurlagewits.nl>

Welcome to Epiphany BSP's documentation!
========================================

This documentation provides an introduction to each component of the
Epiphany BSP (EBSP) library. Each section introduces a number of new EBSP functions
which we call *primitives* through an example. At the end of each section the
new primitives are summarized and detailed documentation is provided for them. Finally,
we also provide a complete reference of the API and a short introduction to the formal BSP model.

-----------------

About Coduin
------------

Coduin (formerly Buurlage Wits) is a small company based in Utrecht, the Netherlands. Next to our work on software libraries and models for many-core processors in embedded systems, we are also active in the area of data analysis and predictive modelling.

.. image:: img/coduin_logo.png
    :width: 250 px
    :align: center

If you are using EBSP, or have any questions, remarks or ideas then please get in touch at info@buurlagewits.nl! We would very much like to hear from you.

-----------------

.. toctree::
    :maxdepth: 2
    :caption: User Documentation

    introduction
    basic
    variables
    mp
    host_client
    streaming
    other_features

.. toctree::
    :maxdepth: 2
    :caption: Support Library

    output
    memory_management

.. toctree::
    :maxdepth: 1
    :caption: Reference & Background

    bsp
    api_reference

Indices and tables
==================

* :ref:`genindex`
* :ref:`modindex`
* :ref:`search`

