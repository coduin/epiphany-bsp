
.. Epiphany BSP documentation master file, created by
   sphinx-quickstart on Thu Sep 17 21:08:04 2015.
   You can adapt this file completely to your liking, but it should at least
   contain the root `toctree` directive.

API Reference
=============

On this page we give a complete overview of all the primitives we expose in the main EBSP library.

Host
----

bsp_init
^^^^^^^^

.. doxygenfunction:: bsp_init
   :project: ebsp

ebsp_spmd
^^^^^^^^^

.. doxygenfunction:: ebsp_spmd
   :project: ebsp

bsp_begin
^^^^^^^^^

.. doxygenfunction:: bsp_begin
   :project: ebsp

bsp_end
^^^^^^^

.. doxygenfunction:: bsp_end
   :project: ebsp

bsp_nprocs
^^^^^^^^^^

.. doxygenfunction:: bsp_nprocs
   :project: ebsp

ebsp_set_tagsize
^^^^^^^^^^^^^^^^

.. doxygenfunction:: ebsp_set_tagsize
   :project: ebsp

ebsp_send_down
^^^^^^^^^^^^^^

.. doxygenfunction:: ebsp_send_down
   :project: ebsp

ebsp_get_tagsize
^^^^^^^^^^^^^^^^

.. doxygenfunction:: ebsp_get_tagsize
   :project: ebsp

ebsp_qsize
^^^^^^^^^^

.. doxygenfunction:: ebsp_qsize
   :project: ebsp

ebsp_get_tag
^^^^^^^^^^^^

.. doxygenfunction:: ebsp_get_tag
   :project: ebsp

ebsp_move
^^^^^^^^^

.. doxygenfunction:: ebsp_move
   :project: ebsp

ebsp_hpmove
^^^^^^^^^^^

.. doxygenfunction:: ebsp_hpmove
   :project: ebsp

ebsp_create_down_stream
^^^^^^^^^^^^^^^^^^^^^^^

.. doxygenfunction:: ebsp_create_down_stream
   :project: ebsp

ebsp_create_down_stream_raw
^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. doxygenfunction:: ebsp_create_down_stream_raw
   :project: ebsp

ebsp_create_up_stream
^^^^^^^^^^^^^^^^^^^^^

.. doxygenfunction:: ebsp_create_up_stream
   :project: ebsp

ebsp_write
^^^^^^^^^^

.. doxygenfunction:: ebsp_write
   :project: ebsp

ebsp_read
^^^^^^^^^

.. doxygenfunction:: ebsp_read
   :project: ebsp

ebsp_set_sync_callback
^^^^^^^^^^^^^^^^^^^^^^

.. doxygenfunction:: ebsp_set_sync_callback
   :project: ebsp

ebsp_set_end_callback
^^^^^^^^^^^^^^^^^^^^^

.. doxygenfunction:: ebsp_set_end_callback
   :project: ebsp

Epiphany
--------

bsp_begin
^^^^^^^^^

.. doxygenfunction:: bsp_begin
   :project: ebsp

bsp_end
^^^^^^^

.. doxygenfunction:: bsp_end
   :project: ebsp

bsp_nprocs
^^^^^^^^^^

.. doxygenfunction:: bsp_nprocs
   :project: ebsp

bsp_pid
^^^^^^^

.. doxygenfunction:: bsp_pid
   :project: ebsp

bsp_time
^^^^^^^^

.. doxygenfunction:: bsp_time
   :project: ebsp

ebsp_host_time
^^^^^^^^^^^^^

.. doxygenfunction:: ebsp_host_time
   :project: ebsp

ebsp_raw_time
^^^^^^^^^^^^^

.. doxygenfunction:: ebsp_raw_time
   :project: ebsp

bsp_sync
^^^^^^^^

.. doxygenfunction:: bsp_sync
   :project: ebsp

ebsp_barrier
^^^^^^^^^^^^

.. doxygenfunction:: ebsp_barrier
   :project: ebsp

bsp_push_reg
^^^^^^^^^^^^

.. doxygenfunction:: bsp_push_reg
   :project: ebsp

bsp_pop_reg
^^^^^^^^^^^^

.. doxygenfunction:: bsp_pop_reg
   :project: ebsp

bsp_put
^^^^^^^

.. doxygenfunction:: bsp_put
   :project: ebsp

bsp_get
^^^^^^^

.. doxygenfunction:: bsp_get
   :project: ebsp

bsp_hpput
^^^^^^^^^

.. doxygenfunction:: bsp_hpput
   :project: ebsp

bsp_hpget
^^^^^^^^^

.. doxygenfunction:: bsp_hpget
   :project: ebsp

bsp_set_tagsize
^^^^^^^^^^^^^^^

.. doxygenfunction:: bsp_set_tagsize
   :project: ebsp

ebsp_get_tagsize
^^^^^^^^^^^^^^^^

.. doxygenfunction:: ebsp_get_tagsize
   :project: ebsp

bsp_send
^^^^^^^^

.. doxygenfunction:: bsp_send
   :project: ebsp

bsp_qsize
^^^^^^^^^

.. doxygenfunction:: bsp_qsize
   :project: ebsp

bsp_get_tag
^^^^^^^^^^^

.. doxygenfunction:: bsp_get_tag
   :project: ebsp

bsp_move
^^^^^^^^

.. doxygenfunction:: bsp_move
   :project: ebsp

bsp_hpmove
^^^^^^^^^^

.. doxygenfunction:: bsp_hpmove
   :project: ebsp

ebsp_send_up
^^^^^^^^^^^^

.. doxygenfunction:: ebsp_send_up
   :project: ebsp

ebsp_move_chunk_down
^^^^^^^^^^^^^^^^^^^^

.. doxygenfunction:: ebsp_move_chunk_down
   :project: ebsp

ebsp_move_chunk_up
^^^^^^^^^^^^^^^^^^

.. doxygenfunction:: ebsp_move_chunk_up
   :project: ebsp

ebsp_move_down_cursor
^^^^^^^^^^^^^^^^^^^^^

.. doxygenfunction:: ebsp_move_down_cursor
   :project: ebsp

ebsp_reset_down_cursor
^^^^^^^^^^^^^^^^^^^^^^

.. doxygenfunction:: ebsp_reset_down_cursor
   :project: ebsp

ebsp_open_up_stream
^^^^^^^^^^^^^^^^^^^

.. doxygenfunction:: ebsp_open_up_stream
   :project: ebsp

ebsp_open_down_stream
^^^^^^^^^^^^^^^^^^^^^

.. doxygenfunction:: ebsp_open_down_stream
   :project: ebsp

ebsp_close_up_stream
^^^^^^^^^^^^^^^^^^^^

.. doxygenfunction:: ebsp_close_up_stream
   :project: ebsp

ebsp_close_down_stream
^^^^^^^^^^^^^^^^^^^^^^

.. doxygenfunction:: ebsp_close_down_stream
   :project: ebsp

ebsp_set_up_chunk_size
^^^^^^^^^^^^^^^^^^^^^^

.. doxygenfunction:: ebsp_set_up_chunk_size
   :project: ebsp

bsp_abort
^^^^^^^^^

.. doxygenfunction:: bsp_abort
   :project: ebsp
