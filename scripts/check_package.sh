#!/usr/bin/env bash
set -e

function error_if_exists {
    #-prune -not \( -path ${WORKSPACE}/frameworks/testfw -prune \) -not \( -path ${WORKSPACE}/.repo -prune \) 
    ERROR_TARGETS=$(find "${WORKSPACE}" \
                        -not \( -path "*3rdparty/*" \) \
                        -not \( -path "${WORKSPACE}/frameworks/keys/*" \) \
                        -not \( -path "${WORKSPACE}/frameworks/testfw/*" \) \
                        -not \( -path "${WORKSPACE}/frameworks/cmake-utils/*" \) \
                        -not \( -path "${WORKSPACE}/frameworks/kcl_framework/*" \) \
                        -not \( -path "${WORKSPACE}/frameworks/ngrtl/*" \) \
                        -not \( -path "*images_*/*" \) \
                        -not \( -path "*/doc/*" \) \
                        -not \( -path "${WORKSPACE}/.repo/*" \) \
                        -name "$1" -depth)
    if [[ ! "${ERROR_TARGETS}" == "" ]]; then
        FORBIDDEN_FOUND="true"
        echo "found forbidden files/directories:"
        for item in ${ERROR_TARGETS[*]}
        do
            echo "    ${item}"
        done 
    fi
}

echo ""
echo "+-----------------------------------+"
echo "| Checking cleanup.                 |"
echo "+-----------------------------------+"
echo ""

#
# Do self checking -- if forbidden files were found eit with error
#

FORBIDDEN_FOUND="false"


#
# Common checks
#

error_if_exists "GFX-DAOGenerator"

#
# Check based on edition
#

if [[ ! "${COMMUNITY_BUILD}" == "true" ]]; then
    error_if_exists "*community*"
fi

#
# Check based on benchmark
#

if [[ "${PRODUCT_ID}" == gfxbench* ]]; then
    error_if_exists "compubench*"

elif [[ "${PRODUCT_ID}" == compubench* ]]; then
    error_if_exists "gfxbench*"
    error_if_exists "*GFX*"
fi

#
# Check based on api
#

if [[ "${PRODUCT_ID}" == *_gl* ]]; then
    error_if_exists "*_cl-*"
    error_if_exists "*_rs-*"
    error_if_exists "*_cu-*"
    error_if_exists "*metal*"
    error_if_exists "*_mtl*"
    error_if_exists "*_dx*"
    error_if_exists "*d3d11*"
    error_if_exists "*adas*"

elif [[ "${PRODUCT_ID}" == *_cl* ]]; then
    error_if_exists "*_gl*"
    error_if_exists "*_rs-*"
    error_if_exists "*_cu-*"
    error_if_exists "*metal*"
    error_if_exists "*_mtl*"
    error_if_exists "*_dx*"
    error_if_exists "*d3d11*"
    error_if_exists "*adas*"

elif [[ "${PRODUCT_ID}" == *_rs* ]]; then
    error_if_exists "*_gl*"
    error_if_exists "*_cl-*"
    error_if_exists "*_cu-*"
    error_if_exists "*metal*"
    error_if_exists "*_mtl*"
    error_if_exists "*_dx*"
    error_if_exists "*d3d11*"
    error_if_exists "*adas*"

elif [[ "${PRODUCT_ID}" == *_cu* ]]; then
    error_if_exists "*_gl*"
    error_if_exists "*_cl-*"
    error_if_exists "*_rs-*"
    error_if_exists "*metal*"
    error_if_exists "*_mtl*"
    error_if_exists "*_dx*"
    error_if_exists "*d3d11*"
    error_if_exists "*adas*"

elif [[ "${PRODUCT_ID}" == *_metal* ]]; then
#    error_if_exists "*_gl*"  matches for *_glass*
    error_if_exists "*_cl-*"
    error_if_exists "*_rs-*"
    error_if_exists "*_cu-*"
    error_if_exists "*_dx*"
    error_if_exists "*d3d11*"
    error_if_exists "*adas*"

#    error_if_exists "*888*"  matches quality match reference image
    error_if_exists "*DXT1*"
    error_if_exists "*ASTC*"

    error_if_exists "*egypt*"

elif [[ "${PRODUCT_ID}" == *_dx* ]]; then
    error_if_exists "*_gl-*"
    error_if_exists "*_cl-*"
    error_if_exists "*_rs-*"
    error_if_exists "*_cu-*"
    error_if_exists "*metal*"
    error_if_exists "*_mtl*"
    error_if_exists "*adas*"

    error_if_exists "*PVRTC4*"
    error_if_exists "*ETC2*"
    error_if_exists "*ASTC*"
fi

#
# Check based on version
#

if [[ "${BENCHMARK_VERSION_MAJOR}" == 3 && "${BENCHMARK_VERSION_MINOR}" == 0 ]]; then
    error_if_exists "*egypt*"
    error_if_exists "*888*"
    error_if_exists "*ASTC*"

    error_if_exists "*2.7*"
    error_if_exists "*3.1*"
    error_if_exists "*4.0*"
    error_if_exists "*shaders.4*"
    error_if_exists "*gfx3_1*"
    error_if_exists "*gfx4*"
    error_if_exists "*virtual_dashboard*"
    error_if_exists "*bullet*"
    error_if_exists "*libIJG*"
    error_if_exists "*31*"
    error_if_exists "*40*"

elif [[ "${BENCHMARK_VERSION_MAJOR}" == 3 && "${BENCHMARK_VERSION_MINOR}" == 1 ]]; then
    error_if_exists "*egypt*"
    error_if_exists "*888*"
    error_if_exists "*ASTC*"

    error_if_exists "*2.7*"
    error_if_exists "*3.0*"
    error_if_exists "*4.0*"
    error_if_exists "*shaders.4*"
    error_if_exists "*gfx3_0*"
    error_if_exists "*gfx4*"
    error_if_exists "*virtual_dashboard*"
    error_if_exists "*30*"
    error_if_exists "*40*"

elif [[ "${BENCHMARK_VERSION_MAJOR}" == 4 && "${BENCHMARK_VERSION_MINOR}" == 0 ]]; then
    error_if_exists "*egypt*"
    error_if_exists "*888*"
    error_if_exists "*ASTC*"

    error_if_exists "*2.7*"
    error_if_exists "*3.0*"
    error_if_exists "*3.1*"
    error_if_exists "*gfx3_0*"
    error_if_exists "*gfx3_1*"
    error_if_exists "*virtual_dashboard*"
    error_if_exists "*30*"
    error_if_exists "*31*"

fi




if [[ "${FORBIDDEN_FOUND}" == "true" ]]; then
    echo ""
    echo "+-----------------------------------+"
    echo "| Cleanup has errors (see above).   |"
    echo "+-----------------------------------+"
    echo ""

else
    echo ""
    echo "+---------------------------------------------------+"
    echo "| Cleanup was successful.                           |"
    echo "|                                                   |"
    echo "| Please note that the full list of forbidden files |"
    echo "| is manually created, so human check is            |"
    echo "| recommended.                                      |"
    echo "| If check errors found please update clean-repo.sh |"
    echo "+---------------------------------------------------+"
    echo ""

fi


