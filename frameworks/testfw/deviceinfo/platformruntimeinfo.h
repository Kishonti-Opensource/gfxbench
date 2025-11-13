/*
 * Copyright (c) 2005-2025, Kishonti Ltd
 * SPDX-License-Identifier: BSD-3-Clause
 * This file is part of GFXBench. See the top-level LICENSE file for details.
 */
#include "runtimeinfo.h"

#if __linux__
    #include "linuxruntimeinfo.h"
    namespace tfw { typedef LinuxRuntimeInfo PlatformRuntimeInfo; }
#elif WIN32
    #include "windowsruntimeinfo.h"
    namespace tfw { typedef WindowsRuntimeInfo PlatformRuntimeInfo; }
#elif __APPLE__
    #include "TargetConditionals.h"
    #if TARGET_IPHONE_SIMULATOR
        #include "iosruntimeinfo.h"
        namespace tfw { typedef IosRuntimeInfo PlatformRuntimeInfo; }
    #elif TARGET_OS_IPHONE
        #include "iosruntimeinfo.h"
        namespace tfw { typedef IosRuntimeInfo PlatformRuntimeInfo; }
    #elif TARGET_OS_MAC
        #include "osxruntimeinfo.h"
        namespace tfw { typedef OsxRuntimeInfo PlatformRuntimeInfo; }
    #endif
#else
    #include "nullruntimeinfo.h"
    namespace tfw { typedef NullRuntimeInfo PlatformRuntimeInfo; }
#endif
