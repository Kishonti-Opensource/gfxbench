#!/bin/bash

export PATH=$PATH:/Users/hydra/Qt/5.10.1/clang_64/bin/

export COMMUNITY_BUILD=true
export BUNDLE_DATA=false
export STORE_VERSION=true
export PRODUCT_ID="gfxbench_metal"
export PRODUCT_NAME="GFXBench Metal"
export PRODUCT_VERSION="5.0.0"

WORKSPACE=${PWD} PLATFORM=macosx CONFIG=Release ./scripts/build-3rdparty.sh
WORKSPACE=${PWD} PLATFORM=macosx CONFIG=Release APPLICATION_TYPE="gui" ./scripts/build.sh
