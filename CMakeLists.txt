cmake_minimum_required(VERSION 3.14)

project(intersembly VERSION 0.0.1)

# set cpp standard requirements
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_CXX_FLAGS "-m64 -Wall")
set(CMAKE_C_FLAGS ${CMAKE_CXX_FLAGS})


# project's include directories
include_directories(AFTER
    source/
    include/
    /usr/local/include/
)
add_executable(intersembly source/usermode/main.cpp)
# compile the executer part of the project
set_source_files_properties(source/core/execute/usermode/x86_64/switch_modes.S PROPERTIES COMPILE_FLAGS "-x assembler-with-cpp")
add_library(execute
    source/core/execute/usermode/usermode_executor.cpp
    source/core/execute/usermode/x86_64/usermode_x86_64_executor.cpp
    source/core/execute/usermode/x86_64/switch_modes.S
)

# compile the assembler part of the project
add_library(assemble
    source/core/assemble/keystone/keystone_assembler.cpp
)

# link the parts with the executer
target_link_libraries(intersembly PRIVATE
    assemble
    execute
    # external libraries
    keystone
)


add_library(
    keystone SHARED IMPORTED
)
set_target_properties( keystone
    PROPERTIES IMPORTED_LOCATION /usr/local/lib/libkeystone.dylib
)

link_directories(
    /usr/local/lib
)
