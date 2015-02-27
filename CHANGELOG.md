# Changelog

## 0.2
- Added `ebsp_message`
- External memory in one large block
- Fixed hardcoded BSP addresses to no longer overlap
- All BSP variables are stored in a struct
- Host checks for `MAX_N_REGISTER` and outputs warning if reached.
- Added memory inspector with `ebsp_inspector_enable()`.

## 0.1
- Initial release
