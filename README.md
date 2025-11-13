# GFXBench 5 Source Code for GL, DX11, DX12, Vulkan, Metal Graphics APIs

For build steps, consult the documentation in the `doc` folder.

## Licensing

- Code: BSD-3-Clause
- Assets: CC BY 4.0

## Third-Party Licenses

This project incorporates third-party components under various open source licenses. The main project is licensed under the BSD 3-Clause License (c) 2005–2025 Kishonti Ltd. Full license texts for third-party components are available in the referenced files.

Component (Path) – License:
- 3rdparty/AgilitySDK/doc/LICENSE.txt – MIT License
- 3rdparty/glew/LICENSE.txt – Modified BSD License
- 3rdparty/glfw/LICENSE.md – zlib/libpng License
- 3rdparty/libepoxy/COPYING – MIT License
- 3rdparty/libpng/LICENSE – libpng License
- 3rdparty/poco/LICENSE – Boost Software License 1.0
- 3rdparty/zlib/zlib.h – zlib License
- frameworks/kcl_framework/kcl_libraries/libIJG – Independent JPEG Group License
- frameworks/kcl_framework/kcl_libraries/libogg_theora_vorbis – BSD-style Licenses
- frameworks/kcl_framework/kcl_libraries/tinythread – zlib License
- frameworks/kcl_framework/kcl_libraries/tinyxml – zlib License
- frameworks/kcl_framework/kcl_libraries/tinyxml2 – zlib License
- frameworks/kcl_framework/kcl/src/forsyth.cpp – zlib License
- frameworks/kcl_framework/kcl/src/hdr.cpp – Igor Kravtchenko
- frameworks/kcl_framework/kcl/src/hdr.h – Igor Kravtchenko
- frameworks/kcl_framework/kcl/src/jsonserializer.h – MIT License
- frameworks/systeminfo/windows/nvapi – MIT License
- frameworks/systeminfo/windows/ags – MIT License
- frameworks/clew/khronos/CL – Khronos License (MIT-style)
- frameworks/ngl/src/glslang_spirv0x10000.3 – Apache License 2.0
- frameworks/ngl/src/glslpp – Intel notice (BSD-like, see file)
- frameworks/ngl/src/v1.0.3 – Khronos License
- frameworks/oglx/dummy/EGL – Khronos License
- frameworks/oglx/dummy/GLES2 – Khronos License
- frameworks/oglx/dummy/GLES3 – Khronos License
- frameworks/oglx/dummy/KHR – Khronos License
- frameworks/platform-utils/ScopedLocalRef.h – Apache License 2.0
- frameworks/testfw/android/aosp/4.4.4_r1/include/frameworks/native/include/android – Apache License 2.0
- frameworks/ngl/src/vulkan_wrapper.cpp – Apache License 2.0
- frameworks/ngl/src/vulkan_wrapper.h – Apache License 2.0
- frameworks/ngl/src/wglew.h – Modified BSD License
- frameworks/ngl/src/glew.h – Modified BSD License

## Quick Build Instructions

For example build configurations, see the GitHub Actions workflow: `.github/workflows/main.yml`.

### Windows

* Cygwin is not supported.
* Use Git Bash (https://git-scm.com/install/).

Windows ARM64 compilation has only been tested via cross-compilation. First build the patched Qt base available at: https://github.com/Kishonti-Opensource/qtbase515 (you must compile it yourself).

### iOS

Generate the Xcode project with the following commands:

```bash
# Ensure CMake binaries are on PATH (example below for the macOS app bundle install)
export PATH="$PATH:/Applications/CMake.app/Contents/bin"

PLATFORM=ios CONFIG=Release ./scripts/build-3rdparty.sh
PRODUCT_VERSION=5.0 BUNDLE_DATA=false PLATFORM=ios CONFIG=Release APPLICATION_TYPE="gui" ./scripts/build.sh
```

After generation:
1. Open the project at `out/build/ios/app_ios/app_ios.xcodeproj`.
2. Configure code signing (team, provisioning profile, certificates).
3. Select the `Release` configuration.
4. Select a physical `Device` target. (Simulator builds are experimental.)
