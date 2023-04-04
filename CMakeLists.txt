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
#   git -C repos clone https://github.com/devkitPro/libtonc.git
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
    elseif(tagType STREQUAL "tag" OR tagType STREQUAL "branch")
        set(shallow ON)
        # CMake docs recommend that "branch names and tags should
        # generally be specified as remote names"
        set(tag "origin/${tag}")
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
    PATCH_COMMAND git reset --quiet --hard && git clean --quiet --force -dx
    # Add the llvm subdirectory later to ensure that
    # LLVMEmbeddedToolchainForArm is the first project declared.
    # Otherwise CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT
    # can't be used.
    SOURCE_SUBDIR do_not_add_bmt_subdir_yet
)

FetchContent_MakeAvailable(bmt)

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

string(TOLOWER ${CMAKE_SYSTEM_PROCESSOR} processor_name)
string(REGEX MATCH "amd64|x64|x86" x86_match ${processor_name})
if(x86_match)
    set(processor_name "x86_64")
else()
    set(processor_name "AArch64")
endif()

set(CPACK_SYSTEM_NAME "${CMAKE_SYSTEM_NAME}-${processor_name}")

set(LLVM_TOOLCHAIN_LIBRARY_VARIANTS armv4t)

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
