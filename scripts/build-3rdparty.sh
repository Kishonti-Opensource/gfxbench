#!/bin/bash
set -e

# The directory where git repos are checked out
: ${WORKSPACE:=$PWD}
export WORKSPACE

OVERRIDE_OSX_ARCHS=${OSX_ARCHS}
OSX_ARCHS=${OVERRIDE_OSX_ARCHS:="x86_64"} # x86_64,arm64

: ${ENABLE_CCACHE:=false}
if [ "$ENABLE_CCACHE" = "true" ] ;then
	export PATH=$PATH:/usb/lib/ccache
	export NDK_CCACHE=$(which ccache)
fi

# Load product description
if [ -e $WORKSPACE/.repo/manifests/product ]; then
    source $WORKSPACE/.repo/manifests/product
elif [ -e $WORKSPACE/product ]; then
    source $WORKSPACE/product
else
    echo "No product description was found in workspace or manifest repo."
fi

# must be set before running the script
: ${PLATFORM?" not set"}
: ${CONFIG?"not set"}

# set default values
: ${ANDROID_NATIVE_API_LEVEL:="android-24"}

: ${ENABLE_CLANG:="false"}
: ${USE_WAYLAND:="false"}


# Build with multiple threads
: ${MP_COMPILE:="false"}
export MP_COMPILE

if [[ $COMMUNITY_BUILD == "true" ]]; then
    PROJECTS="3rdparty/openssl-cmake"
fi

#case $PLATFORM in
#	ios|macosx)
#	  if [ "$COMMUNITY_BUILD" = "true" ]; then
#	    PROJECTS+=" 3rdparty/libepoxy"
#	  fi
#	;;
#	*)
#	    PROJECTS+=" 3rdparty/libepoxy"
#	;;
#esac
if [ $PLATFORM != "qnx-armv7" ] && [ $PLATFORM != "qnx-aarch64" ] && [ $PLATFORM != "qnx-x86_64" ]; then
    PROJECTS+=" 3rdparty/libepoxy"
fi

PROJECTS+="
	3rdparty/zlib
	3rdparty/libpng
	3rdparty/poco
	3rdparty/AgilitySDK
"

if [[ $PRODUCT_ID != *_dx* ]]; then
	PROJECTS+=" 3rdparty/AgilitySDK"
fi

zlib_OPTS="-DBUILD_SHARED_LIBS=0"
libpng_OPTS="-DPNG_STATIC=1 -DPNG_SHARED=0"
poco_OPTS="-DPOCO_STATIC=1 -DDISABLE_MONGODB=1 -DDISABLE_PDF=1 -DDISABLE_DATA=1 -DDISABLE_ZIP=1"
glfw_OPTS="-DGLFW_BUILD_EXAMPLES=0 -DGLFW_BUILD_TESTS=0"
if [ "$USE_WAYLAND" == "true" ]; then
    glfw_OPTS+=" -DGLFW_USE_WAYLAND=1 -DGLFW_CLIENT_LIBRARY=glesv2"
fi

source $WORKSPACE/frameworks/cmake-utils/scripts/env.sh $PLATFORM

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
		OPTS+=" --jobs 16"
		OPTS+=" /maxcpucount:16"
		## disable MSBuild node reuse; may solve MSBuild child node exit error
		OPTS+=" /nodeReuse:false"
    ;;
    *)
        echo "Ignore MP_COMPILE env var for Generator: $NG_CMAKE_GENERATOR"
    esac
fi

case $PLATFORM in
	android|android-armv7a|android-x86|android-arm64-v8a|android-x86-64)
		COMMON_OPTS+=" -DCMAKE_FIND_ROOT_PATH_MODE_PACKAGE=CMAKE_FIND_ROOT_PATH_BOTH"
		COMMON_OPTS+=" -DCMAKE_FIND_ROOT_PATH_MODE_INCLUDE=CMAKE_FIND_ROOT_PATH_BOTH"
		COMMON_OPTS+=" -DCMAKE_FIND_ROOT_PATH_MODE_LIBRARY=CMAKE_FIND_ROOT_PATH_BOTH"

		COMMON_OPTS+=" -DANDROID_STL=c++_shared"
		COMMON_OPTS+=" -DANDROID_NATIVE_API_LEVEL=${ANDROID_NATIVE_API_LEVEL}"
		COMMON_OPTS+=" -DCMAKE_C_FLAGS=-Wno-everything -DCMAKE_CXX_FLAGS=-Wno-everything"
	;;
	vs2019-*|vs2022-*)
		if [[ $PRODUCT_ID != *_dx* ]]; then
			if [ -d "3rdparty/glew" -a -d "3rdparty/glfw" ] ; then
				PROJECTS+=" 3rdparty/glew 3rdparty/glfw"
				PROJECTS+=" frameworks/ngl/src/v1.0.3/loader"
			fi
		fi
	;;
	emscripten)
		PROJECTS="3rdparty/zlib 3rdparty/libpng"
		libpng_OPTS+=" -DPNG_TESTS=0"
	;;
	nacl|nacl64)
		PROJECTS="3rdparty/zlib 3rdparty/libpng"
		libpng_OPTS+=" -DPNG_TESTS=0"
	;;
	linux)
		PROJECTS+=" 3rdparty/glew 3rdparty/glfw"
		PROJECTS+=" frameworks/ngl/src/v1.0.3/loader"
	;;
	macosx)
		PROJECTS+=" 3rdparty/glew 3rdparty/glfw"
		COMMON_OPTS="-DCMAKE_C_FLAGS=-Wno-everything -DCMAKE_CXX_FLAGS=-Wno-everything -DCMAKE_OSX_ARCHITECTURES=${OSX_ARCHS} -DCMAKE_OSX_DEPLOYMENT_TARGET=10.13"
	;;
	linux_arm|linux_arm64|linux_cross)
	;;
	ios|iossim|ios.libstdc++|iossim.libstdc++)
                COMMON_OPTS="-DCMAKE_OSX_DEPLOYMENT_TARGET=14"
	#	COMMON_OPTS="-DCMAKE_C_FLAGS=-fembed-bitcode -DCMAKE_CXX_FLAGS=-fembed-bitcode -DCOMPILE_DEFINITIONS=-Wdeprecated-declarations"
	;;
	v120_wp81-arm|v120_wp81-win32|v140_wp81-arm|v140_wp81-win32|v120_rt81-arm|v120_rt81-x86|v120_rt81-x64)
		PROJECTS="
			3rdparty/zlib
			3rdparty/libpng
			3rdparty/poco
		"
		if [[ $COMMUNITY_BUILD == "true" ]]; then
			PROJECTS+=" 3rdparty/openssl-cmake"
		fi
		libpng_OPTS+=" -DPNG_TESTS=0"
	;;
	qnx-armv7|qnx-aarch64|qnx-x86_64)
		# Add QNX things
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
		echo "Platform \"$PLATFORM\" not supported"
		exit 1
	;;
esac

function print_header {
echo
echo
echo "######################################################"
echo "# PROJECT:  $1"
echo "# PLATFORM: $PLATFORM"
echo "# CONFIG:   $CONFIG"
echo "######################################################"
}

if [ ! -z "$CMAKE_MAKE_PROGRAM" ]
then
    COMMON_OPTS+=" -DCMAKE_MAKE_PROGRAM=${CMAKE_MAKE_PROGRAM}"
fi

for path in $EXTRA_PROJECTS $PROJECTS
do
	NAME=${path##*/}            # get last component after '/'
	OPTS=${NAME//-/_}_OPTS      # replace '-' with underscore and append '_OPTS'
	TARGET=${NAME//-/_}_TARGET
	print_header $NAME
	if [[ $SKIP_PROJECT == *$NAME* ]]
	then
		echo "SKIPPING..."
	else
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
