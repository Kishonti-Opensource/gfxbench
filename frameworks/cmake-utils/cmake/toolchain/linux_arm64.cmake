set(CMAKE_SYSTEM_NAME Linux CACHE STRING "System Name")
set(CMAKE_SYSTEM_PROCESSOR aarch64 CACHE STRING "System Processor")
if (APPLE)
    # Carlson-Minot toolchain has also gcc-4.8 (compatible with Ubuntu 14.04 target)
    # Most project should compile fine on Mac if not using system dependencies like X11.
    # Dependencies can be copied from target device.
    set(TOOLCHAIN_DIR /usr/local/carlson-minot/crosscompilers)
    set(TOOLCHAIN_PREFIX arm-none-linux-gnueabi-)
    set(CMAKE_C_COMPILER "${TOOLCHAIN_DIR}/bin/${TOOLCHAIN_PREFIX}gcc" CACHE STRING "gcc")
    set(CMAKE_CXX_COMPILER "${TOOLCHAIN_DIR}/bin/${TOOLCHAIN_PREFIX}g++" CACHE STRING "g++")
else()
    # No version restrictions
    set(CMAKE_C_COMPILER "/usr/bin/aarch64-linux-gnu-gcc")
    set(CMAKE_CXX_COMPILER "/usr/bin/aarch64-linux-gnu-g++")
endif()
set(CMAKE_C_COMPILER_ID "GNU")
set(CMAKE_CXX_COMPILER_ID "GNU")
set(CMAKE_AR ar CACHE STRING "ar")

# Skip the platform compiler checks for cross compiling
set (CMAKE_CXX_COMPILER_WORKS TRUE)
set (CMAKE_C_COMPILER_WORKS TRUE)

set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -fPIC -fvisibility=hidden" CACHE STRING "c++ flags" )
set(CMAKE_C_FLAGS "$ENV{CFLAGS} -fPIC -fvisibility=hidden" CACHE STRING "c flags" )

set(CMAKE_FIND_ROOT_PATH ${CMAKE_PREFIX_PATH} ${CMAKE_INSTALL_PREFIX} "/usr/aarch64-linux-gnu")
