#!/usr/bin/env bash
set -e

: ${PRODUCT_ID?"not set"}
: ${BENCHMARK_VERSION_MAJOR?"not set"}

echo
echo "--------------- BUILD PARAMETERS ---------------"
echo "PRODUCT_ID                : ${PRODUCT_ID}"
echo "BENCHMARK_VERSION_MAJOR   : ${BENCHMARK_VERSION_MAJOR}"
echo "------------------------------------------------"
echo

cp -r gfxbench40/resources/data/gfx content/data
cp -r gfxbench-data40/data/gfx content/data

if [ "$PRODUCT_ID" = "gfxbench" ] ; then
    # Car Chase requires ES3.1+AEP and AEP requires ASTC support
    if [[ "$BENCHMARK_VERSION_MAJOR" -ge "4" && "${COMMUNITY_BUILD}" = "true" ]] ; then
        rm -rf content/data/gfx/car_chase/images_ETC2
    fi

    rm -rf content/data/gfx/shaders_mtl
fi

if [ "$PRODUCT_ID" = "gfxbench_gl" ] ; then
    # Car Chase requires ES3.1+AEP and AEP requires ASTC support
    if [[ "$BENCHMARK_VERSION_MAJOR" -gt "4" && "${COMMUNITY_BUILD}" = "true" ]] ; then
        rm -rf content/data/gfx/car_chase/images_ETC2
    fi

    rm -rf content/data/gfx/shaders_dx
    rm -rf content/data/gfx/shaders_dx_bin
    rm -rf content/data/gfx/shaders_mtl
fi

if [ "$PRODUCT_ID" = "gfxbench_metal" ]
then
    rm -rf content/data/gfx/shaders_dx
    rm -rf content/data/gfx/shaders_dx_bin
    rm -rf content/data/gfx/shaders
fi

if [ "$PRODUCT_ID" = "gfxbench_dx" ]
then
    rm -rf content/data/gfx/shaders_mtl
    rm -rf content/data/gfx/shaders
fi

rm -rf content/data/gfx/egypt

rm -rf content/data/gfx/common/gt540
rm -rf content/data/gfx/common/hd5500
rm -rf content/data/gfx/common/hd7700m
