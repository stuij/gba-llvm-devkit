# Copyright (c) 2023, Ties Stuij
# SPDX-License-Identifier: Apache-2.0
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# See repo README.md for more information on how to build
#
# address some MacOs compatibility issues"
# * On MacOs, for libtool, the convention is to rename libtool(ize) to
#   glibtool(ize), to avoid a name conflict with NeXT libtool.
# * Homebrew libraries don't seem to propagate
# * MacOs Clang by default compiles with '98. But for grit we need '14
#   (g++ is an alias for Clang++)
if(APPLE)
    set(LIBTOOLIZE glibtoolize CACHE STRING "set libtoolize binary")
    set(GRIT_LDFLAGS LDFLAGS=-L/opt/homebrew/lib CACHE STRING "set LDFLAGS for Grit")
    set(GRIT_CPPFLAGS CPPFLAGS=-I/opt/homebrew/include CACHE STRING "set CPPFLAGS for Grit")
    set(GRIT_COMPILER "CXX=clang -std=c++14" CACHE STRING "set compiler for Grit")
else()
    set(LIBTOOLIZE libtoolize CACHE STRING "set libtoolize binary")
    set(GRIT_LDFLAGS "" CACHE STRING "set LDFLAGS for Grit")
    set(GRIT_CPPFLAGS "" CACHE STRING "set CPPFLAGS for Grit")
    set(GRIT_COMPILER "" CACHE STRING "set compiler for Grit")
endif()

set(LLVM_DISTRIBUTION_COMPONENTS
    clang-format
    clang-resource-headers
    clang
    dsymutil
    lld
    lldb
    lldb-server
    lldb-vscode
    llvm-ar
    llvm-config
    llvm-cov
    llvm-cxxfilt
    llvm-dwarfdump
    llvm-nm
    llvm-lto
    llvm-lto2
    llvm-objcopy
    llvm-objdump
    llvm-profdata
    llvm-ranlib
    llvm-readelf
    llvm-readobj
    llvm-size
    llvm-strip
    llvm-strings
    llvm-symbolizer
    LTO
    CACHE INTERNAL ""
)

set(LLVM_TOOLCHAIN_DISTRIBUTION_COMPONENTS
    llvm-toolchain-libs
    llvm-toolchain-third-party-licenses
    gba-toolchain
    gba-toolchain-third-party-licenses
    gba-toolchain-examples
    CACHE INTERNAL "Components defined by this CMakeLists that should be
installed by the install-llvm-toolchain target"
)

# set(CPACK_COMPONENTS_ALL ${LLVM_TOOLCHAIN_DISTRIBUTION_COMPONENTS} ${LLVM_DISTRIBUTION_COMPONENTS} gba_toolchain CACHE INTERNAL "")

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
    gba-llvm-devkit
    VERSION 1
    DESCRIPTION "LLVM-based C/C++ toolchain for Game Boy Advance development"
    HOMEPAGE_URL "https://github.com/stuij/gba-llvm-devkit"
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
set(CPACK_PACKAGE_NAME "gba-llvm-devkit")

set(LLVM_ENABLE_PROJECTS clang;lld;lldb CACHE STRING "")
set(LLDB_ENABLE_LIBXML2 ON CACHE INTERNAL "")
# on MacOS we get warnings that LLDB tests won't pass properly
# because of code signing. We disregard for now and see what happens.
# set(LLDB_USE_SYSTEM_DEBUGSERVER ON CACHE STRING "")
# We need libcxx for testing but libcxx is added as an external project
# in the embedded toolchain configuration. So we go without for now.
set(LLDB_INCLUDE_TESTS OFF CACHE INTERNAL "")

set(LLVM_TOOLCHAIN_LIBRARY_VARIANTS armv4t CACHE INTERNAL "")

add_subdirectory(
    ${bmt_SOURCE_DIR}
)

install(
    FILES
    DESTINATION .
    COMPONENT llvm-toolchain
)

# Groups all the targets that comprise the toolchain.
add_custom_target(gba-llvm ALL)

add_dependencies(
    install-llvm-toolchain
    gba-llvm
)

add_custom_target(install-gba-llvm)
add_dependencies(
    install-gba-llvm
    install-llvm-toolchain
)

add_custom_target(package-gba-llvm)
add_dependencies(
    package-gba-llvm
    package-llvm-toolchain
)

add_custom_target(unpack-gba-llvm)
add_dependencies(
    unpack-gba-llvm
    unpack-llvm-toolchain
)

ExternalProject_Add(
  tools
  DEPENDS llvm-toolchain
  SOURCE_DIR ${tools_SOURCE_DIR}
  CONFIGURE_COMMAND ${tools_SOURCE_DIR}/configure --prefix=${CMAKE_CURRENT_BINARY_DIR}/gba-tools
  PREFIX gba-tools
  BUILD_COMMAND make
  INSTALL_COMMAND ""
  BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/gba-tools/src/tools-build/gbafix
)

ExternalProject_Add_Step(
  tools prepare_configure
  WORKING_DIRECTORY ${tools_SOURCE_DIR}
  COMMAND aclocal
  COMMAND autoconf
  COMMAND automake --add-missing
  DEPENDERS configure
)

foreach(cmd gbafix gbalzss gbfs insgbfs lsgbfs ungbfs)
  ExternalProject_Add_Step(
    tools ${cmd}
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/gba-tools/src/tools-build/${cmd} ${LLVM_BINARY_DIR}/bin
    DEPENDEES build
  )
endforeach()

ExternalProject_Add(
  grit
  DEPENDS llvm-toolchain
  SOURCE_DIR ${grit_SOURCE_DIR}
  # On MacOS, the system Clang, which 'g++' is an alias of, defaults to '98
  CONFIGURE_COMMAND ${CMAKE_COMMAND} -E env ${GRIT_LDFLAGS} ${GRIT_CPPFLAGS} ${GRIT_COMPILER} ${grit_SOURCE_DIR}/configure --prefix=${CMAKE_CURRENT_BINARY_DIR}/grit
  PREFIX grit
  BUILD_COMMAND ${CMAKE_COMMAND} -E env ${GRIT_LDFLAGS} ${GRIT_CPPFLAGS} make
  INSTALL_COMMAND ""
)

ExternalProject_Add_Step(
  grit prepare_configure
  WORKING_DIRECTORY ${grit_SOURCE_DIR}
  COMMAND ${LIBTOOLIZE}
  COMMAND aclocal
  COMMAND autoconf
  COMMAND automake --add-missing
  DEPENDERS configure
)

ExternalProject_Add_Step(
  grit put_files
  COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/grit/src/grit-build/grit ${LLVM_BINARY_DIR}/bin
  DEPENDEES build
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
  COMMAND ${CMAKE_COMMAND} -E copy ${common}/gba_cart.ld ${LLVM_BINARY_DIR}/lib/clang-runtimes/arm-none-eabi/armv4t/lib
  DEPENDERS configure
)

install(
  FILES
  ${common}/${config}
  DESTINATION bin
  COMPONENT gba-toolchain
)

install(
   FILES
   ${common}/gba_cart.ld
   DESTINATION lib/clang-runtimes/arm-none-eabi/armv4t/lib
   COMPONENT llvm-toolchain
)

install(
  DIRECTORY
  ${CMAKE_CURRENT_BINARY_DIR}/grit/src/grit-build/
  DESTINATION bin
  COMPONENT gba-toolchain
  FILES_MATCHING PATTERN "grit"
  PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
)

install(
  DIRECTORY
  ${CMAKE_CURRENT_BINARY_DIR}/gba-tools/src/tools-build/
  DESTINATION bin
  COMPONENT gba-toolchain
  FILES_MATCHING PATTERN "*gb*"
  PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
)

install(
  FILES
  ${CMAKE_CURRENT_BINARY_DIR}/gba_sources/src/gba_sources-build/_deps/aas-src/build/conv2aas/conv2aas
  DESTINATION bin
  COMPONENT gba-toolchain
  PERMISSIONS OWNER_EXECUTE OWNER_WRITE OWNER_READ
)

install(
  FILES ${CMAKE_SOURCE_DIR}/README.md
  DESTINATION .
  COMPONENT gba-toolchain
)

set(gba_toolchain_third_party_license_files
    ${CMAKE_CURRENT_BINARY_DIR}/gba_sources/src/gba_sources-build/_deps/aas-src/LICENSE apex-audio-system.LICENSE
    ${tools_SOURCE_DIR}/COPYING          gba-tools.COPYING
    ${grit_SOURCE_DIR}/COPYING           grit.COPYING
    ${CMAKE_CURRENT_BINARY_DIR}/gba_sources/src/gba_sources-build/_deps/libtonc-src/license.txt libtonc.license.txt
    ${common}/MPL-v2.0.txt               gba_crt0.license.txt
)
while(gba_toolchain_third_party_license_files)
    list(POP_FRONT gba_toolchain_third_party_license_files source_file destination_name)
    install(
        FILES ${source_file}
        DESTINATION third-party-licenses
        COMPONENT gba-toolchain-third-party-licenses
        RENAME ${destination_name}
    )
endwhile()

install(
    FILES ${common}/GBA-THIRD-PARTY-LICENSES.txt
    DESTINATION .
    COMPONENT gba-toolchain-third-party-licenses
)

set(examples_regex "Makefile|.*\\.(c|cpp|h|ase|png|md)")
install(
    DIRECTORY ${CMAKE_SOURCE_DIR}/examples
    DESTINATION .
    COMPONENT gba-toolchain-examples
    FILES_MATCHING REGEX "${examples_files_regex}"
)
