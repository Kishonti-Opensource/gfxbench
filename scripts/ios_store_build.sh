#!/bin/bash


export COMMUNITY_BUILD=true
export BUNDLE_DATA=false

export STORE_VERSION=true
export GFXBENCH_DEVELOPMENT_TEAM='REMOVED'
export BUILD_NUMBER=5


export PRODUCT_ID="gfxbench_metal"
export PRODUCT_NAME="GFXBench Metal"
export PRODUCT_VERSION="5.0.0"

WORKSPACE=${PWD} PLATFORM=ios CONFIG=Release ./scripts/build-3rdparty.sh
WORKSPACE=${PWD} PLATFORM=ios CONFIG=Release APPLICATION_TYPE="gui" ./scripts/build.sh
