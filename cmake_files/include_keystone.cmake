cmake_minimum_required(VERSION 3.17)
include(FetchContent)

set(KEYSTONE_REPO https://github.com/lersi/keystone.git)
set(KEYSTONE_TAG "master")

FetchContent_Declare(
    keystone
    GIT_REPOSITORY ${KEYSTONE_REPO}
    GIT_TAG        ${KEYSTONE_TAG}
    GIT_PROGRESS   TRUE
)
FetchContent_Populate(keystone
    # PROJECT CMAKE CONFIGURATION
    # CMAKE_GENERATOR "Unix Makefiles"
    # CMAKE_ARGS
    # BUILD_COMMAND make -j 4
    # BUILD_ALWAYS TRUE
)
set(CMAKE_INSTALL_PREFIX=/usr)

set(CMAKE_BUILD_TYPE=Release)
set(BUILD_SHARED_LIBS=OFF)
set(LLVM_TARGETS_TO_BUILD="AArch64, X86")
set(BUILD_LIBS_ONLY=TRUE)
add_subdirectory(${keystone_SOURCE_DIR} ${keystone_BINARY_DIR})


