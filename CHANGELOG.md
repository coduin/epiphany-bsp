# Changelog

## 0.2
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
