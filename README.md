# Introduction

<todo>

# Components

- LLVM compiler suite tools: clang/clang++, lld, lldb, etc..
- compiler-rt: LLVM builtins runtime support
- LLVM libcxx: C++ library
- picolibc: embedded C library
- gba_cart.ld: linker script for GBA cart executables (as opposed to multiboot)
- gba_crt0.s: startup code for the GBA
- libtonc: GBA library
- Apex Audio System: play music and sound effects on the GBA
- Grit: GBA graphics swiss army knife
- gbafix: fix up GBA headers


# Usage

### command-line invocation

TLDR: use the following invocations to create a GBA program:

    <root>/bin/clang --config armv4t-gba.cfg -Wl,-T,gba_cart.ld program.c -o program.elf
    <root>/bin/llvm-objcopy -O binary program.elf program.gba
    <root>/bin/gbafix program.gba

#### armv4t-gba.cfg

Specifying `--config armv4t-gba.cfg` will include cmdline arguments found in the
`armv4t-gba.cfg` file found in the same directory as clang.

Which are:

- --target=armv-none-eabi -mcpu arm7tdmi
- -fno-exceptions
- -fno-rtti
- --sysroot <CFGDIR>/../lib/clang-runtimes/arm-none-eabi/armv4t
- <CFGDIR>/../lib/clang-runtimes/arm-none-eabi/armv4t/lib/gba_crt0.o
- -D_LIBCPP_AVAILABILITY_HAS_NO_VERBOSE_ABORT

Breakdown:

For cross-compiling, one of the main differences between GCC and LLVM is that
LLVM can target multiple architectures from the same binary. So LLVM needs to be
told this on the command-line by using what is called a target triple.

The GBA sports an ARM7TDMI chip, which is from the ARMv4T architecture
family. We're targeting the Arm Embedded ABI in a bare-metal environment, so the
target triple becomes:

    --target=arm-none-eabi

And we target the GBA CPU:

    -mcpu=arm7tdmi

We currently don't support C++ exceptions or RTTI, so for C++ programs we should
disable those:

    -fno-exceptions
    -fno-rtti

Furthermore the compiler should understand where it can find libraries and
headers. As Clang can potentially target multiple architectures and multiple
library variants, we need to supply a `--sysroot` argument. the libraries for
ARMv4T can be found at `toolchain-root/lib/clang-runtimes/arm-none-eabi/armv4t`:

    --sysroot <CFGDIR>/../lib/clang-runtimes/arm-none-eabi/armv4t

Where <CFGDIR> is a magic variable with points to the config file directory.

We also need to include the GBA startup code object file, called `gba_crt0.o`:

    <CFGDIR>/../lib/clang-runtimes/arm-none-eabi/armv4t/lib/gba_crt0.o

Lastly as we're on an embedded platform, and we usually don't have stderr and
friends available. Recently in verbose aborting has been enabled by default in
libcxx, which means that on an abort, an error message is printed. To stop the
compiler linking in symbols that don't exist, add:

    -D_LIBCPP_AVAILABILITY_HAS_NO_VERBOSE_ABORT

#### gba_cart.ld

Speaks  somewhat for itself.

The linker file can be found at `lib/clang-runtimes/arm-none-eabi/armv4t/lib/gba_cart.ld`. As we've set `--sysroot`, the linker can resolve `gba_cart.ld`.

In our example above, as we need to pass the linker file from the driver to the linker, the cmdline argument becomes:

    -Wl,-T,gba_cart.ld

This will set up the correct data regions for the GBA, which the startup code
uses to put code and data in the right place.

### Targeting memory regions

<todo>

### env variable

After unpacking the archive to the desired location, point the GBA_LLVM
environmental variable to the root of the installation:

    export GBA_LLVM=/root/of/your/installation

This variable will be used by the gba-llvm-devkit examples and some compatible
software packages to configure themselves correctly. Although the examples will
use the relative path to the toolchain if they can't find GBA_LLVM.


# Source installations

### prerequisites

Recent versions of the following tools are pre-requisites:
* A toolchain such as GCC/Clang & binutils
* git
* cmake
* make
* meson (On Linux the apt version has been known to fail, but pip installed
  version did wor For me 1.2.1 works.)
* ninja
* python3
* autoconf
* automake suite of tools
* libtoolize
* libfreeimage (for Grit)
* qemu to run general LLVM tests

#### Linux:

    sudo apt-get install python3 git make ninja-build qemu-system-arm libfreeimage-dev libtool automake autoconf cmake clang
    pip install meson

#### macOS:

- install Xcode from the app store to get Clang
- then use your favorite package manager (homebrew in the below example) to install the following dependencies:

    brew install python3 git make ninja qemu libfreeimage libtool automake autoconf cmake clang meson

#### Windows:

This toolchain hasn't been tested on Windows yet at all, but hopefully not too
much tweaking should be necessary


### Before you build on macOS:

Some tweaks are necesary for macOS to compile Grit:

- for macOs silicon you to explicitly set the include and lib paths of your
  package manager so Grit can find libfreeimage. This is a known issue:
  https://github.com/Homebrew/brew/issues/13481.
- GNU libtool has a name clashes with the macOS libtool which is different. The
  conventional workaround is to name GNU libtool glibtool, and consequently name
  libtoolize glibtoolize.
- The system/Xcode clang is stuck on c++98 by default. But Grit needs at minimum
  c++14.

To overcome this, CMakeList.txt sets some cache variables, so they can be
overwritten from the command line. The lib/include paths default to the Homebrew
default install location:

    LIBTOOLIZE=glibtoolize
    GRIT_LDFLAGS="LDFLAGS=-L/opt/homebrew/lib"
    GRIT_CPPFLAGS="CPPFLAGS=-I/opt/homebrew/include"
    GRIT_COMPILER="CXX=clang -std=c++14"

On non-macOS platforms LIBTOOLIZE is set to libtoolize and the rest of the vars
are empty.


### Commands to build:

    mkdir build
    cd build
    cmake .. -GNinja -DFETCHCONTENT_QUIET=OFF
    ninja install
    ninja package-gba-llvm

This will (should) create a functional toolchain in the 'build' dir:

- An install package with the general form: gba-llvm-devkit-1-Linux-AArch64.tar.xz
- An `install` directory that also contains a functional toolchain to use. But
  it also contains all the install targets of all the CMAKE subtargets that the
  package omits. So this includes a lot more LLVM tools for example, which might
  be useful for development of the toolchain itself.


### testing the toolchain/examples

    ninja check-llvm
    ninja check-picolib
    ninja check-compiler-rt
    ninja check-cxxabi
    ninja check-unwind
    ninja check-cxx

The below will check everything, but we're failing some expected llvm tests at
the moment, and after that the picolibc tests won't be run.

    ninja check-llvm-toolchain

Just checking the runtimes:

    ninja check-llvm-toolchain-runtimes

The toolchain doesn't have automated GBA tests yet, but you can run the examples
in the 'examples' dir. See below.


# examples

The distribution package comes with a couple of examples. Two very basic ones to
just test compiling a GBA program with C and C++.

The `slides` example is a poor-persons integration test of sorts, which uses
Tonc, Apex Audio System and Grit.

If the GBA_LLVM environmental variable is set, the examples will use this to
find the root of the toolchain. If not set, the examples will use the relative
path from the example dir to to the root dir of the toolchain.

Type `make` within an example directory to build.


# todo:

- convert/include Maxmod sound library.
- Include posprintf printf replacement.
- The GBFS tools are included in the distribution, but not the GBFS GBA library
  and header file.
- linker script for multiboot
- some kind of GBA-specific test suite
- add tonc examples repo
- maybe support stdout/stdwarn/stderr as some of the tonc text functionality
  depends on it
