cmake_minimum_required(VERSION 3.20)
project(
  common
)

include(FetchContent)

FetchContent_Declare(libtonc
    GIT_REPOSITORY https://github.com/stuij/libtonc
    GIT_TAG "${libtonc_TAG}"
    GIT_SHALLOW "${libtonc_SHALLOW}"
    GIT_PROGRESS TRUE
    # PATCH_COMMAND git reset --quiet --hard && git clean --quiet --force -dx
    SOURCE_SUBDIR do_not_add_libtonc_subdir_yet
)

FetchContent_MakeAvailable(libtonc)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX
        "${CMAKE_BINARY_DIR}/install"
        CACHE PATH "" FORCE
    )
endif()

enable_language(ASM)
add_library(crt0 OBJECT gba_crt0.s)

set(ENV GBA_LLVM ${LLVM_BINARY_DIR})

add_custom_target(libtonc ALL
  COMMAND make
  COMMAND make install DESTDIR=${CMAKE_INSTALL_PREFIX}
  WORKING_DIRECTORY ${libtonc_SOURCE_DIR}
)

# It's surprisingly difficult to change the name of an object
# After much too much research, this was the most elegant solution:
add_custom_target(copycrt0 ALL
  DEPENDS crt0
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_OBJECTS:crt0> "${CMAKE_INSTALL_PREFIX}/gba_crt0.o")

install(FILES)
