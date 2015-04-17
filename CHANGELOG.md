# Changelog

## 0.3
- Updated bsp_init to use the path relative to the host binary instead of current directory
- Rewrote both Makefiles to be cleaner
- Added bsp_raw_time, now implemented in assembly
- Added some documentation
- Rewrote all examples to use the message passing system instead of hardcoded addresses
- Separated the epiphany library into multiple files
- Added 'assembly' target in makefile to produce readable assembly
- Added ebsp_ext_malloc for allocating external memory, using self-made allocation table system
- Modified bspbench to use less iterations for higher h-values
- Makefile now detects architecture and uses cross compiler if needed
- Implemented BSP abort using an Epiphany instruction that halts all cores, and host code to handle this
- Added example that will show usage of all primitives
- Moved large BSP functions to external memory
- Implemented BSP message passing from host to core and back
- Makefile updated, library now optimized for size
- Epiphany core data is no longer tightly packed for better speed and reduced code size
- Implemented BSP Message passing
- Host does no longer take part in synchronization
- All cores now use the same space to store data payloads (mutexed)
- All bsp_push_reg logic now happens on the epiphany side
- Added benchmark comparing bsp_put to bsp_hpput

## 0.2
- All shared structures now have maximal packing because the two compilers use non-compatible alignments
- All cores now use the same memory for `bsp_put` data payload allowing a lot larger payloads
- Moved `bsp_var_list` from on-core memory to external memory
- Host no longer interferes with variable registrattion
- Added `bsp_get`, `bsp_hpget` and `bsp_put`
- Fixed timer to use real time instead of clockcycles
- Added `ebsp_message`
- External memory is now in one large block
- Fixed hardcoded BSP addresses to no longer overlap
- All BSP variables are stored in a struct
- Host checks for `MAX_N_REGISTER` and outputs warning if reached.
- Added memory inspector with `ebsp_inspector_enable()`.
- Added LU decomposition example 

## 0.1
- Initial release
