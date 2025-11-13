string(TOUPPER ${CMAKE_CXX_COMPILER_ID} CXX_COMPILER_ID_UPPER)

if ( (CXX_COMPILER_ID_UPPER STREQUAL CLANG) OR APPLE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -pedantic-errors -Wno-unused-parameter -Wno-gnu-zero-variadic-macro-arguments -fno-strict-aliasing -Wno-strict-aliasing -Wno-deprecated-register -Wno-gnu-anonymous-struct -Wno-nested-anon-types -Wno-invalid-offsetof -Wno-error=deprecated-declarations")

    ## suppress deprecation error to warning with macOS 10.12 sdk (Xcode 8.0)
    if (APPLE)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-error=deprecated-declarations")
    elseif (UNIX)
        set(CMAKE_EXE_LINKER_FLAGS_RELEASE "${CMAKE_EXE_LINKER_FLAGS_RELEASE} -s")
        set(CMAKE_EXE_LINKER_FLAGS_MINSIZEREL "${CMAKE_EXE_LINKER_FLAGS_MINSIZEREL} -s")
        set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "${CMAKE_SHARED_LINKER_FLAGS_RELEASE} -s")
        set(CMAKE_SHARED_LINKER_FLAGS_MINSIZEREL "${CMAKE_SHARED_LINKER_FLAGS_MINSIZEREL} -s")
    endif()
elseif (UNIX)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -s")
    set(CMAKE_CXX_FLAGS_MINSIZEREL "${CMAKE_CXX_FLAGS_MINSIZEREL} -s")

    if(QNX)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Wno-unused-parameter -fno-strict-aliasing -Wno-strict-aliasing -Wno-invalid-offsetof")
    else()
        # no-strict ---- SWIG
        # no-deprecated-register ---- POCO
        if(CMAKE_CXX_COMPILER_VERSION GREATER "7.5")
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-class-memaccess -Wno-deprecated-declarations -Wno-c++20-compat -Wno-format-overflow")
        endif()
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-array-bounds -Wno-maybe-uninitialized -std=c++11 -Wall -Werror -Wno-unused-parameter -fno-strict-aliasing -Wno-strict-aliasing -Wno-invalid-offsetof")
    endif()

elseif (${CXX_COMPILER_ID_UPPER} STREQUAL MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W3 /WX /wd4275 /EHsc /wd4834 /wd4996")
endif()

#
# warning C4275: non dll-interface class'std::exception' used as base for dll-interface class
#
