# coroutine

A stateful coroutine library.

## Build

### Clang on windows:

The [Visual C++](https://www.visualstudio.com) and the [clang](http://clang.llvm.org/) are required to be installed on your windows system.


1.  Initializes Visual C++ command-line build environment variables.

        >call "vcvarsall.bat" x64

2.  Specifies the toolchain (clang) with build flags. The 'clang.exe' can not work with Visual C++, use 'clang-cl.exe' to instead.

        >set CC=clang-cl
        >set CXX=clang-cl
        >set CFLAGS=-m64 -fmsc-version=1910
        >set CXXFLAGS=-m64 -fmsc-version=1910

3.  Build from source:

*   Using cmake with GNU make build system.

        >cmake ../ -G"Unix Makefiles"
        >make

*   Using cmake with ninja build system.

        >cmake ../ -GNinja
        >ninja



