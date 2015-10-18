.. sectionauthor:: Tom Bannink <tom@buurlagewits.nl>

.. highlight:: c

.. _memory_details:

Parallella Memory Details
=========================

This page is meant for people who want to understand the different types of memory that are available on the Parallella. It does not contain required knowledge for those who only want to use the Epiphany BSP library.

This page tries to take away any confusion about the different types of memory available for the Epiphany cores and explain the terminology that is being used in the community.

An Epiphany core is sometimes referred to as a **mesh node** since the network of cores is called a **mesh network**.

Memory Types
------------

The Epiphany cores have access to two types of memory.
Both types can be accessed directly (i.e. dereferencing a pointer).

All addresses shown below are the ones used by the Epiphany cores. They can **not** be used directly by the ARM processor.

Internal memory
...............

**Size**: 32KB (``0x8000``) per core

Location in address space:

- ``0x00000000 - 0x00007fff`` when a core is referring to its own memory
- ``0x???00000 - 0x???07fff`` when referring to the memory of any other core (or itself). The ``???`` indicate the core with 6 bits for the row and 6 bits for the column, see the Epiphany Architecture Reference for how exactly.

Terminology:

- Internal memory
- eCore memory
- SRAM or Static RAM. Not to be confused with Shared RAM.

Usage:

- Program code, starting at lower addresses 
- Program data (global variables), starting at lower addresses after code
- Stack (local variables), starting at ``0x8000`` expanding downwards

External memory
...............

**Size**: 32 MB (``0x02000000``) shared over all cores

Location in address space:

- ``0x8e000000 - 0x8fffffff``

Terminology:

- External memory
- Shared memory
- DRAM or Dynamic RAM
- SDRAM or Shared DRAM

Usage:

.. note::
    Some of the following information depends on the linker script that is being used. The information below is valid when using the ``fast.ldf`` linkerscript.

-   **Location**: ``0x8e000000 - 0x8effffff``

    **Size**:``0x01000000`` (16 MB)

    **Contents**: newlib (the C library, with code, data, stack)

-   **Location**: ``0x8f000000 - 0x8fffffff``

    **Size**:``0x01000000`` (16 MB)

    **Contents**:

    -   **Location**:  ``0x8f000000 - 0x8f7fffff``

        **Size**: ``0x00800000`` (8 MB)

        **Section label**: ``shared_dram`` (see below for section info)

        **Contents**: used by the ``e_shm_xxx`` functions of the ESDK

        **Extra info**: The C function ``malloc`` returns addresses from this region (possibly a bug?) which causes this region to be corrupted if one uses any C function that uses malloc internally. This region is for example altered when calling any ``printf`` variant with a floating point specifier ``"%f"`` in the string.

    -   **Location**:  ``0x8f800000 - 0x8fffffff``

        **Size**: ``0x00800000`` (8 MB)

        **Section label**: ``heap_dram`` (see below for section info)

        **Contents**: is meant to be divided in 512KB for each core (``16 * 512KB = 8MB``) and then used for ``malloc`` but this does **not** currently work. Instead ``malloc`` returns addresses from ``shared_dram``

Accessing the memory from the Epiphany cores
--------------------------------------------

Normal access
.............

All types of memory can be accessed by for example dereferencing a pointer to an address.
If one does not want to hardcode addresses, **section labels** can be used to put data in certain sections, in the following way:::

    //Internal memory
    char my_char; //normal method
    char *my_other_char = (char*)0x6000; //hardcoding addresses
    
    //External memory using section labels
    int my_integer SECTION("shared_dram"); //section at 0x8f000000
    float my_float SECTION("heap_dram"); //section at 0x8f800000
    //External memory using hardcoded addresses
    int *my_other_integer = (int*)0x8f000000;
    float *my_other_float = (float*)0x8f800000;

If one wants to read or write to another core's memory, the ESDK functions ``e_read`` and ``e_write`` can be used, which will compute the correct address (of the form ``0x???00000 + offset``) and memcpy the data.
Alternativly one can use :cpp:func:`ebsp_get_direct_address` to get a direct pointer to the data on the remote core.

DMA Engine
..........

Each Epiphany processor contains a *DMA engine* which can be used to transfer data.
The advantage of the DMA engine over normal memory access is that the DMA engine is **faster** and can transfer data **while the CPU does other things**. There are **two DMA channels**, meaning that two pairs of source/destination addresses can be set and the CPU can continue while the DMA engine is transfering data. This source and destination addresses can even *both be pointing at other cores' internal memory*.
To use the DMA engine one can use the ``e_dma_xxx`` functions from the ESDK or :cpp:func:`ebsp_dma_push`.

Accessing the memory from the ARM processor
...........................................

One can use ``e_read`` and ``e_write`` ESDK functions in order to write to the internal memory of each core.

To write to external memory, one has to use ``e_alloc`` to "allocate" external memory. This function does not actually **allocate** memory (it is already there), it _only_ gives you a ``e_mem_t`` struct that allows you to access the memory with ``e_read`` and ``e_write`` calls.
The ``offset`` that you pass to ``e_alloc`` will be an offset from ``0x8e000000``, meaning an offset of ``0x01000000`` will give you access to the external memory at ``0x8e000000 + 0x01000000 = 0x8f000000 (shared_dram)`` as seen from the Epiphany. Subsequent offsets can then be added on top of this in ``e_read`` and ``e_write`` calls.

Memory speed
------------

This benchmark data has been taken from
https://parallella.org/forums/viewtopic.php?f=23&t=307&sid=773cf3c3fc58f303645cfe0a684965a7
::

    SRAM = Internal memory
    ERAM = External memory
    
    Host -> SRAM: Write speed =   14.62 MBps
    Host <- SRAM: Read speed  =   17.85 MBps
    Host -> ERAM: Write speed =  100.71 MBps
    Host <- ERAM: Read speed  =  135.42 MBps
    
    Using memcpy:
    Core -> SRAM: Write speed =  504.09 MBps clocks = 9299
    Core <- SRAM: Read speed  =  115.65 MBps clocks = 40531
    Core -> ERAM: Write speed =  142.99 MBps clocks = 32782
    Core <- ERAM: Read speed  =    4.19 MBps clocks = 1119132
    
    Using DMA:
    Core -> SRAM: Write speed = 1949.88 MBps clocks = 2404
    Core <- SRAM: Read speed  =  480.82 MBps clocks = 9749
    Core -> ERAM: Write speed =  493.21 MBps clocks = 9504
    Core <- ERAM: Read speed  =  154.52 MBps clocks = 30336
