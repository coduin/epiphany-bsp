# Changelog

## 1.0.0-beta.2 - 2016-04-21

### Added
- Add support for Epiphany SDK version 2016.3

### Fixed
- Fix non-aligned transfers using `ebsp_memcpy` (@kmate)
- Fix unnecessary arguments in interrupt handlers
- Make target barrier volatile
- Fix minor errors and inconsistencies in the documentation

### Removed
- Remove srec support from examples and tests

## 1.0b - 2015-10-21

### Added
- Update support for cross-compiling the library.
- Add data streaming to the library.
- Add function `ebsp_ext_malloc` for managing the external memory.
- Add dynamic memory management for local memory through `ebsp_malloc` and `ebsp_free`.
- Add support for easily using the DMA engine with alternative chaining through the library.
- Add `ebsp_barrier` for high performance synchronization between cores.
- Add primitives example showing how to use a combination of EBSP functions.
- Add Cannon's algorithm example for dense-dense matrix multiplication.
- Add streaming dot product example for large vectors.
- Add `ebsp_get_direct_address` to obtain a direct pointer to a remote variable.
- Add `ebsp_memcpy`, a more efficient implementation of `memcpy`.
- Add support for using `n < 16` cores.
- Use DMA interrupt handler to implement controllable DMA chaining
- Improved unit test coverage.
- Improved unit test script, support expected values of the form `output(pid)`.
- Improved error handling throughout the library.
- Greatly improved documentation and README.

### Fixed
- Move the code of many *slow* functions to external memory to save space in local memory.
- Fix local message passing sometimes using the wrong queue.
- Fix vertical message passing sometimes using the wrong queue.
- Fix general issue with read/write packets to external memory, sometimes breaking `ebsp_message`.
- Use `-O3` flag in the compilation of the library by default instead of the unstable `-Os` flag.
- Library source code is now formatted using `clang-format`.
- Speedup the lookup of remote addresses in BSP message passing system.
- Update LU decomposition example to use data streams and message passing.

### Removed
- Remove obsolete and incomplete examples.
- Remove the memory inspector.

## 1.0a - 2015-04-24
- Updated `bsp_init` to use the path relative to the host binary instead of current directory
- Rewrote both Makefiles to be cleaner
- Added `bsp_raw_time`, now implemented in assembly
- Added some documentation
- Rewrote all examples to use the message passing system instead of hardcoded addresses
- Separated the epiphany library into multiple files
- Added 'assembly' target in makefile to produce readable assembly
- Added `ebsp_ext_malloc` for allocating external memory, using self-made allocation table system
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
- All `bsp_push_reg` logic now happens on the epiphany side
- Added benchmark comparing `bsp_put` to `bsp_hpput`
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
