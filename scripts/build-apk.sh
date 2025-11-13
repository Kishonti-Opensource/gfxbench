#!/bin/bash
set -e

# must be set before running the script
: ${WORKSPACE?" not set"}
: ${TARGET?" not set"} # ALL or the name of the store

# set defult values
: ${ANDROID_NATIVE_API_LEVEL:="android-18"}
: ${CONFIG:=release}

STORE_Android="Android/REMOVED"
STORE_Amazon="Amazon/REMOVED"
STORE_HiAPK="HiAPK/REMOVED"
STORE_QQ="QQ/REMOVED"
STORE_360="360/REMOVED"
STORE_91Market="91Market/REMOVED"
STORE_Xiaomi="Xiaomi/REMOVED"
STORE_Wandoujia="Wandoujia/REMOVED"
STORE_gFan="gFan/REMOVED"
STORE_AnZhi="AnZhi/REMOVED"
STORE_Liqucn="Liqucn/REMOVED"
STORE_AppChina="AppChina/REMOVED"

TARGETS=""

case $TARGET in
    ALL)
        TARGETS+=" ${STORE_Android}"
        TARGETS+=" ${STORE_Amazon}"
        TARGETS+=" ${STORE_HiAPK}"
        TARGETS+=" ${STORE_QQ}"
        TARGETS+=" ${STORE_360}"
        TARGETS+=" ${STORE_91Market}"
        TARGETS+=" ${STORE_Xiaomi}"
        TARGETS+=" ${STORE_Wandoujia}"
        TARGETS+=" ${STORE_gFan}"
        TARGETS+=" ${STORE_AnZhi}"
        TARGETS+=" ${STORE_Liqucn}"
        TARGETS+=" ${STORE_AppChina}"
    ;;
    *)
        STORE=STORE_${TARGET}
        TARGETS+=" ${!STORE}"
    ;;
esac

function print_header {
echo
echo
echo "######################################################"
echo "# STORE:  $1"
echo "# KEY:  $2"
echo "# CONFIG:   $CONFIG"
echo "######################################################"
}

if [[ $COMMUNITY_BUILD == "true" ]]; then
    PRODUCT_TYPE="community"
else
    PRODUCT_TYPE="corporate"
fi

if [[ $CONFIG == "Release" ]]; then
    LOWER_CASE_CONFIG="release"
elif  [[ $CONFIG == "Debug" ]]; then
    LOWER_CASE_CONFIG="debug"
fi

for target in $TARGETS
do
    IFS=/ read NAME KEY <<< "${target}"
    print_header $NAME $KEY
    echo "benchmark.store.name=${NAME}" >> ${WORKSPACE}/tfw-pkg/benchmark.properties
    echo "benchmark.store.key=${KEY}" >> ${WORKSPACE}/tfw-pkg/benchmark.properties
    ant -f ${WORKSPACE}/app_android/${PRODUCT_ID}-${PRODUCT_TYPE}/build.xml ${LOWER_CASE_CONFIG} -Dbenchmark.tfw-pkg.dir=${WORKSPACE}/tfw-pkg
done
