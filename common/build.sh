TOOLCHAIN=../LLVMEmbeddedToolchainForArm-17.0.0-Darwin-AArch64/
TOOLS=$TOOLCHAIN/bin
CLANG=$TOOLS/clang
$CLANG -o $TOOLCHAIN/lib/clang-runtimes/arm-none-eabi/armv4t/lib/gba_crt0.o --target=armv4t-none-eabi -mfloat-abi=soft -march=armv4t -c gba_crt0.s
