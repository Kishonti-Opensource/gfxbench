#!/usr/bin/env bash
set -e

# The directory where git repos are checked out
: ${WORKSPACE:=$PWD}
export WORKSPACE

#Disable error 4275 because vs2015 generate error
#warning C4275: non dll-interface class 'std::exception' used as base for dll-interface class 'ng::SystemError'
#export CXXFLAGS="-wd4275"


OVERRIDE_OSX_ARCHS=${OSX_ARCHS}
OSX_ARCHS=${OVERRIDE_OSX_ARCHS:="x86_64"} # x86_64,arm64


OVERRIDE_CONFIG=${CONFIG}
OVERRIDE_PLATFORM=${PLATFORM}
OVERRIDE_BUNDLE_DATA=${BUNDLE_DATA}
OVERRIDE_EDITOR=${EDITOR}
OVERRIDE_TEST=${TEST}
OVERRIDE_RENDER_API=${RENDER_API}

# Load product description
OVERRIDE_PRODUCT_ID=${PRODUCT_ID}
OVERRIDE_PRODUCT_NAME=${PRODUCT_NAME}
OVERRIDE_PRODUCT_VERSION=${PRODUCT_VERSION}
if [ -e $WORKSPACE/product ]; then
    . $WORKSPACE/product
elif [ -e $WORKSPACE/.repo/manifests/product ]; then
    . $WORKSPACE/.repo/manifests/product
else
    echo "No product description was found in workspace or manifest repo."
fi
PRODUCT_ID=${OVERRIDE_PRODUCT_ID:="${PRODUCT_ID}"}
PRODUCT_NAME=${OVERRIDE_PRODUCT_NAME:="${PRODUCT_NAME}"}
PRODUCT_VERSION=${OVERRIDE_PRODUCT_VERSION:="${PRODUCT_VERSION}"}


CONFIG=${OVERRIDE_CONFIG:="${CONFIG}"}
PLATFORM=${OVERRIDE_PLATFORM:="${PLATFORM}"}
BUNDLE_DATA=${OVERRIDE_BUNDLE_DATA:="${BUNDLE_DATA}"}
EDITOR=${OVERRIDE_EDITOR:="${EDITOR}"}
TEST=${OVERRIDE_TEST:="${TEST}"}
RENDER_API=${OVERRIDE_RENDER_API:="${RENDER_API}"}


# Which product to build. Different product sources might be in the same
# repository. E.g: gfxbench_gl, gfxbench_dx, gfxbench_metal
: ${PRODUCT_ID?"not set"}
export PRODUCT_ID

# What product name to show on the ui
: ${PRODUCT_NAME?"not set"}
export PRODUCT_NAME

# Version string as visible on the ui
: ${PRODUCT_VERSION?"not set"}
export PRODUCT_VERSION

# Target platform (android-armv7a / android-x86 / ios / iossim / vs2010 /
# vs2012 / vs2013 / v120_wp81-arm / v120_wp81-win32 / v120_rt81-arm /
# v120_rt81-x86 / v120_rt81-x64 / v110_wp80-arm / v110_wp80-win32 /
# macosx / linux)
: ${PLATFORM?"not set"}
export PLATFORM

# Build Configuration (Release / Debug / RelWithDebInfo / MinSizeRel)
: ${CONFIG?"not set. Valid values: Release|Debug|RelWithDebInfo|MinSizeRel"}
export CONFIG

# Copy benchmark data to tfw package directory (true / false)
: ${BUNDLE_DATA:="true"}
export BUNDLE_DATA

: ${FIXUP_BUNDLE:=true}
: ${ENABLE_CLANG:=false}
: ${EDITOR:=false}

: ${ENABLE_CCACHE:=false}
if [ "$ENABLE_CCACHE" = "true" ] ;then
    export PATH=$PATH:/usb/lib/ccache
    export NDK_CCACHE=$(which ccache)
fi

: ${CPPCHECK:=""}
export CPPCHECK

: ${VK_NULL:=false}
export VK_NULL

# Build with multiple threads
: ${MP_COMPILE:="false"}
export MP_COMPILE

# Set OGLX_VARIANT and OGLX_DRIVER for frameworks/oglx
: ${OGLX_VARIANT:="default"}
: ${OGLX_DRIVER:=""}

# INTERNAL USE FOR KISHONTI: Full UI application or command line test runner
# (gui / developer)
: ${APPLICATION_TYPE:="developer"}
export APPLICATION_TYPE
if [ "$APPLICATION_TYPE" != "gui" ] && [ "$APPLICATION_TYPE" != "developer" ]; then
    echo "APPLICATION_TYPE must be set 'gui' or 'developer'"
    exit 1
fi

# INTERNAL USE FOR KISHONTI: Build community application (true / false)
: ${COMMUNITY_BUILD:="false"}
export COMMUNITY_BUILD

: ${DISABLE_GFX4:="false"}
export DISABLE_GFX4

# INTERNAL USE FOR KISHONTI: Build application for upload to a store (true / false)
: ${STORE_VERSION:="false"}
export STORE_VERSION

# INTERNAL USE FOR KISHONTI: Don't delete tfw-pkg directory before build (true / false)
: ${KEEP_TFW_PACKAGE:="false"}
export KEEP_TFW_PACKAGE

# INTERNAL USE FOR KISHONTI: Jenkins build number
: ${BUILD_NUMBER:="0"}
export BUILD_NUMBER

# INTERNAL USE FOR KISHONTI: Copy build artifacts to this directory (path, can be empty)
: ${ARCHIVE_ROOT:=""}
export ARCHIVE_ROOT

# INTERNAL USE FOR KISHONTI: Render benchmark ui
: ${BENCHMARK_GUI:="false"}
export BENCHMARK_GUI

# INTERNAL USE FOR KISHONTI: Significant frame mode
: ${SIGNIFICANT_FRAME_MODE:="false"}
export SIGNIFICANT_FRAME_MODE

# INTERNAL USE FOR KISHONTI: when building android gui app for multiple archs use with KEEP_TFW_PACKAGE
# to build the native parts. For the last arch enable BUILD_APK, it will include all native archs.
: ${BUILD_APK:="true"}
export BUILD_APK

# INTERNAL USE FOR KISHONTI: Build application for upload to a store (developer / adhoc / media / store)
: ${DISTRIBUTION_TYPE:="developer"}
export DISTRIBUTION_TYPE

# Set default value for benchmark dir
: ${BENCHMARK_DIR:="${WORKSPACE}/${PRODUCT_ID}"}
export BENCHMARK_DIR

# Set default value for benchmark dir
: ${OBFUSCATE_PACKAGE_NAME:="false"}
export OBFUSCATE_PACKAGE_NAME


if [ ${#PRODUCT_ID} -eq 0 ];then
    echo "PRODUCT_ID was not set"
    exit 1
fi
if [ ${#PRODUCT_NAME} -eq 0 ];then
    export PRODUCT_NAME="${PRODUCT_ID}"
fi
if [ ${#PRODUCT_VERSION} -eq 0 ];then
    echo "PRODUCT_VERSION was not set"
    exit 1
fi


# Print environment variables
echo
echo "--------------- BUILD PARAMETERS ---------------"
echo "product id       : ${PRODUCT_ID}"
echo "product name     : ${PRODUCT_NAME}"
echo "product version  : ${PRODUCT_VERSION}"
echo "workspace        : ${WORKSPACE}"
echo "platform         : ${PLATFORM}"
echo "application type : ${APPLICATION_TYPE}"
echo "clang compiler   : ${ENABLE_CLANG}"
echo "configuration    : ${CONFIG}"
echo "bundle data      : ${BUNDLE_DATA}"
echo "multiprocess     : ${MP_COMPILE}"
echo "community build  : ${COMMUNITY_BUILD}"
echo "store version    : ${STORE_VERSION}"
echo "keep tfw package : ${KEEP_TFW_PACKAGE}"
echo "build apk        : ${BUILD_APK}"
echo "------------------------------------------------"
echo

if [ "$APPLICATION_TYPE" = "gui" ]; then
    case $PLATFORM in
        android-armv7a|android-x86|android-arm64-v8a|android-x86-64|vs2019-*-arm64|vs2022-*-arm64)
            export NO_GL=1 # testfw doesn't work with GLFW
        ;;
    esac
fi
source $WORKSPACE/frameworks/cmake-utils/scripts/env.sh $PLATFORM


# CMake usually create TFW_PACKAGE_DIR structure from scratch
case $PLATFORM in
    qnx-armv7|qnx-aarch64|qnx-x86_64)
        export TFW_PACKAGE_DIR=${WORKSPACE}/tfw-pkg/${PLATFORM}
    ;;
    *)
        export TFW_PACKAGE_DIR=${WORKSPACE}/tfw-pkg
    ;;
esac

if [ "$KEEP_TFW_PACKAGE" != "true" ] && [ -d "${TFW_PACKAGE_DIR}" ]; then
    if [ "${COMSPEC}" != "" ]; then
        # rm -rf follows junctions on windows
        $COMSPEC "/C rmdir /S /Q tfw-pkg"
    else
        rm -rf $TFW_PACKAGE_DIR
    fi
fi
if [ "${TFW_PACKAGE_DIR}" != "" ]; then
    mkdir -p ${TFW_PACKAGE_DIR}
fi

# Enable parallel compile for the Generator, if requested
if [ "$MP_COMPILE" = "true" ] ; then
    case $NG_CMAKE_GENERATOR in
    *Makefiles)
        echo "Enable parallel build for: $NG_CMAKE_GENERATOR"
        export MAKEFLAGS+=" -j16"
    ;;
    Visual\ Studio*)
        echo "Enable parallel build for: $NG_CMAKE_GENERATOR"
        export CFLAGS+=" -MP"
        export CXXFLAGS+=" -MP"
    ;;
    *)
        echo "Ignore MP_COMPILE env var for Generator: $NG_CMAKE_GENERATOR"
    esac
fi

COMMON_OPTS+=" -DOPT_FIXUP_BUNDLE=${FIXUP_BUNDLE}"
COMMON_OPTS+=" -DSIGNIFICANT_FRAME_MODE=${SIGNIFICANT_FRAME_MODE}"
COMMON_OPTS+=" -DPRODUCT_ID=${PRODUCT_ID}"
COMMON_OPTS+=" -DPRODUCT_VERSION=${PRODUCT_VERSION}"
COMMON_OPTS+=" -DAPPLICATION_TYPE=${APPLICATION_TYPE}"
COMMON_OPTS+=" -DBUNDLE_DATA=${BUNDLE_DATA}"
COMMON_OPTS+=" -DSTORE_VERSION=${STORE_VERSION}"
COMMON_OPTS+=" -DBUILD_NUMBER=${BUILD_NUMBER}"
COMMON_OPTS+=" -DTFW_PACKAGE_DIR=${TFW_PACKAGE_DIR}"
COMMON_OPTS+=" -DPLATFORM=${PLATFORM}"
COMMON_OPTS+=" -DBENCHMARK_GUI=${BENCHMARK_GUI}"
COMMON_OPTS+=" -DOPT_COMMUNITY_BUILD=${COMMUNITY_BUILD}"

for i in $DISABLED_PACKAGES; do
    COMMON_OPTS+=" -DCMAKE_DISABLE_FIND_PACKAGE_$i=TRUE"
done

ngrtl_OPTS="-DNGRTL_MODULES=core;pngio"
oglx_OPTS=" -DOGLX_VARIANT=${OGLX_VARIANT}"
if [ "$COMMUNITY_BUILD" = "false" ]; then
    ngrtl_OPTS="${ngrtl_OPTS} -DBUILD_SHARED_LIBS=0"
fi

if [ "${ARCHIVE_ROOT}" != "" ]; then
    DEFAULT_TARGET="-t package"
else
    DEFAULT_TARGET="-t install"
fi

. semver.sh
semverParseInto ${PRODUCT_VERSION} MAJOR MINOR PATCH F U
echo "$MAJOR $MINOR $PATCH"

#explicit check existing of gfxbench or gfxbench_gl folder to avoid mixing with
#any other e.g. gfxbench-data
case $PRODUCT_ID in
    *gfxbench*)
            if [ -d "${WORKSPACE}/gfxbench" ];then
                BENCHMARK_DIR=${WORKSPACE}/gfxbench
            elif [ -d "${WORKSPACE}/gfxbench_gl" ];then
                BENCHMARK_DIR=${WORKSPACE}/gfxbench_gl
            fi
            if [ -n "${CPPCHECK}" ];then
                ${CPPCHECK} --enable=all --inconclusive --xml --xml-version=2 \
                -iglew.c -i${WORKSPACE}/frameworks/ngl/src/glslang_spirv0x10000.2 -i${WORKSPACE}/frameworks/ngl/src/glslang_spirv30 -i${WORKSPACE}/frameworks/ngl/src/glslang_spirv31 -i${WORKSPACE}/frameworks/ngl/src/glslang_spirv32 -i${WORKSPACE}/frameworks/ngl/src/glslpp -i${WORKSPACE}/frameworks/ngl/src/v210.1 -i${WORKSPACE}/frameworks/ngl/src/v170.2 \
                ${WORKSPACE}/frameworks/kcl_framework/kcl ${WORKSPACE}/frameworks/ngl ${WORKSPACE}/gfxbench 2> cppcheck.xml
            fi
    ;;
esac

export PRODUCT_VERSION_MAJOR=$MAJOR
export PRODUCT_VERSION_MINOR=$MINOR
export PRODUCT_VERSION_PATCH=$PATCH


PROJECTS="frameworks/ngrtl"
if [ ! "$COMMUNITY_BUILD" = "true" ] || [ "$BUNDLE_DATA" = "true" ] ; then
    PROJECTS="${PROJECTS} gfxbench-data"
fi


case $PLATFORM in
    v120_wp81-arm|v120_wp81-win32|v140_wp81-arm|v140_wp81-win32|v120_rt81-arm|v120_rt81-x86|v120_rt81-x64)
    ;;
    qnx-armv7|qnx-aarch64|qnx-x86_64)
    ;;
    ios|macosx)
    ;;
    *)
    PROJECTS+=" frameworks/clew frameworks/cudaw"
    ;;
esac
#systeminfo have to followed by clew and cudaw
PROJECTS+=" frameworks/systeminfo"

if [ "${PRODUCT_ID}" = gfxbench ] ; then
    export TEST=scene5
    PROJECTS+=" frameworks/oglx"
fi
COMMON_OPTS+=" -DTEST_MODULE=${TEST}"

if [ "${PRODUCT_ID}" = gfxbench_gl ] || [ "${PRODUCT_ID}" = gfxbench_ngl ] || [ "${PRODUCT_ID}" = gfxbench_vulkan ] || [ "${PRODUCT_ID}" = "gfxbench_vr" ] ; then
    PROJECTS+=" frameworks/oglx"
fi

if [ "$APPLICATION_TYPE" = "gui" ]; then
    PROJECTS+=" frameworks/localization"
fi

if [ "$COMMUNITY_BUILD" = "true" ]; then
    PROJECTS+=" frameworks/netman frameworks/syncdir"
fi

if [ "${OGLX_DRIVER}" = "ES3" ] || [ "${OGLX_DRIVER}" = "ES2" ]; then
    COMMON_OPTS+=" -DOPT_USE_GLES=1"
fi

if [ "${OVR_SDK}" = true ]; then
    COMMON_OPTS+=" -DWITH_OVR_SDK=1"
else
    COMMON_OPTS+=" -DWITH_OVR_SDK=0"
fi

if [ "${OVR_BENCHMARK}" = true ]; then
    COMMON_OPTS+=" -DOVR_BENCHMARK_MODE=1"
else
    COMMON_OPTS+=" -DOVR_BENCHMARK_MODE=0"
fi

if [ "${OVR_APP}" = true ]; then
    COMMON_OPTS+=" -DBUILD_OVR_APP=1"
else
    COMMON_OPTS+=" -DBUILD_OVR_APP=0"
fi

if [ "${DISABLE_GFX4}" = true ]; then
    COMMON_OPTS+=" -DDISABLE_GFX4=1"
fi


#DISPLAY_PROTOCOL lists : UNSET, NONE, WIN32, ANDROID, WAYLAND, XCB
#UNSET used for platforms that have no VULKAN support
#NONE mean VK_KHR_DISPLAY
: ${DISPLAY_PROTOCOL:="XCB"}
export DISPLAY_PROTOCOL

if [ "$ENABLE_CLANG" == "true" ];then
    export CC=clang
    export CXX=clang++
fi


case $PLATFORM in
    android-armv7a|android-x86|android-arm64-v8a|android-x86-64)

        DISPLAY_PROTOCOL=ANDROID
        COMMON_OPTS+=" -DANDROID_STL=c++_shared -DANDROID_COMPILER_FLAGS_CXX=-std=c++14 -DANDROID_STL_FORCE_FEATURES=ON"
        COMMON_OPTS+=" -DCMAKE_FIND_ROOT_PATH_MODE_PACKAGE=CMAKE_FIND_ROOT_PATH_BOTH"
        COMMON_OPTS+=" -DCMAKE_FIND_ROOT_PATH_MODE_INCLUDE=CMAKE_FIND_ROOT_PATH_BOTH"
        COMMON_OPTS+=" -DCMAKE_FIND_ROOT_PATH_MODE_LIBRARY=CMAKE_FIND_ROOT_PATH_BOTH"

        if [ "${PRODUCT_ID}" = gfxbench ] || [ "${PRODUCT_ID}" = gfxbench_vulkan ] || [ "${PRODUCT_ID}" = gfxbench_ngl ] ; then
            RENDER_API=" -DRENDER_API=VULKAN;ES31"
         else
            RENDER_API=" -DRENDER_API=ES31"
        fi
        RENDER_API=${OVERRIDE_RENDER_API:="${RENDER_API}"}
        COMMON_OPTS+=" -DRENDER_API=${RENDER_API}"
        COMMON_OPTS+=" -DANDROID_NATIVE_API_LEVEL=24"

        PROJECTS="frameworks/platform-utils $PROJECTS frameworks/testfw"
        COMMON_OPTS+=" -DOPT_SWIG_JAVA=1 -DLIBRARY_OUTPUT_PATH_ROOT:PATH=${TFW_PACKAGE_DIR}"
        testfw_OPTS=" -DBENCHMARK_DIR=${BENCHMARK_DIR} -DOPT_COMMUNITY_BUILD=${COMMUNITY_BUILD} -DBUILD_SHARED_LIBS=1"

        if [ "$APPLICATION_TYPE" = "gui" -a "${BUILD_APK}" = "true" ]; then
            app_android_OPTS=" -DOPT_COMMUNITY_BUILD=${COMMUNITY_BUILD} -DSTORE_VERSION=${STORE_VERSION}"
            app_android_TARGET="-t install"
            PROJECTS+=" app_android"
        fi
    ;;

    emscripten)

        PROJECTS+=" gfxbench_gl app_emscripten"
        gfxbench_gl_TARGET="-t ALL_BUILD"
        app_emscripten_TARGET="-t ALL_BUILD"

        #python2 "$EMSCRIPTEN/tools/file_packager.py" configs.data --preload tfw-pkg/config --js-output=configs.js
        #python2 "$(EMSCRIPTEN)/tools/file_packager.py" $(OUTDIR)/common.data --preload data/common@data/raw --js-output=$(OUTDIR)/data_common.js
    ;;

    nacl|nacl64)
        COMMON_OPTS+=" -DBENCHMARK_DIR=${BENCHMARK_DIR} -DBUILD_SHARED_LIBS=0"
        PROJECTS+=" app_chrome "
        app_chrome_TARGET="-t ALL_BUILD"
    ;;

    ios|iossim)
        DISPLAY_PROTOCOL=UNSET
        if [ "$ARCHIVE_ROOT" != "" ]; then
            echo ""
            rm -rf ~/Library/MobileDevice/Provisioning\ Profiles/*
            ruby $WORKSPACE/app_ios/update_keys.rb
            source $WORKSPACE/export_prov_prof_names.sh
            # source $WORKSPACE/app_ios/ios_setup_bundle.sh
        fi

        COMMON_OPTS+=" -DRENDER_API=Metal -DCMAKE_OSX_DEPLOYMENT_TARGET=14"
        COMMON_OPTS+=" -DBENCHMARK_DIR=${BENCHMARK_DIR} -DBUILD_SHARED_LIBS=0"
        if [ "$APPLICATION_TYPE" = "gui" ]; then
            app_ios_OPTS=" -DOPT_COMMUNITY_BUILD=${COMMUNITY_BUILD}"
            app_ios_TARGET="-t xcarchive"
            PROJECTS+=" app_ios"
        else
            testfw_OPTS=""
            if [ -n "$EXTRA_RESOURCE_DIRS" ]; then
                testfw_OPTS+=" -DTESTFW_APP_EXTRA_RESOURCE_DIRS=${EXTRA_RESOURCE_DIRS}"
            fi
            testfw_TARGET="-t ipa"
            PROJECTS+=" frameworks/testfw"
        fi

    ;;

    vs2019-*|vs2022-*|macosx|linux|linux_arm|linux_arm64|linux_cross)
        COMMON_OPTS+=" -DCMAKE_SYSTEM_VERSION=10.0"
        case $PLATFORM in
            vs2019-*|vs2022-*)
                DISPLAY_PROTOCOL=WIN32
                ;;
        esac

        case $PLATFORM in
            linux_arm|linux_arm64|linux_cross)
                ngrtl_OPTS+=" -DBUILD_SHARED_LIBS=0"
                ;;
            linux)
                ngrtl_OPTS+=" -DBUILD_SHARED_LIBS=0"
                ;;
        esac
        case $PLATFORM in
            linux_arm64)
               COMMON_OPTS+=" -DOGLX_VARIANT=dummy"
        esac
        case $PLATFORM in
                macosx)
                    DISPLAY_PROTOCOL=UNSET
		    export CXXFLAGS="-std=c++11"
		    COMMON_OPTS+=" -DCMAKE_OSX_ARCHITECTURES=${OSX_ARCHS} -DCMAKE_DISABLE_FIND_PACKAGE_clew=YES -DCMAKE_DISABLE_FIND_PACKAGE_cudaw=YES -DCMAKE_OSX_SYSROOT=macosx -DCMAKE_OSX_DEPLOYMENT_TARGET=10.13":
        esac
        if [ "${PRODUCT_ID}" = "gfxbench_vulkan" ]; then
            RENDER_API="VULKAN;GL"
            case $PLATFORM in
                macosx)
                    RENDER_API=";GL"
                    ;;
            esac

        elif [ "${PRODUCT_ID}" = "gfxbench_dx" ]; then
            RENDER_API="D3D11"
            case $PLATFORM in
                vs2019-*|vs2022-*)
                    RENDER_API+=";D3D12"
                    ;;
            esac

        elif [ "${PRODUCT_ID}" = "gfxbench_metal" ]; then
            RENDER_API="Metal"

        elif [ "${PRODUCT_ID}" = "gfxbench" ] && [ "$PLATFORM" = "linux" ]; then
                RENDER_API="GL"

        elif [ "${PRODUCT_ID}" = "gfxbench" ] && [ "$PLATFORM" = "macosx" ]; then
                RENDER_API="Metal"

        elif [ "${PRODUCT_ID}" = "gfxbench" ]; then
                RENDER_API="VULKAN;D3D11;D3D12;GL"
                case $PLATFORM in
                    vs2019-arm64|vs2022-arm64)
                    RENDER_API="D3D11;D3D12;VULKAN"
                    ;;
                esac

        else
            if [ "${OGLX_DRIVER}" = "ES3" ] || [ "${OGLX_DRIVER}" = "ES2" ]; then
                RENDER_API="ES31" #GLES happen via emulator must not mix with OpenGL
            else
                RENDER_API="ES31;GL" #GLES happen via driver's extension
            fi
        fi

        RENDER_API=${OVERRIDE_RENDER_API:="${RENDER_API}"}
        COMMON_OPTS+=" -DRENDER_API=${RENDER_API} -DVK_NULL=${VK_NULL}"
        if [ "${EDITOR}" = "true" ];then
            COMMON_OPTS+=" -DBENCHMARK_DIR=${BENCHMARK_DIR}"
        else
            COMMON_OPTS+=" -DBENCHMARK_DIR=${BENCHMARK_DIR}"
        fi
        COMMON_OPTS+=" -DEDITOR=${EDITOR}"

        if [ "$APPLICATION_TYPE" = "gui" ]; then
            COMMON_OPTS+=" -DBUILD_SHARED_LIBS=1"
            app_qt_OPTS=" -DOPT_CONFIG=${CONFIG} -DOPT_COMMUNITY_BUILD=${COMMUNITY_BUILD}"
            app_qt_TARGET="${DEFAULT_TARGET}"
            PROJECTS+=" app_qt"
        else
            COMMON_OPTS+=" -DBUILD_SHARED_LIBS=0"
            testfw_OPTS=" -DBENCHMARK_DIR=${BENCHMARK_DIR} -DSILENCE_PLUGIN_LOAD_WARNINGS=1"
            testfw_TARGET="${DEFAULT_TARGET}"
            PROJECTS+=" frameworks/testfw"
        fi
    ;;

    v120_wp81-arm|v120_wp81-win32|v140_wp81-arm|v140_wp81-win32|v120_rt81-arm|v120_rt81-x86|v120_rt81-x64)

        COMMON_OPTS+=" -DOPT_SWIG_CSHARP=1 -DOPT_WINSTORE=1"
        #RENDER_API="D3D11;D3D12"
        RENDER_API="D3D11"
        COMMON_OPTS+=" -DRENDER_API=${RENDER_API}"

        if [ "$APPLICATION_TYPE" = "gui" ]; then
            app_winstore_OPTS="-DBENCHMARK_DIR=$WORKSPACE/$PRODUCT_ID -DOPT_COMMUNITY_BUILD=${COMMUNITY_BUILD} -DBUILD_SHARED_LIBS=1"
            app_winstore_TARGET="${DEFAULT_TARGET}"
            PROJECTS+=" app_winstore"
        else
            testfw_OPTS="-DBENCHMARK_DIR=$WORKSPACE/$PRODUCT_ID -DOPT_COMMUNITY_BUILD=${COMMUNITY_BUILD} -DBUILD_SHARED_LIBS=1"
            testfw_TARGET="${DEFAULT_TARGET}"
            PROJECTS+=" frameworks/testfw"
        fi

        # cmake sets the platform of external projects included with include_external_msproject from the PLATFORM variable
        case $PLATFORM in
            v120_wp81-arm|v140_wp81-arm|v120_rt81-arm)
                PLATFORM=arm
            ;;
            v120_wp81-win32|v140_wp81-win32|v120_rt81-x86)
                PLATFORM=x86
            ;;
            v120_rt81-x64)
                PLATFORM=x64
            ;;
        esac
    ;;
    qnx-armv7|qnx-aarch64|qnx-x86_64)
        DISPLAY_PROTOCOL=SCREEN
        PROJECTS="$PROJECTS frameworks/testfw"
        # Currently only ES3.1 is being used as Vulkan is not supported
        COMMON_OPTS+=" -DLIBRARY_OUTPUT_PATH_ROOT:PATH=${TFW_PACKAGE_DIR} -DRENDER_API=ES31;VULKAN"
        testfw_OPTS=" -DBENCHMARK_DIR=${BENCHMARK_DIR} -DOPT_COMMUNITY_BUILD=${COMMUNITY_BUILD} -DBUILD_SHARED_LIBS=0"

        # QNX builds are static, so any warnings about not being able to load plugins (since they are a part of the executable)
        # should be ignored
        testfw_OPTS+=" -DSILENCE_PLUGIN_LOAD_WARNINGS=1"

        STAGING_LOCATION=`make -s --no-print-directory -f QNX_Makefile_Staging_Info print-INSTALL_ROOT_nto`
        USE_STAGING=`make -s --no-print-directory -f QNX_Makefile_Staging_Info print-USE_INSTALL_ROOT`

        # The value for true, indicating to use the root install, may have different representations
        if [[ "$USE_STAGING" == *"1" ]] || [[ "$USE_STAGING" =~ ((T|t)(R|r)(U|u)(E|e))$ ]]; then
            # Staging location is in form of INSTALL_ROOT_nto = <location>. Split this string
            # by the '=' followed by any spaces following it
            PARSED_STAGING_LOCATION=`sed 's/=\s*/\n/g' <<< $STAGING_LOCATION`
            readarray -t SPLIT_PARSED_TOKENS <<<"$PARSED_STAGING_LOCATION"

            # The staging location will be the last parsed token
            export QNX_STAGE=${SPLIT_PARSED_TOKENS[-1]}
        fi
        # Modify cmake parallel level setting
        export CMAKE_BUILD_PARALLEL_LEVEL="12"
        # Enable debug messages
        COMMON_OPTS+=" -DCMAKE_VERBOSE_MAKEFILE=1 "
    ;;
    *)
        echo "Unknown/unsupported platform: $PLATFORM"
        exit 1
    ;;
esac

COMMON_OPTS+=" -DDISPLAY_PROTOCOL=${DISPLAY_PROTOCOL}"

if [ ! -z "$CMAKE_MAKE_PROGRAM" ]
then
    COMMON_OPTS+=" -DCMAKE_MAKE_PROGRAM=${CMAKE_MAKE_PROGRAM}"
fi

function print_project_info {
    echo
    echo "######################################################"
    echo "# PROJECT:  $1"
    echo "# PLATFORM: $PLATFORM"
    echo "# CONFIG:   $CONFIG"
    echo "# OPTIONS:  ${!2} ${COMMON_OPTS} -DPRODUCT_NAME=${PRODUCT_NAME}"
    echo "# TARGET :  ${!3}"
    echo "######################################################"
    echo
}


if [ "$SAVE" = "true" ] || [ "$SAVE" = "1" ] ;then

cat<< EOF > product
PRODUCT_ID="$PRODUCT_ID"
PRODUCT_NAME="$PRODUCT_NAME"
PRODUCT_VERSION="$PRODUCT_VERSION"
PLATFORM="$PLATFORM"
CONFIG="$CONFIG"
BUNDLE_DATA="$BUNDLE_DATA"
EDITOR="$EDITOR"
TEST="$TEST"
RENDER_API="$RENDER_API"
EOF

fi

for path in $PROJECTS
do
    NAME=${path##*/}            # get last component after '/'
    OPTS=${NAME//-/_}_OPTS      # replace '-' with underscore and append '_OPTS'
    TARGET=${NAME//-/_}_TARGET
    print_project_info $NAME $OPTS $TARGET
    if [[ "$SKIP_PROJECT" == *$NAME* ]] || [[ "$SKIP" == *$NAME* ]] ;then
        echo "SKIPPING..."
    else
        # PRODUCT_NAME passed separately for correct escaping

        case $PLATFORM in
            ios|iossim)
                run_cmake -c $CONFIG $WORKSPACE/$path ${!TARGET} -- ${COMMON_OPTS} ${!OPTS} "-DPRODUCT_NAME=${PRODUCT_NAME}"
            ;;
            *)
                run_cmake -c $CONFIG $WORKSPACE/$path ${!TARGET} -- ${COMMON_OPTS} ${!OPTS} "-DPRODUCT_NAME=${PRODUCT_NAME}"
            ;;
        esac
    fi
done
