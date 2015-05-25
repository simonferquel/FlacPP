# What is it ?
FlacPP is a pure C++ implementation of a Flac decoder inspired by the reference implementation written in C code.
It features some specific optimizations for ARM NEON (for LPC signal resolving) and leverages auto vectorization for some other parts (Rice block decoding)

# What platforms are targetted ?
For now, only Windows (Desktop, Store) and Windows Phone are supported, but the only non-standard functions used are _byteswap_ulong, _byteswap_uint64, _byteswap_ushort and _bitscanReverse32 as well as ARM neon intrinsics that should behave the same on other ARM compilers.
So it should be easy to port the code to other platforms.

Feel free to do pull requests !

Simon
