# Changelog

## 0.2
- Added `ebsp_message`
- Shared memory in one large block (wrapper read/write functions with offset)
- Fixed hardcoded BSP addresses to no longer overlap
- Host checks for `MAX_N_REGISTER` and outputs warning if reached.
- Added memory inspector with `ebsp_inspector_enable()`.

## 0.1
- Initial release
