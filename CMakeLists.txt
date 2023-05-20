cmake_minimum_required(VERSION 3.14)

project(intersembly VERSION 0.0.1 LANGUAGES CXX ASM)
enable_language(ASM-ATT)
enable_language(CXX)

# set cpp standard requirements
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# set user input variables
set(TARGET_ARCH CACHE STRING "target architecture, posiable values: x86, x64, arm, aarch64")
message("TARGET_ARCH: ${TARGET_ARCH}")


set(CMAKE_CXX_FLAGS "-Wall -g")
if(TARGET_ARCH STREQUAL "x64")
    set(CMAKE_CXX_FLAGS "-m64 ${CMAKE_CXX_FLAGS}")
    add_compile_definitions("__X64__")
    set(USE_INTEL TRUE)
elseif(TARGET_ARCH STREQUAL "aarch64")
    set(CMAKE_CXX_FLAGS "-march=armv8-a ${CMAKE_CXX_FLAGS}")
    add_compile_definitions("__ARM64__")
    set(USE_ARM TRUE)
else()
    message(FATAL_ERROR "The target architecture is not supported")
endif()
set(CMAKE_C_FLAGS ${CMAKE_CXX_FLAGS})

set(CMAKE_ASM_FLAGS "-g")

include(cmake_files/include_keystone.cmake)

# project's include directories
include_directories(AFTER
    source/
    include/
    /usr/local/include/
    ${keystone_SOURCE_DIR}/include
)
add_executable(intersembly source/usermode/main.cpp)

# compile the assembler part of the project
add_library(assemble
    source/core/assemble/keystone/keystone_assembler.cpp
)

# compile the executer part of the project, for the specific architecture and link it
if(USE_INTEL)
    add_library(execute_x86
        source/core/execute/usermode/usermode_executor.cpp
        source/core/execute/usermode/x86_64/usermode_x86_64_executor.cpp
        source/core/execute/usermode/x86_64/switch_modes.S
    )
    # link the parts with the executer
    target_link_libraries(intersembly PRIVATE
        assemble
        execute_x86
        # external libraries
        keystone
    )
endif()
if(USE_ARM)
    add_library(execute_arm
        source/core/execute/usermode/usermode_executor.cpp
        source/core/execute/usermode/armv8/switch_modes.S
        source/core/execute/usermode/armv8/usermode_arm64_executor.cpp
    )
    target_link_libraries(intersembly PRIVATE
        assemble
        execute_arm
        # external libraries
        keystone
    )
endif()


# legacy use local binary instead of compiling keystone as part of the project 
# add_library(
#     keystone STATIC IMPORTED
# )
# find_library(KEYSTONE keystone)
# set_target_properties( keystone
# 	PROPERTIES IMPORTED_LOCATION ${KEYSTONE}
# )

# link_directories(
#     /usr/local/lib
# )
