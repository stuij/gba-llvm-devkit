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

FetchContent_Declare(aas
    GIT_REPOSITORY https://github.com/stuij/apex-audio-system
    GIT_TAG "${aas_TAG}"
    GIT_SHALLOW "${aas_SHALLOW}"
    GIT_PROGRESS TRUE
    # PATCH_COMMAND git reset --quiet --hard && git clean --quiet --force -dx
    SOURCE_SUBDIR do_not_add_aas_subdir_yet
)

FetchContent_MakeAvailable(libtonc)
FetchContent_MakeAvailable(aas)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX
        "${CMAKE_BINARY_DIR}/install"
        CACHE PATH "" FORCE
    )
endif()

enable_language(ASM)

add_library(crt0 OBJECT gba_crt0.s)

# It's surprisingly difficult to change the name of an object
# After much too much research, this was the most elegant solution:
add_custom_target(copycrt0 ALL
  DEPENDS crt0
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_OBJECTS:crt0> "${LLVM_BINARY_DIR}/lib/clang-runtimes/arm-none-eabi/armv4t/lib/gba_crt0.o")

add_custom_target(libtonc ALL
  COMMAND ${CMAKE_COMMAND} -E env GBA_LLVM=${LLVM_BINARY_DIR} make
  COMMAND ${CMAKE_COMMAND} -E env GBA_LLVM=${LLVM_BINARY_DIR} make install DESTDIR=${LLVM_BINARY_DIR}/lib/clang-runtimes/arm-none-eabi/armv4t
  WORKING_DIRECTORY ${libtonc_SOURCE_DIR}
  DEPENDS copycrt0
)

add_custom_target(aas ALL
  COMMAND ${CMAKE_COMMAND} -E env GBA_LLVM=${LLVM_BINARY_DIR} make
  COMMAND ${CMAKE_COMMAND} -E copy build/conv2aas/conv2aas ${LLVM_BINARY_DIR}/bin
  COMMAND ${CMAKE_COMMAND} -E copy_directory build/aas/lib ${LLVM_BINARY_DIR}/lib
  COMMAND ${CMAKE_COMMAND} -E copy_directory build/aas/include ${LLVM_BINARY_DIR}/include
  WORKING_DIRECTORY ${aas_SOURCE_DIR}
  DEPENDS libtonc
)

# install(FILES)
