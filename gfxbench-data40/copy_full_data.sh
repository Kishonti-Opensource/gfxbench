#!/bin/bash
set -e

: ${WORKSPACE:=$PWD}
export WORKSPACE

# Which product to build. Different product sources might be in the same
# repository. E.g: gfxbench_gl, gfxbench_dx, gfxbench_metal
: ${PRODUCT_ID?"not set"}
export PRODUCT_ID

# Version string as visible on the ui
: ${PRODUCT_VERSION?"not set"}
export PRODUCT_VERSION


# Print environment variables
echo
echo "--------------- BUILD PARAMETERS ---------------"
echo "product id       : ${PRODUCT_ID}"
echo "product version  : ${PRODUCT_VERSION}"
echo "workspace        : ${WORKSPACE}"
echo "------------------------------------------------"
echo


cp -r git/glb2/resources/data/gfx gfxbench/data
cp -r git/gfxbench-data/data/gfx gfxbench/data

if [ "$PRODUCT_ID" = "gfxbench_gl" ] ; then
    if [ "$PRODUCT_VERSION" = "4.0.0" ] ; then
       cp -rf /y/Products/BenchmarkData/4/latest_release_data/data/gfx gfxbench/data

       if [ "${COMMUNITY_BUILD}" = "true" ] ; then
            rm -rf gfxbench/data/gfx/car_chase/images_ETC2
       fi
    fi

    rm -rf gfxbench/data/gfx/shaders_dx
    rm -rf gfxbench/data/gfx/shaders_dx_bin
    rm -rf gfxbench/data/gfx/shaders_mtl
fi

if [ "$PRODUCT_ID" = "gfxbench_metal" ]
then
    rm -rf gfxbench/data/gfx/shaders_dx
    rm -rf gfxbench/data/gfx/shaders_dx_bin
    rm -rf gfxbench/data/gfx/shaders
fi

if [ "$PRODUCT_ID" = "gfxbench_dx" ]
then
    rm -rf gfxbench/data/gfx/shaders_mtl
    rm -rf gfxbench/data/gfx/shaders
fi

rm -rf gfxbench/data/gfx/egypt

rm -rf gfxbench/data/gfx/common/gt540
rm -rf gfxbench/data/gfx/common/hd5500
rm -rf gfxbench/data/gfx/common/hd7700m