# Introduction

This is a C/C++ Game Boy Advance toolchain based on LLVM.

Currently there are binary packages available for Linux (x86_64, AArch64) and
macOS (arm64). See the
[https://github.com/stuij/gba-llvm-devkit/releases](release) page to grab the
latest packages.

The CMakefile.txt file in the source repo will build all the LLVM and GBA tools
and components from scratch. See the [Building from
source](#building-from-source) section below for instructions.


# Why

No good reason but 'seemed like a fun idea at the time'. Mostly to make sure
that LLVM would not loose the ability to compile for ARMv4T. For example
compiler-rt and LLD didn't support ARMv4T before this was fixed as part of this
project.

Also, the more choice the better. It's good to have an alternative to GCC. And the GBA
is a fun platform to try LLVM concepts on.


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

These are essentially the same commands you'd execute when using DevkitArm,
except that `-specs=gba.specs` is replaced with `--config armv4t-gba.cfg`.

See the below sections for a breakdown of what's involved.

#### armv4t-gba.cfg

Specifying `--config armv4t-gba.cfg` will include cmdline arguments found in the
`armv4t-gba.cfg` file found in the same directory as clang.

Which are:

- `--target=armv4-none-unknown-eabi`
- `-mfpu=none`
- `-fno-exceptions`
- `-fno-rtti`
- `--sysroot <CFGDIR>/../lib/clang-runtimes/arm-none-eabi/armv4t`
- `-lcrt0-gba`
- `-D_LIBCPP_AVAILABILITY_HAS_NO_VERBOSE_ABORT`

Breakdown:

For cross-compiling, one of the main differences between GCC and LLVM is that
LLVM can target multiple architectures from the same binary. So LLVM needs to be
told this on the command-line by using what is called a target triple.

The GBA sports an ARM7TDMI chip, which is from the ARMv4T architecture
family. We're targeting the Arm Embedded ABI in a bare-metal environment, so the
target triple becomes:

    --target=armv4-none-unknown-eabi

And we have no fpu:

    -mfpu=none

We currently don't support C++ exceptions or RTTI, so for C++ programs we should
disable those:

    -fno-exceptions
    -fno-rtti

Furthermore the compiler should understand where it can find the correct
libraries and headers. As Clang can potentially target multiple architectures
and multiple library variants, we need to supply a `--sysroot` argument. the
libraries for ARMv4T can be found at
`<root>/lib/clang-runtimes/arm-none-eabi/armv4t`:

    --sysroot <CFGDIR>/../lib/clang-runtimes/arm-none-eabi/armv4t

Where \<CFGDIR> is a magic variable with points to the config file
directory. This should be handled by Clang's multiboot, but there's currently an
outstanding bug when targetting Thumb, so for now this works too.

We also need to include the GBA startup code library, which is resolved relative
to `sysroot`.

    -lcrt0-gba

Lastly as we're on an embedded platform, and we usually don't have stderr and
friends available. Recently in verbose aborting has been enabled by default in
libcxx, which means that on an abort, an error message is printed. To stop the
compiler linking in symbols that don't exist, add:

    -D_LIBCPP_AVAILABILITY_HAS_NO_VERBOSE_ABORT

#### gba_cart.ld

Speaks  somewhat for itself.

The linker file can be found at
`<root>/lib/clang-runtimes/arm-none-eabi/armv4t/lib/gba_cart.ld`. As we've set
`--sysroot`, the linker can resolve `gba_cart.ld`.

In our example above, as we need to pass the linker file from the driver to the
linker, the cmdline argument becomes:

    -Wl,-T,gba_cart.ld

This will set up the correct data regions for the GBA, which the startup code
uses to put code and data in the right place.

### Targeting memory regions

There are three memory regions that code and data could reside in:
- `iwram` (internal working ram)
- `ewram` (external working ram)
- the default is cartidge rom

`iwram` and `ewram` are mapped to linker sections `iwram` and `ewram`, so use
the appropriate C or asm declarations:

- C: `extern void foobar(void) __attribute__((section("iwram")));`
- arm: `.section iwram`

Alternately, filenames of the form `*.iwram.c` or `*.ewram.c` will be placed in
the appropriate section.

### Env variable

After unpacking the archive to the desired location, point the GBA_LLVM
environmental variable to the root of the installation:

    export GBA_LLVM=/root/of/your/installation

This variable will be used by the gba-llvm-devkit examples and some compatible
software packages to configure themselves correctly. Although the examples will
use the relative path to the toolchain if they can't find GBA_LLVM.

### Known command-line differences between GCC and LLVM

There are of course a lot of command line differences between GCC and LLVM. But
if you've used DevkitArm, these are the ones you might have a good chance to run
into:

- LLVM has no support for `-mthumb-interwork`. This is fine because these days,
  this option does nothing (AAPCS calling convention is the default).
- LLVM does not support the `long-call` function attribute. Which is also fine,
  because these days the linker automatically places range extention thunks for
  long calls.
- LLVM does not support spec files. It copes by using multilib and config files.

Let us know if you encounter others worth mentioning.


# Building from source

### repo

https://github.com/stuij/gba-llvm-devkit

### Prerequisites

Recent versions of the following tools are pre-requisites:
* A toolchain such as GCC/Clang & binutils
* git
* cmake
* make
* meson (Use pip for latest version. 1.2.1 works.)
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

- then use your favorite package manager (homebrew in the below example) to
  install the following dependencies:

```  brew install python3 git make ninja qemu libfreeimage libtool automake autoconf cmake clang meson```

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
    ninja

    ninja install
    # or
    ninja package-gba-llvm

This will (should) create a functional toolchain in the 'build' dir:

- `ninja package-gba-llvm` will create a package with the general form:
  gba-llvm-devkit-1-Linux-AArch64.tar.xz
- `ninja install` will install all the needed files to the `install` directory
  that will contains all the install targets of all the CMAKE subtargets, some
  of which `ninja package-gba-llvm omits. So this includes a lot more LLVM tools
  for example, which might be useful for development of the toolchain itself.


### Testing the toolchain/examples

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


# Examples

The distribution package comes with a couple of examples.

Two very basic ones to just test compiling a GBA program with C and C++ and
another basic one, called `ubsan`, triggers an undefined behaviour sanitizer.

A more involved one is the `slides` example, which is a poor-persons
integration test of sorts, which uses Tonc, Apex Audio System and Grit.

If the GBA_LLVM environmental variable is set, the examples will use this to
find the root of the toolchain. If not set, the examples will use the relative
path from the example dir to to the root dir of the toolchain.

Type `make` within an example directory to build.


# Issues

For bug reports or any questions, create an issue on the source repo:
https://github.com/stuij/gba-llvm-devkit/issues

# Todo

- convert/include Maxmod sound library.
- Include posprintf printf replacement.
- The GBFS tools are included in the distribution, but not the GBFS GBA library
  and header file.
- linker script for multiboot
- some kind of GBA-specific test suite
- add tonc examples repo
- support stdout/stdwarn/stderr
- debug why memcpy seems to be so slow
- macOS universal libraries
- LLVM currently does no ARMv4T-specific optimizations. Should be a bunch of
  low-hanging fruit (for example loop unrolling).

# Resources

If you want to learn how to do Game Boy Advance development, but don't know
where to start, this page will get you on your way:

https://github.com/gbadev-org/awesome-gbadev

# Changelog

v1:

Initial release. Packages for Linux and macOS. Tested by compiling a relatively
involved rom.

No immediately known issues exept for that memcpy seems to be really slow. Needs
an investigation. Workaround: use memcpy16/memcpy32.
