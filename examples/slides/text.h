char* text_slides[] = {
    "  Intro\n\n"

    "- Fairly obsessed with Game Boy Advance\n"
    "- Made my first substantial program on it\n"
    "- Taught me assembly, made a toy compiler\n"
    "- Got my first two programming jobs out of it\n"
    "- All good things came from the GBA\n"
    "- Got to work at Arm in LLVM Embedded team\n"
    "- Wanted LLVM-based toolchain\n"
    "- Not obvious how things fit together\n",


    "  Talk about\n\n"

    "- Embedded compiler how\n"
    "- GCC toolchain compatibility issues\n"
    "- Current LLVM embedded hot topics",


    "  Vehicle\n\n"

    "Game Boy Advance (GBA) toolchain\n"
    "  - Completely based on LLVM (no GCC)\n"
    "  - LLVM Embedded Toolchain for Arm (BMT)\n"
    "  - Completely open source\n"
    "  - https://github.com/stuij/gba-llvm-devkit\n"
    "  - 1st release\n"
    "  - Includes these slides (examples/slides)\n\n"

    "- Some of talk on BMT, some on GBA toolchain\n"
    "- Meant to be general tutorial, not BMT specific",


    "  Talk overview\n\n"

    "- Short history of embedded LLVM\n"
    "- Game Boy Advance (re)introduction\n"
    "- Example program\n"
    "- Embedded toolchain component overview\n"
    "- Build setup\n"
    "- Components: Compiler, LLD, etc..\n"
    "- Optimization!",


    "\n\n"
    "             Creating an embedded toolchain\n"
    "               for the Game Boy Advance",


    "\n\n"
    "             Creating an embedded toolchain\n"
    "               for the Game Boy Advance\n"
    "\n\n\n\n\n\n"
    "               Taming a dragon bit by bit",


    "  Very short LLVM embedded history\n\n"

    "- LLVM concentrated on hosted systems\n"
    "- Up until 2020 baremetal not much love\n"
    "- 2020 LLVM talk by Hafiz Abid Qadeer\n"
    "  * LLVM in a Bare Metal Environment\n"
    "- Now also support for RiscV, PowerPC, AArch64\n"
    "- LLVM Embedded Toolchains Working group\n",


    "  Game Boy Advance/ARM7TDMI overview\n\n"

    "- Nintendo handheld, successor of Game Boy\n"
    "- Released in March 2001, 82M units sold\n"
    "- Super Nintendo in your pocket\n"
    "- ARM7TDMI processor at 16.78 MHz, ARMv4T\n"
    "    * Arm, 32 bit instruction format\n"
    "    * Thumb, 16 bit instruction format\n"
    "- Nokia 6110 (1998), 3210 (1999), 3310 (2000)\n"
    "- Thought up by Dave Jagger, in trip to Japan in '94\n"
    "- 10 billion chips in total, 200M in 2020\n",


    "  Game Boy Advance internals\n\n"

    "- Picture Processing Unit (PPU)\n"
    "  * bitmap modes, tile modes\n"
    "- Orig Game Boy hardware\n"
    "  * No access, except for..\n"
    "- Sound\n"
    "  * Orig Game Boy PSG\n"
    "    + Manipulate 4 waveforms, 4 channels\n"
    "  * 2 channels to convert PGM format to DAC\n"
    "- Memory mapped IO all the way\n"
    "- Bunch of memory we will talk about later\n",


    "  Example\n\n"

    "Let's paint the screen red:\n"
    "\n"
    "int main() {\n"
    "  draw_screen();\n"
    "  while(1) {};\n"
    "  return 1;\n"
    "}",


    "\n"
    "void draw_screen() {\n"
    "  // set bg mode 3 (bitmap) and turn on bg 2\n"
    "  // 0x4000000 == REG_DISPCNT\n"
    "  *(unsigned long*) 0x4000000 = 0x403;\n"
    "\n"
    "  u16* vram = (u16*) 0x6000000;\n"
    "  // write red to all pixels on 240x160 screen\n"
    "  for(int x = 0; x < 240; x++)\n"
    "    for(int y = 0; y < 160; y++)\n"
    "      vram[x + y * 240] = 31;\n"
    "}",


    "  Toolchain",


    "  Components\n\n"

    "LLVM compiler suite tools\n"
    "compiler-rt builtins: target-specific code\n"
    "LLVM libcxx: libc++\n"
    "picolibc: embedded C library\n"
    "gba_cart.ld: linker script\n"
    "gba_crt0.s: startup code for the GBA\n"
    "libtonc: GBA low-level library\n"
    "Apex Audio System: GBA music library\n"
    "Grit: GBA graphics swiss army knife\n"
    "gbafix: fix up GBA headers",


    "  Invocation\n\n"

    "- grit bg.bmp -gB4 -Mw8 -Mh8 -pS -obg -fa -pT1\n"
    "- conv2aas AAS_Data\n"
    "- clang --config gba.cfg -Wl,-T,gba_cart.ld\n"
    "      program.c bg.s AAS_Data.s -o game.elf\n"
    "- llvm-objcopy -O binary game.elf game.gba\n"
    "- gbafix program.gba",


    "  gba.cfg\n\n"

    "--target=armv4t-none-eabi -mfpu=none\n"
    "-fno-exceptions\n"
    "-fno-rtti\n"
    "--sysroot <CFGDIR>/../lib/clang-runtimes/\n"
    "    arm-none-eabi/armv4t\n"
    "-lcrt0-gba\n"
    "-D_LIBCPP_AVAILABILITY_HAS_NO_VERBOSE\n"
    "    _ABORT\n",


    "  Toolchain layout\n\n"

    "bin/\n"
    "  compiler tools, gba tools, gba.cfg\n"
    "lib/clang-runtimes/\n"
    "  multilib.yaml\n"
    "  arm-none-eabi/armv4t/\n"
    "    lib/\n"
    "      libcrt0-gba.a, gba_cart.ld, libraries\n"
    "    include/\n"
    "      headers",


    "  Dependency graph",


    "  Build setup\n\n"

    "- GBA toolchain built on top of BMT\n"
    "- Uses CMake. Very composable, feels bit icky\n"
    "  * Download dependency sources\n"
    "  * ExternalProject_Add -> target\n"
    "    # Feed them CMAKE_ARGS\n"
    "  * External project for GBA libraries\n"
    "- Loop through library variants\n"
    "- Piggyback on everything: CPACK, testing, infra\n"
    "- Theme: target/host lib and includes mixing\n"
    "- Use BMT as example",


    "  CMake argument themes\n\n"

    "- Set target\n"
    "- Specify building for embedded\n"
    "  * -DLIBCXX_ENABLE_STATIC=ON\n"
    "  * CMAKE_SYSTEM_NAME=Generic\n"
    "  * -D*_BAREMETAL*=ON\n"
    "- Feature options\n"
    "- Where are tools, config files\n"
    "- Where is target sysroot\n"
    "- test/build configuration",


    "  CMake example: compiler-rt\n\n"

    "-DCMAKE_TRY_COMPILE_TARGET_TYPE=\n"
    "    STATIC_LIBRARY\n"
    "-DCOMPILER_RT_BUILD_BUILTINS=ON\n"
    "-DCOMPILER_RT_BUILD_SANITIZERS=OFF\n"
    "-DCOMPILER_RT_BUILD_XRAY=OFF\n"
    "-DCOMPILER_RT_BUILD_LIBFUZZER=OFF\n"
    "-DCOMPILER_RT_BUILD_PROFILE=OFF\n"
    "-DCMAKE_C_COMPILER_TARGET=<triple>\n"
    "-DCMAKE_ASM_COMPILER_TARGET=<triple>",


    "  CMake: compiler-rt cont..\n\n"

    "-DCMAKE_C_COMPILER=${BIN}/clang\n"
    "-DCMAKE_AR=${BIN}/llvm-ar\n"
    "-DCMAKE_NM=${BIN}/llvm-nm\n"
    "-DCMAKE_RANLIB=${BIN}/llvm-ranlib\n"
    "-DCOMPILER_RT_BAREMETAL_BUILD=ON\n"
    "-DCOMPILER_RT_DEFAULT_TARGET_ONLY=\n"
    "    ON\n"
    "-DLLVM_CONFIG_PATH=${BIN}/llvm-config\n"
    "-DCOMPILER_RT_INCLUDE_TESTS=OFF",


    "  CMake: runtimes\n\n"

    "-DLIBCXXABI_USE_COMPILER_RT=ON\n"
    "-DLIBCXXABI_USE_LLVM_UNWINDER=ON\n"
    "-DLIBCXX_ENABLE_RANDOM_DEVICE=OFF\n"
    "-DLIBCXX_ENABLE_RTTI=OFF\n"
    "-DLIBCXX_ENABLE_WIDE_CHARACTERS=OFF\n"
    "-DCMAKE_SYSTEM_NAME=Generic\n"
    "-DCMAKE_TRY_COMPILE_TARGET_TYPE=\n"
    "    STATIC_LIBRARY\n"
    "-DLIBC_LINKER_SCRIPT=picolibcpp.ld\n",


    "  CMake: runtimes cont..\n\n"

    "-DLIBCXXABI_BAREMETAL=ON\n"
    "-DLIBCXXABI_ENABLE_STATIC=ON\n"
    "-DLIBCXXABI_ENABLE_ASSERTIONS=OFF\n"
    "-DLIBCXXABI_LIBCXX_INCLUDES=\n"
    "    '${INSTALL_DIR}/include/c++/v1'\n"
    "-DCOMPILER_RT_INCLUDE_TESTS=ON\n"
    "-DCMAKE_INSTALL_PREFIX=<DIR>",


    "Individual components\n\n"

    "What to look out for\n"
    "GBA issues\n"
    "Hot topics",


    "Clang\n\n"

    "- ARMv4 lowest supported Arm arch in LLVM\n"
    "- Arm: Clang defaults to ARMv4T/ARM7TDMI\n"
    "- Bare metal file:\n"
    "  clang/lib/Driver/ToolChains/BareMetal.cpp\n"
    "- handles things like -nostdinc, multilib\n"
    "- Compiling old GBA programs:\n"
    "  * Not too happy with pre-UAL syntax\n"
    "    + Thumb: mov -> movs\n"
    "    + ARM: stmneia -> stmiane\n"
    "  * No long-call fn attr, no -mthumb-interwork",


    "  Multilib issues\n\n"

    "- embedded affliction, complex build systems\n"
    "- GCC has spec files\n"
    "- issues to solve\n"
    "  * need to build lots of libraries for variants\n"
    "  * Libraries not mapping to target triples\n"
    "  * hardwiring library paths in C++ code\n",


    "  Multilib solution\n\n"

    "- More flexible option based on:\n"
    "  * normalizing cmdline arguments\n"
    "  * yaml files containing rules\n"
    "    + variant\n"
    "    + matchers\n"
    "- Sets –sysroot, –isystem, -L\n"
    "- Can compose multiple matches\n"
    "- Currently not working for GBA\n"
    "  * Tiny bug when building for thumb\n"
    "  * Can be fixed with matcher rule.\n",


    "  Linking\n\n",

    "- GBA memory region overview\n"
    "- Linker script overview\n"
    "- Linker issues\n"
    "- Linker what's hot",


    "  Linker script/startup code what\n\n"

    "- Linker script\n"
    "  * Define memory regions\n"
    "  * What goes in them\n"
    "- startup code\n"
    "  * Copy data from rom to ram\n"
    "  * Zero initialize .bss\n"
    "  * Call routines from .init_array\n"
    "  * Call main\n",


    "  Linker issues\n\n"

    "- For ARMv4T\n"
    "   * Long branches and Arm/Thumb interworking"
    "     - uses BLX\n"
    "     - no MOVW/MOVT\n"
    "- For GBA:\n"
    "  * Using DevkitPro linker script \n"
    "  * LLD didn't accept overlay syntax\n"
    "  * C++ data section names\n"
    "  * clib incompatibilities\n"
    "  * Add picolibc stubs",


    "  Linker improvements\n\n"

    "- LTO\n"
    "  * Respect code regions\n"
    "  * Still bunch of issues\n"
    "  * Code size can worsen\n"
    "  * Goal is to implement API\n"
    "- Distribute code over memory locations\n"
    "- Compression\n"
    "- Debuggability\n"
    "- Place section at specific address\n"
    "- Automatic overlays",


    "  compiler-rt\n\n"

    "- Runtime support\n"
    "- builtins: low-level target-specific hooks\n"
    "- libgcc drop-in replacement\n"
    "- Also sanitizers, profiling, fuzzing, xray,\n"
    "    scudo, BlocksRuntime, etc..\n"
    "- Currently only builtins\n"
    "- For ARMv4(T) there was no builtins target\n"
    "  * Easy to fix\n"
    "- You get ubsan for free. It's cheap.\n"
    "- Demo time",


    "  clib\n\n"

    "- Newlib(-nano), picolibc, LLVM Libc, etc..\n"
    "- Went with picolibc\n"
    "  * BSD license\n"
    "  * Newlib doesn’t have LLVM config\n"
    "  * Responsive maintainers\n"
    "  * More suitable for embedded development\n"
    "- LLVM Libc \n"
    "  * Looks promising, but needs work\n"
    "  * Talks underway on scalability\n"
    "- GBA: implement stdout and friends",


    " libcxx\n\n"

    "- LLVM libcxx\n"
    "  * Make more efficient for embedded\n"
    "- On GBA\n"
    "  * Butano, a popular GBA C++ engine, uses ETL\n"
    "      (Embedded Template Library)\n"
    "    + Optimized for embedded applications\n"
    "- Distributed as source?\n"
    "- carving of functionality",


    "  LLDB\n\n"

    "- Works with mGBA emulator\n"
    "- Also works from VSCode\n"
    "  * Use CodeLLDB or lldb-vscode from toolchain\n"
    "- Shout-out to David Spickett",


    "  GBA tools\n\n"

    "- Grit\n"
    "- gbafix\n"
    "- GBFS\n"
    "- Maxmod\n"
    "- posprintf\n"
    "- Apex Audio System\n"
    "- Aseprite\n"
    "- Propulse Tracker",


    "  Performance / Coremark\n\n"

    "- Industry standard embedded benchmark\n"
    "- (iterations/sec) / Mhz, single-core\n"
    "- examples:\n"
    "  * Cortex-M0 minimal gate-count: 2.33\n"
    "  * Cortex-M85 high end embedded: 6.28\n"
    "  * NXP LS2088A (Cortex-A72): 36.10\n"
    "- Small binary size (sub 30K), fits in IWRAM\n"
    "- loop unrolling: from 26K to 140K\n"
    "- jump threading, inline|unroll-threshold\n"
    "  * less size bloat (~ 1K each)",


    "  The end\n\n"

    "- did we actually manage to get here?\n"
    "- questions?"
};

u32 text_slide_len = sizeof(text_slides)/sizeof(text_slides[0]);
u32 text_slide_cur = 0;

void write_slides() {
  tte_write("#{es;P}");
  tte_write(text_slides[text_slide_cur]);
}

void toggle_text() {
  REG_BLDCNT ^= 3 << 6;
  REG_DISPCNT ^= DCNT_BG0;
}

void text_on() {
  REG_BLDCNT |= 3 << 6;
  REG_DISPCNT |= DCNT_BG0;
}

void update_slides(s32 key_tri) {
  if (key_tri < 0) {
    if (text_slide_cur == 0)
      text_slide_cur = text_slide_len - 1;
    else
      text_slide_cur--;
    text_on();
    write_slides();
  } else if (key_tri > 0) {
    if (text_slide_cur == text_slide_len - 1)
      text_slide_cur = 0;
    else
      text_slide_cur++;
    text_on();
    write_slides();
  }
}
