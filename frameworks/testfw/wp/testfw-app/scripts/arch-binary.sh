#!/bin/bash
set -e
set -x

: ${PRODUCT_VERSION?" not set"}
: ${PRODUCT_ID?" not set"}
: ${PLATFORM?" not set"}
: ${COMMUNITY_BUILD?" not set"}
: ${WORKSPACE?" not set"}

VERSION="$PRODUCT_VERSION.0"
CLEANUP="true"
PLATFORM=$NG_TARGET_PLATFORM

case $PLATFORM in
	v120_wp81-arm)
		FOLDER=$WORKSPACE/out/build/v120_wp81-arm/testfw/WindowsPhoneApp/AppPackages/WindowsPhoneApp_"$VERSION"_arm_Test
		BINARY="*"
		;;
	v120_wp81-win32)
		FOLDER=$WORKSPACE/$PLATFORM/GFXBench.WindowsPhone_"$VERSION"_Bundle
		BINARY=WindowsPhoneApp_"$VERSION"_x86.appx
		;;
    vs2013)
        FOLDER=$WORKSPACE/out/build/$PLATFORM/apps/Release/
		BINARY="*"
        CLEANUP="false"
        ;;
esac
echo "platform      : $PLATFORM"
echo "folder        : $FOLDER"
echo "binary        : $BINARY"
echo "archive root  : $ARCHIVE_ROOT"

#cp $WORKSPACE/testfw_app/scripts/windows_phone/* $FOLDER

cd $FOLDER
zip "$ARCHIVE_ROOT/${PRODUCT_ID}-bin-${PLATFORM}-${VERSION}.zip" -r "$BINARY"
cd --

if [ $CLEANUP == true ]; then
    rm -R $FOLDER
fi

echo ":)"