cmake_minimum_required(VERSION 3.20)
project(
  common
)

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
  COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_OBJECTS:crt0> "${CMAKE_INSTALL_PREFIX}/gba_crt0.o")
