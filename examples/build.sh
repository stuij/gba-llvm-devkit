TOOLCHAIN=../build/install
TOOLS=$TOOLCHAIN/bin
CLANG=$TOOLS/clang
CLANG_PP=$TOOLS/clang++
LD_SCRIPT=gba_cart.ld
CONFIG=armv4t-gba.cfg

# build the C program
$CLANG --config $CONFIG -c -o test.o test.c
$CLANG --config $CONFIG -T $LD_SCRIPT -o test.elf test.o
$TOOLS/llvm-objcopy -O binary test.elf test.gba
$TOOLS/gbafix test.gba

# build the C++ program
$CLANG_PP --config $CONFIG -c -o test_cpp.o test.cpp
$CLANG_PP --config $CONFIG -T $LD_SCRIPT -o test_cpp.elf test_cpp.o -D_LIBCPP_AVAILABILITY_HAS_NO_VERBOSE_ABORT
$TOOLS/llvm-objcopy -O binary test_cpp.elf test_cpp.gba
$TOOLS/gbafix test_cpp.gba
