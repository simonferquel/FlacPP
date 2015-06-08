# What is it ?
FlacPP is a pure C++ implementation of a Flac decoder inspired by the reference implementation written in C code.
It features some specific optimizations for ARM NEON (for LPC signal resolving) and leverages auto vectorization for some other parts (Rice block decoding)

# What platforms are targetted ?
All Windows based platforms are supported (Windows Desktop, Windows Store, Windows Phone).
Android support is quite experimental but functionnal. Android Build is based on Microsoft MSBuild for cross platform, and uses Clang under the hood. The structure of the project is quite simple, so baking MakeFiles for the library should not be too painful.

Supported processor architectures are X86, X64 and ARM. ARM implementation relies on ARM Neon specific code for better performance. X86/X64 code does not make use of any intrisics, so it can be used as a baseline to target other platforms.

# how do I use it ?

The main class here is FlacPP::FlacDecoder, and it relies entirely on an InputStream interface that can be implemented to suit the targetted platform. The project comes with an InputStream implementation based on std::istream and derived, and an implementation over WinRT IRandomAccessStream. The android sample app also features an implementation over an AAsset* component.

The FlacDecoder provides a very simple interface : the constructor validates the underlying stream, the streamInfo() method expose usefull metadata to the client, decodeNextFrame extract PCM data for the next frame (and gives the corresponding timestamp to the caller), and seek() allows to change the position inside the current track.

# thread safety 

Each FlacDecoder instance is not threadsafe, but is "free-threaded" (that is, as long as only one thread at a time calls methods on a given instance, everything is fine). However, 2 FlacDecoder instances can be manipulated simultaneously from 2 different threads.

However, beware of the InputStream implementation you are using : on many platforms, std::ifstream instances share states and thus, it is not safe to read from 2 different ifstream simultaneously. Thus, when using std::ifstream as the underlying stream, it is not safe to manipulate 2 FlacDecoder simultaneously.

Feel free to do pull requests !

Simon
