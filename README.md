# Introduction

# Components

- clang/clang++
- picolibc
- LLVM libcxx
- libtonc
- linker script and startup code for generating roms that can run on GBA carts.


# Usage

After unpacking the archive to the desired location, point the GBA_LLVM environmental variable to the root of the installation:

    export GBA_LLVM=/root/of/your/installation

This variable will be used by the examples and some compatible software packages to configure themselves correctly.


# Source installations

Recent versions of the following tools are pre-requisites:
* A toolchain such as gcc & binutils
* cmake
* make
* meson (On Linux the apt version has been known to fail, but pip installed version did wor  For me 1.2.1 works.)
* ninja
* python3
* libtoolize or glibtoolize for MacOs
* libfreeimage (Grit)
* qemu to run general LLVM tests

### Before you build:

for MacOs Silicon I needed to explicitly set the include and lib paths of my
package manager. This seems to be a known issue:
https://github.com/Homebrew/brew/issues/13481

    export LDFLAGS="-L/opt/homebrew/lib"; export CPPFLAGS="-I/opt/homebrew/include"

Also make sure to set the proper value for the GRIT_LDPATH variable below.

### Commands to build:

    mkdir build
    cd build
    cmake .. -GNinja -DFETCHCONTENT_QUIET=OFF
    ninja install
    ninja package-gba-llvm

This will create a functional toolchain in the 'build/install' dir:

- An install package with the general form: gba-llvm-1-Darwin-AArch64.tar.xz
- An 'install' directory that also contains a functional toolchain to use. But
  it also contains all the install targets of all the CMAKE subtargets that the
  package omits. So this includes a lot more LLVM tools for example, which might
  be useful for development of the toolchain itself.


# todo:

- Include Maxmod sound library.
- Include posprintf printf replacement.
- The GBFS tools are included in the distribution, but not the GBFS GBA library
  and header file.
- linker script for multiboot