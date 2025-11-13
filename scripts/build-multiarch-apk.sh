#!/bin/bash -xe

# must be set before running the script
: ${WORKSPACE?" not set"}

# default values
: ${PLATFORMS:="android-armv7a android-x86 android-arm64-v8a android-x86-64"}
: ${COMMUNITY_BUILD:="false"}
: ${BUILD_THIRDPARTY:="true"}

if [ "$STORE_VERSION" = "true" ]; then
    echo "STORE_VERSION: COMMUNITY_BUILD=true, CONFIG=Release, BUNDLE_DATA=false"
    export COMMUNITY_BUILD=true
    export CONFIG=Release
    export BUNDLE_DATA=false
fi

export ARCHIVE_ROOT=${WORKSPACE}/archive
export APPLICATION_TYPE=gui
export BUILD_APK=false

LAST_PLATFORM=($PLATFORMS)
LAST_PLATFORM=${LAST_PLATFORM[@]:(-1)}


# Build native tests, and collect the result in tfw-pkg.
# For the last build also the apk (all archs will be included)
for PLATFORM in $PLATFORMS
do
    if [ "$LAST_PLATFORM" = "$PLATFORM" ]; then
        BUILD_APK=true
    fi
    if [ "$BUILD_THIRDPARTY" = "true" ]; then
        PLATFORM=$PLATFORM ${WORKSPACE}/scripts/build-3rdparty.sh
    fi
    PLATFORM=$PLATFORM ${WORKSPACE}/scripts/build.sh
    export BUNDLE_DATA=false # don't copy data more than once
    export KEEP_TFW_PACKAGE=true
done

if [ "$USE_APK_SIGNER" = "true" ]
then
    ARCHIVE_NAME="$(ls $PWD/archive)"
    if [ -f '../frameworks/keys/jar_signer.sh' ]
    then
        . ../frameworks/keys/jar_signer.sh
    fi
fi
