#!/bin/bash
set -e

# The directory where git repos are checked out
: ${WORKSPACE:=$PWD}
export WORKSPACE

# Load product description
if [ -e $WORKSPACE/.repo/manifests/product ]; then
    source $WORKSPACE/.repo/manifests/product
elif [ -e $WORKSPACE/product ]; then
    source $WORKSPACE/product
else
    echo "No product description was found in workspace or manifest repo."
fi

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

. $WORKSPACE/frameworks/cmake-utils/scripts/semver.sh

# Target platform (android-armv7a / android-x86 / ios / iossim / vs2010 /
# vs2012 / vs2013 / v120_wp81-arm / v120_wp81-win32 / v120_rt81-arm /
# v120_rt81-x86 / v120_rt81-x64 / v110_wp80-arm / v110_wp80-win32 /
# macosx / linux)
: ${PLATFORM?"not set"}
export PLATFORM

# INTERNAL USE FOR KISHONTI: Build application for upload to a store (developer / adhoc / media / store)
: ${DISTRIBUTION_TYPE:="developer"}
export DISTRIBUTION_TYPE



echo
echo "--------------- DISTRIBUTION PARAMETERS ---------------"
echo "product id       : ${PRODUCT_ID}"
echo "product name     : ${PRODUCT_NAME}"
echo "version          : ${PRODUCT_VERSION}"
echo "workspace        : ${WORKSPACE}"
echo "platform         : ${PLATFORM}"
echo "application type : ${DISTRIBUTION_TYPE}"
echo "------------------------------------------------"
echo


export BASE_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

if [[ "${PRODUCT_ID}" == gfxbench* ]]; then
    export BENCHMARK_ID="gfxbench"
elif [[ "${PRODUCT_ID}" == compubench* ]]; then
    export BENCHMARK_ID="compubench"
fi

source $WORKSPACE/frameworks/cmake-utils/scripts/env.sh $PLATFORM

export submitterMail="jenkins@kishonti.net"
export submitterPass=`cat ${WORKSPACE}/frameworks/keys/ios/JenkinsDeveloperKeys/.pass`
export kotkodapass=`cat ${WORKSPACE}/frameworks/keys/ios/JenkinsDeveloperKeys/.kotkodapass`


# source ${WORKSPACE}/app_ios/ios_setup_bundle.sh
rm -rf ~/Library/MobileDevice/Provisioning\ Profiles/*
ruby ${WORKSPACE}/app_ios/update_keys.rb
source ${WORKSPACE}/export_prov_prof_names.sh

if [[ "${COMMUNITY_BUILD}" == "true" ]]; then
    source ${WORKSPACE}/scripts/distribution/ios/setup_distribution_${PRODUCT_ID}.sh
fi
cmake -DIOS_DISTRIBUTION_OUTPUT_DIR:PATH=${ARCHIVE_ROOT} \
	-DSTORE_VERSION=${STORE_VERSION} \
	-DCOMMUNITY_BUILD=${COMMUNITY_BUILD} \
	-DPRODUCT_ID=${PRODUCT_ID} \
	-DPRODUCT_NAME="${PRODUCT_NAME}" \
	-DPRODUCT_VERSION=${PRODUCT_VERSION} \
	-DARCHIVE_NAME:STRING=${ARCHIVE_NAME} \
	-P $WORKSPACE/frameworks/cmake-utils/cmake/ios_distribute.cmake
