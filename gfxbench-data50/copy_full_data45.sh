#!/usr/bin/env bash
set -e

: ${PRODUCT_ID?"not set"}
: ${PLATFORM?"not set"}

echo
echo "--------------- BUILD PARAMETERS ---------------"
echo "PRODUCT_ID        : ${PRODUCT_ID}"
echo "PLATFORM          : ${PLATFORM}"
echo "------------------------------------------------"
echo

mkdir -p content/data/gfx/shaders
cp -r gfxbench50/shaders/scene5 content/data/gfx/shaders
cp -r gfxbench-data50/data/gfx content/data

if [ "${COMMUNITY_BUILD}" = "true" ] ; then
    if [[ "$PRODUCT_ID" = "gfxbench" || "$PRODUCT_ID" = "gfxbench_gl" ]]
    then
        if [ "${PLATFORM}" = "windows" ] ; then
            rm -rf content/data/gfx/scene5_high/export/images_ASTC
            rm -rf content/data/gfx/scene5_high/export/images_ETC2
        fi

        if [ "${PLATFORM}" = "android" ] ; then
            rm -rf content/data/gfx/scene5_high/export/images_DXT5
        fi
    fi

    if [ "$PRODUCT_ID" = "gfxbench_metal" ]
    then
        if [ "${PLATFORM}" = "macos" ] ; then
            rm -rf content/data/gfx/scene5_high/export/images_ASTC
            rm -rf content/data/gfx/scene5_high/export/images_ETC2
        fi

        if [ "${PLATFORM}" = "ios" ] ; then
            rm -rf content/data/gfx/scene5_high/export/images_DXT5
        fi
    fi
    
    rm -rf content/data/gfx/shaders/scene5/debug
    rm -rf content/data/gfx/shaders/scene5/shaders_ovr
fi
