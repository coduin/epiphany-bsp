.. sectionauthor:: Jan-Willem Buurlage <janwillem@buurlagewits.nl>

.. highlight:: c

Output
======

Using `printf` on the Epiphany you can send messages to a debugger. However, it can be very convenient to write directly to `stdout` using the Epiphany cores. For this we provide a mechanism to output information from each individual Epiphany core to the `stdout` of the host program. For this you simply call `ebsp_message` completely identically to how you would call `printf` in regular programs.


Example
-------

Writing text::

    ebsp_message("Hello world!"); // -> $pid: Hello world!

You can also output information for local variables using standard formatting strings::

    ebsp_message("Hello world from core %i of %i!",
                 bsp_pid(), bsp_nprocs()); // -> $pid: Hello world from core pid of 16!

We would not recommend outputting floating point numbers through this method, because this pulls in a lot of floating-point conversion code which takes up precious memory and might even corrupt critical memory areas used by EBSP. In a future version we will provide a lightweight floating-point conversion implementation directly in our `ebsp_message` implementation.

Interface
------------------

.. doxygenfunction:: ebsp_message
   :project: ebsp_e
