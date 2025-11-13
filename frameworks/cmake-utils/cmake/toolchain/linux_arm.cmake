set(CMAKE_SYSTEM_NAME Linux CACHE STRING "System Name")
set(CMAKE_SYSTEM_PROCESSOR armv7l CACHE STRING "System Processor")
if (APPLE)
    # Carlson-Minot toolchain has also gcc-4.8 (compatible with Ubuntu 14.04 target)
    # Most project should compile fine on Mac if not using system dependencies like X11.
    # Dependencies can be copied from target device.
    set(TOOLCHAIN_DIR /usr/local/carlson-minot/crosscompilers)
    set(TOOLCHAIN_PREFIX arm-none-linux-gnueabi-)
    set(CMAKE_C_COMPILER "${TOOLCHAIN_DIR}/bin/${TOOLCHAIN_PREFIX}gcc" CACHE STRING "gcc")
    set(CMAKE_CXX_COMPILER "${TOOLCHAIN_DIR}/bin/${TOOLCHAIN_PREFIX}g++" CACHE STRING "g++")
else()
    # Using gcc-4.7 compiler because Arndale board has older libstdc++ runtime Ubuntu 12.04, causing error:
    # /usr/lib/arm-linux-gnueabihf/libstdc++.so.6: version `GLIBCXX_3.4.20' not found
    #set(CMAKE_C_COMPILER "/usr/bin/arm-linux-gnueabihf-gcc-4.7" CACHE STRING "gcc")
    #set(CMAKE_CXX_COMPILER "/usr/bin/arm-linux-gnueabihf-g++-4.7" CACHE STRING "g++")

    #  No version restrictions
    set(CMAKE_C_COMPILER "/usr/bin/arm-linux-gnueabihf-gcc" CACHE STRING "gcc")
    set(CMAKE_CXX_COMPILER "/usr/bin/arm-linux-gnueabihf-g++" CACHE STRING "g++")
endif()
set(CMAKE_C_COMPILER_ID "GNU")
set(CMAKE_CXX_COMPILER_ID "GNU")
set(CMAKE_AR ar CACHE STRING "ar")

# Skip the platform compiler checks for cross compiling
set (CMAKE_CXX_COMPILER_WORKS TRUE)
set (CMAKE_C_COMPILER_WORKS TRUE)

set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -fPIC -marm -march=armv7-a -fvisibility=hidden" CACHE STRING "c++ flags" )
set(CMAKE_C_FLAGS "$ENV{CFLAGS} -fPIC -marm -march=armv7-a -fvisibility=hidden" CACHE STRING "c flags" )

set(CMAKE_FIND_ROOT_PATH ${CMAKE_PREFIX_PATH} ${CMAKE_INSTALL_PREFIX} "/usr/arm-linux-gnueabihf")
