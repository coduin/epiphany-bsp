.. sectionauthor:: Jan-Willem Buurlage <janwillem@buurlagewits.nl>

API Reference
=============

On this page we give a complete overview of all the primitives we expose in the main EBSP library.

Host
----

bsp_init
^^^^^^^^

.. doxygenfunction:: bsp_init
   :project: ebsp_host

ebsp_spmd
^^^^^^^^^

.. doxygenfunction:: ebsp_spmd
   :project: ebsp_host

bsp_begin
^^^^^^^^^

.. doxygenfunction:: bsp_begin
   :project: ebsp_host

bsp_end
^^^^^^^

.. doxygenfunction:: bsp_end
   :project: ebsp_host

bsp_nprocs
^^^^^^^^^^

.. doxygenfunction:: bsp_nprocs
   :project: ebsp_host

ebsp_set_tagsize
^^^^^^^^^^^^^^^^

.. doxygenfunction:: ebsp_set_tagsize
   :project: ebsp_host

ebsp_send_down
^^^^^^^^^^^^^^

.. doxygenfunction:: ebsp_send_down
   :project: ebsp_host

ebsp_get_tagsize
^^^^^^^^^^^^^^^^

.. doxygenfunction:: ebsp_get_tagsize
   :project: ebsp_host

ebsp_qsize
^^^^^^^^^^

.. doxygenfunction:: ebsp_qsize
   :project: ebsp_host

ebsp_get_tag
^^^^^^^^^^^^

.. doxygenfunction:: ebsp_get_tag
   :project: ebsp_host

ebsp_move
^^^^^^^^^

.. doxygenfunction:: ebsp_move
   :project: ebsp_host

ebsp_hpmove
^^^^^^^^^^^

.. doxygenfunction:: ebsp_hpmove
   :project: ebsp_host

ebsp_create_down_stream
^^^^^^^^^^^^^^^^^^^^^^^

.. doxygenfunction:: ebsp_create_down_stream
   :project: ebsp_host

ebsp_create_up_stream
^^^^^^^^^^^^^^^^^^^^^

.. doxygenfunction:: ebsp_create_up_stream
   :project: ebsp_host

ebsp_write
^^^^^^^^^^

.. doxygenfunction:: ebsp_write
   :project: ebsp_host

ebsp_read
^^^^^^^^^

.. doxygenfunction:: ebsp_read
   :project: ebsp_host

ebsp_set_sync_callback
^^^^^^^^^^^^^^^^^^^^^^

.. doxygenfunction:: ebsp_set_sync_callback
   :project: ebsp_host

ebsp_set_end_callback
^^^^^^^^^^^^^^^^^^^^^

.. doxygenfunction:: ebsp_set_end_callback
   :project: ebsp_host

Epiphany
--------

bsp_begin
^^^^^^^^^

.. doxygenfunction:: bsp_begin
   :project: ebsp_e

bsp_end
^^^^^^^

.. doxygenfunction:: bsp_end
   :project: ebsp_e

bsp_nprocs
^^^^^^^^^^

.. doxygenfunction:: bsp_nprocs
   :project: ebsp_e

bsp_pid
^^^^^^^

.. doxygenfunction:: bsp_pid
   :project: ebsp_e

bsp_time
^^^^^^^^

.. doxygenfunction:: bsp_time
   :project: ebsp_e

ebsp_host_time
^^^^^^^^^^^^^^

.. doxygenfunction:: ebsp_host_time
   :project: ebsp_e

ebsp_raw_time
^^^^^^^^^^^^^

.. doxygenfunction:: ebsp_raw_time
   :project: ebsp_e

bsp_sync
^^^^^^^^

.. doxygenfunction:: bsp_sync
   :project: ebsp_e

ebsp_barrier
^^^^^^^^^^^^

.. doxygenfunction:: ebsp_barrier
   :project: ebsp_e

bsp_push_reg
^^^^^^^^^^^^

.. doxygenfunction:: bsp_push_reg
   :project: ebsp_e

bsp_pop_reg
^^^^^^^^^^^^

.. doxygenfunction:: bsp_pop_reg
   :project: ebsp_e

bsp_put
^^^^^^^

.. doxygenfunction:: bsp_put
   :project: ebsp_e

bsp_get
^^^^^^^

.. doxygenfunction:: bsp_get
   :project: ebsp_e

bsp_hpput
^^^^^^^^^

.. doxygenfunction:: bsp_hpput
   :project: ebsp_e

bsp_hpget
^^^^^^^^^

.. doxygenfunction:: bsp_hpget
   :project: ebsp_e

bsp_set_tagsize
^^^^^^^^^^^^^^^

.. doxygenfunction:: bsp_set_tagsize
   :project: ebsp_e

ebsp_get_tagsize
^^^^^^^^^^^^^^^^

.. doxygenfunction:: ebsp_get_tagsize
   :project: ebsp_e

bsp_send
^^^^^^^^

.. doxygenfunction:: bsp_send
   :project: ebsp_e

bsp_qsize
^^^^^^^^^

.. doxygenfunction:: bsp_qsize
   :project: ebsp_e

bsp_get_tag
^^^^^^^^^^^

.. doxygenfunction:: bsp_get_tag
   :project: ebsp_e

bsp_move
^^^^^^^^

.. doxygenfunction:: bsp_move
   :project: ebsp_e

bsp_hpmove
^^^^^^^^^^

.. doxygenfunction:: bsp_hpmove
   :project: ebsp_e

ebsp_send_up
^^^^^^^^^^^^

.. doxygenfunction:: ebsp_send_up
   :project: ebsp_e

ebsp_move_chunk_down
^^^^^^^^^^^^^^^^^^^^

.. doxygenfunction:: ebsp_move_chunk_down
   :project: ebsp_e

ebsp_move_chunk_up
^^^^^^^^^^^^^^^^^^

.. doxygenfunction:: ebsp_move_chunk_up
   :project: ebsp_e

ebsp_move_down_cursor
^^^^^^^^^^^^^^^^^^^^^

.. doxygenfunction:: ebsp_move_down_cursor
   :project: ebsp_e

ebsp_reset_down_cursor
^^^^^^^^^^^^^^^^^^^^^^

.. doxygenfunction:: ebsp_reset_down_cursor
   :project: ebsp_e

ebsp_open_up_stream
^^^^^^^^^^^^^^^^^^^

.. doxygenfunction:: ebsp_open_up_stream
   :project: ebsp_e

ebsp_open_down_stream
^^^^^^^^^^^^^^^^^^^^^

.. doxygenfunction:: ebsp_open_down_stream
   :project: ebsp_e

ebsp_close_up_stream
^^^^^^^^^^^^^^^^^^^^

.. doxygenfunction:: ebsp_close_up_stream
   :project: ebsp_e

ebsp_close_down_stream
^^^^^^^^^^^^^^^^^^^^^^

.. doxygenfunction:: ebsp_close_down_stream
   :project: ebsp_e

ebsp_set_up_chunk_size
^^^^^^^^^^^^^^^^^^^^^^

.. doxygenfunction:: ebsp_set_up_chunk_size
   :project: ebsp_e

bsp_abort
^^^^^^^^^

.. doxygenfunction:: bsp_abort
   :project: ebsp_e
