# Recent versions of the following tools are pre-requisites:
# * A toolchain such as gcc & binutils
# * cmake
# * meson
# * ninja
# * python3
# * make and qemu to run tests

# Commands to build:
#   mkdir build
#   cd build
#   cmake ..
#   ninja
#   ninja check

# If the repos are checked out automatically then cmake will fetch the
# latest changes and check them out every time it runs. To disable this
# behaviour run:
#   cmake . -DFETCHCONTENT_FULLY_DISCONNECTED=ON

# If you prefer you can check out and patch the repos manually and use those:
#   mkdir repos
#   git -C repos clone https://github.com/stuij/gba-llvm.git
#   git -C repos clone https://github.com/devkitPro/gba-tools.git
#   mkdir build
#   cd build
#   cmake .. -GNinja -DFETCHCONTENT_SOURCE_DIR_LLVMPROJECT=../repos/llvm-project -DFETCHCONTENT_SOURCE_DIR_PICOLIBC=../repos/picolibc
#   ninja
#   ninja check-llvm-toolchain

# CONFIGURE_HANDLED_BY_BUILD was introduced in CMake 3.20 and it
# greatly speeds up incremental builds.
cmake_minimum_required(VERSION 3.20)

set(BUG_REPORT_URL "https://github.com/stuij/gba-llvm/issues" CACHE STRING "")

# Default to a release build
# (CMAKE_BUILD_TYPE is a special CMake variable so if you want to set
# it then you have to FORCE it).
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE BOOL "" FORCE)
endif()

if(NOT CMAKE_C_COMPILER_LAUNCHER AND NOT CMAKE_CXX_COMPILER_LAUNCHER)
    # If ccache is available then use it by default.
    find_program(CCACHE_EXECUTABLE ccache)
    if(CCACHE_EXECUTABLE)
        set(CMAKE_C_COMPILER_LAUNCHER "${CCACHE_EXECUTABLE}" CACHE FILEPATH "" FORCE)
        set(CMAKE_CXX_COMPILER_LAUNCHER "${CCACHE_EXECUTABLE}" CACHE FILEPATH "" FORCE)
    endif()
endif()

include(ExternalProject)
include(FetchContent)

# Read which revisions of the repos to use.
file(READ versions.json VERSIONS_JSON)
function(read_repo_version output_variable_prefix repo)
    string(JSON tag GET ${VERSIONS_JSON} "repos" "${repo}" "tag")
    string(JSON tagType GET ${VERSIONS_JSON} "repos" "${repo}" "tagType")
    if(tagType STREQUAL "commithash")
        # GIT_SHALLOW doesn't work with commit hashes.
        set(shallow OFF)
    elseif(tagType STREQUAL "branch")
        set(shallow ON)
        # CMake docs recommend that "branch names and tags should
        # generally be specified as remote names"
        set(tag "origin/${tag}")
    elseif(tagType STREQUAL "tag")
        set(shallow ON)
        set(tag "${tag}")
    else()
        message(FATAL_ERROR "Unrecognised tagType ${tagType}")
    endif()

    set(${output_variable_prefix}_TAG "${tag}" PARENT_SCOPE)
    set(${output_variable_prefix}_SHALLOW "${shallow}" PARENT_SCOPE)
endfunction()
read_repo_version(bmt bmt)

FetchContent_Declare(bmt
    GIT_REPOSITORY https://github.com/ARM-software/LLVM-embedded-toolchain-for-Arm
    GIT_TAG "${bmt_TAG}"
    GIT_SHALLOW "${bmt_SHALLOW}"
    GIT_PROGRESS TRUE
    # PATCH_COMMAND git reset --quiet --hard && git clean --quiet --force -dx
    # Add the llvm subdirectory later to ensure that
    # BMT is the first project declared.
    # Otherwise CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT
    # can't be used.
    SOURCE_SUBDIR do_not_add_bmt_subdir_yet
)

FetchContent_Declare(tools
    GIT_REPOSITORY https://github.com/devkitPro/gba-tools
    GIT_TAG "${tools_TAG}"
    GIT_SHALLOW "${tools_SHALLOW}"
    GIT_PROGRESS TRUE
    # PATCH_COMMAND git reset --quiet --hard && git clean --quiet --force -dx
    # Add the llvm subdirectory later to ensure that
    # LLVMEmbeddedToolchainForArm is the first project declared.
    # Otherwise CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT
    # can't be used.
    SOURCE_SUBDIR do_not_add_tools_subdir_yet
)

FetchContent_Declare(grit
    GIT_REPOSITORY https://github.com/devkitPro/grit
    GIT_TAG "${grit_TAG}"
    GIT_SHALLOW "${grit_SHALLOW}"
    GIT_PROGRESS TRUE
    # PATCH_COMMAND git reset --quiet --hard && git clean --quiet --force -dx
    # Add the llvm subdirectory later to ensure that
    # LLVMEmbeddedToolchainForArm is the first project declared.
    # Otherwise CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT
    # can't be used.
    SOURCE_SUBDIR do_not_add_grit_subdir_yet
)

FetchContent_MakeAvailable(bmt)
FetchContent_MakeAvailable(tools)
FetchContent_MakeAvailable(grit)

project(
    gba-llvm
    VERSION 1
    DESCRIPTION "LLVM-based C/C++ toolchain for Game Boy Advance development"
    HOMEPAGE_URL "https://github.com/stuij/gba-llvm"
)

# We generally want to install to a local directory to see what the
# output will look like rather than install into the system, so change
# the default accordingly.
# See https://cmake.org/cmake/help/latest/variable/CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT.html
# Note that this code only works after the first call to project so it
# can't be moved after the add_subdirectory command below.
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX
        "${CMAKE_BINARY_DIR}/install"
        CACHE PATH "" FORCE
    )
endif()

# Enable limiting the installed components in TGZ and ZIP packages.
set(CPACK_ARCHIVE_COMPONENT_INSTALL TRUE)
# Don't create a separate archive for each component.
set(CPACK_COMPONENTS_GROUPING ALL_COMPONENTS_IN_ONE)
# Strip debug info from files before packaging them
set(CPACK_STRIP_FILES TRUE)
# When extracting the files put them in an ArmCompiler-.../ directory.
set(CPACK_COMPONENT_INCLUDE_TOPLEVEL_DIRECTORY TRUE)
set(CPACK_PACKAGE_NAME "gba-llvm")

string(TOLOWER ${CMAKE_SYSTEM_PROCESSOR} processor_name)
string(REGEX MATCH "amd64|x64|x86" x86_match ${processor_name})
if(x86_match)
    set(processor_name "x86_64")
else()
    set(processor_name "AArch64")
endif()

set(CPACK_SYSTEM_NAME "${CMAKE_SYSTEM_NAME}-${processor_name}")

set(LLVM_TOOLCHAIN_LIBRARY_VARIANTS ON CACHE INTERNAL armv4t)

set(LLVM_ENABLE_PROJECTS clang;lld;lldb CACHE STRING "")
set(LLDB_ENABLE_LIBXML2 ON CACHE INTERNAL "")
# on MacOS we get warnings that LLDB tests won't pass properly
# because of code signing. We disregard for now and see what happens.
# set(LLDB_USE_SYSTEM_DEBUGSERVER ON CACHE STRING "")
# We need libcxx for testing but libcxx is added as an external project
# in the embedded toolchain configuration. So we go without for now.
set(LLDB_INCLUDE_TESTS OFF CACHE INTERNAL "")

add_subdirectory(
    ${bmt_SOURCE_DIR}
)

install(
    FILES
    DESTINATION .
    COMPONENT bmt
)

# Groups all the targets that comprise the toolchain.
add_custom_target(bmt ALL)

#add_dependencies(
#    bmt
# .. some other dependencies
#)

ExternalProject_Add(
  tools
  SOURCE_DIR ${tools_SOURCE_DIR}
  CONFIGURE_COMMAND ${tools_SOURCE_DIR}/configure --prefix=${CMAKE_CURRENT_BINARY_DIR}/gba-tools
  PREFIX gba-tools
  BUILD_COMMAND make
  INSTALL_COMMAND make install DESTDIR=${CMAKE_CURRENT_BINARY_DIR}
)

ExternalProject_Add_Step(
  tools prepare_configure
  WORKING_DIRECTORY ${tools_SOURCE_DIR}
  COMMAND aclocal
  COMMAND autoconf
  COMMAND automake --add-missing
  DEPENDERS configure
)

ExternalProject_Add(
  grit
  SOURCE_DIR ${grit_SOURCE_DIR}
  CONFIGURE_COMMAND ${CMAKE_COMMAND} -E env LDFLAGS=-L/opt/homebrew/lib "CXX=g++ -std=c++14" ${grit_SOURCE_DIR}/configure --prefix=${CMAKE_CURRENT_BINARY_DIR}/grit
  PREFIX grit
  BUILD_COMMAND make
  INSTALL_COMMAND make install DESTDIR=${CMAKE_CURRENT_BINARY_DIR}
)

ExternalProject_Add_Step(
  grit prepare_configure
  WORKING_DIRECTORY ${grit_SOURCE_DIR}
  COMMAND glibtoolize
  COMMAND aclocal
  COMMAND autoconf
  COMMAND automake --add-missing
  DEPENDERS configure
)

set(config "armv4t-gba.cfg")
set(gba_sources ${CMAKE_SOURCE_DIR}/common)
set(common ${CMAKE_SOURCE_DIR}/common)

set(target_triple "armv4t-none-eabi")
set(target_flags "-march=armv4t -fno-exceptions -fno-rtti --sysroot ${LLVM_BINARY_DIR}/lib/clang-runtimes/arm-none-eabi/armv4t")

ExternalProject_Add(
  gba_sources
  SOURCE_DIR ${gba_sources}
  PREFIX gba_sources
  DEPENDS llvm-toolchain
  CMAKE_ARGS
  -DLLVM_BINARY_DIR=${LLVM_BINARY_DIR}
  -DCMAKE_AR=${LLVM_BINARY_DIR}/bin/llvm-ar${CMAKE_EXECUTABLE_SUFFIX}
  -DCMAKE_ASM_COMPILER_TARGET=${target_triple}
  -DCMAKE_ASM_FLAGS=${target_flags}
  -DCMAKE_BUILD_TYPE=Release
  -DCMAKE_CXX_COMPILER=${LLVM_BINARY_DIR}/bin/clang++${CMAKE_EXECUTABLE_SUFFIX}
  -DCMAKE_CXX_COMPILER_TARGET=${target_triple}
  -DCMAKE_CXX_FLAGS=${target_flags}
  -DCMAKE_C_COMPILER=${LLVM_BINARY_DIR}/bin/clang${CMAKE_EXECUTABLE_SUFFIX}
  -DCMAKE_C_COMPILER_TARGET=${target_triple}
  -DCMAKE_C_FLAGS=${target_flags}
  -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
  -DCMAKE_NM=${LLVM_BINARY_DIR}/bin/llvm-nm${CMAKE_EXECUTABLE_SUFFIX}
  -DCMAKE_RANLIB=${LLVM_BINARY_DIR}/bin/llvm-ranlib${CMAKE_EXECUTABLE_SUFFIX}
  # Let CMake know we're cross-compiling
  -DCMAKE_SYSTEM_NAME=Generic
  INSTALL_COMMAND echo "nop"
)

ExternalProject_Add_Step(
  gba_sources install_config
  COMMAND ${CMAKE_COMMAND} -E copy ${common}/${config} ${LLVM_BINARY_DIR}/bin
  DEPENDERS configure
)

install(
  FILES
  ${common}/${config}
  DESTINATION bin
  COMPONENT llvm-toolchain
)

install(
  FILES
  ${CMAKE_CURRENT_BINARY_DIR}/gba_sources/gba_crt0.o
  ${common}/gba_cart.ld
  DESTINATION lib/clang-runtimes/arm-none-eabi/armv4t/lib
  COMPONENT llvm-toolchain
)

install(
  DIRECTORY
  ${CMAKE_CURRENT_BINARY_DIR}/gba-tools/src/tools-build/
  DESTINATION bin
  COMPONENT llvm-toolchain
  FILES_MATCHING PATTERN "*gb*"
  PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
)

install(
  DIRECTORY
  ${CMAKE_CURRENT_BINARY_DIR}/grit/src/grit-build/
  DESTINATION bin
  COMPONENT llvm-toolchain
  FILES_MATCHING PATTERN "grit"
  PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
)

install(
  FILES
  ${CMAKE_CURRENT_BINARY_DIR}/gba_sources/libtonc/lib/libtonc.a
  DESTINATION lib/clang-runtimes/arm-none-eabi/armv4t/lib
  COMPONENT llvm-toolchain
)

install(
  DIRECTORY
  ${CMAKE_CURRENT_BINARY_DIR}/gba_sources/libtonc/include/
  DESTINATION lib/clang-runtimes/arm-none-eabi/armv4t/include
  COMPONENT llvm-toolchain
  FILES_MATCHING PATTERN "*"
)
