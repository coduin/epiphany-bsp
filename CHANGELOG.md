# Changelog

0.1
- Added `ebsp_message`
- Shared memory in one large block (wrapper read/write functions with offset)
- Fixed hardcoded BSP addresses to no longer overlap
- Host checks for `MAX_N_REGISTER` and outputs warning if reached.