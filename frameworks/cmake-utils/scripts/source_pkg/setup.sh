#!/bin/bash
set -e

# The directory where git repos are checked out

WORKSPACE=$PWD
export WORKSPACE

# Load product description
if [ -e $WORKSPACE/.repo/manifests/product ]; then
    source $WORKSPACE/.repo/manifests/product
elif [ -e $WORKSPACE/product ]; then
    source $WORKSPACE/product
else
    echo "No product description was found in workspace or manifest repo."
fi

# Which product to build (gfxbench / compubench)
: ${PRODUCT_ID?"not set"}
export PRODUCT_ID

# Version string as visible on the ui
: ${PRODUCT_VERSION?"not set"}
export PRODUCT_VERSION

# INTERNAL USE FOR KISHONTI: Build community application (true / false)
: ${COMMUNITY_BUILD:="false"}
export COMMUNITY_BUILD

# INTERNAL USE FOR KISHONTI: Build application for upload to a store (true / false)
: ${STORE_VERSION:="false"}
export STORE_VERSION



# Print environment variables
echo
echo "--------------- BUILD PARAMETERS ---------------"
echo "product id       : ${PRODUCT_ID}"
echo "version          : ${PRODUCT_VERSION}"
echo "workspace        : ${WORKSPACE}"
echo "community build  : ${COMMUNITY_BUILD}"
echo "store version    : ${STORE_VERSION}"
echo "------------------------------------------------"
echo



# Set additional environment variables
source $WORKSPACE/frameworks/cmake-utils/scripts/semver.sh
semverParseInto $PRODUCT_VERSION BENCHMARK_VERSION_MAJOR BENCHMARK_VERSION_MINOR BENCHMARK_VERSION_PATCH BENCHMARK_VERSION_PRERELEASE BENCHMARK_VERSION_META
if [ -n "$BENCHMARK_VERSION_META" ]; then
    echo "VERSION in product must not contain meta information"
    exit 1
fi
if [ "$COMMUNITY_BUILD" != "true" ]; then
    BENCHMARK_VERSION_META="corporate"
else
    BENCHMARK_VERSION_META="community"
fi

export BENCHMARK_VERSION_MAJOR
export BENCHMARK_VERSION_MINOR
export BENCHMARK_VERSION_PATCH
export BENCHMARK_VERSION_PRERELEASE
export BENCHMARK_VERSION_META
