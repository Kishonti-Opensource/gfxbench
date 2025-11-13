#!/usr/bin/env bash
set -e

curr_dir="$(dirname "$0")"
source "${curr_dir}"/setup.sh

echo "+-----------------------------------+"
echo "| Started source cleanup            |"
echo "+-----------------------------------+"
echo ""

TARGETS=$(find "${WORKSPACE}" -maxdepth 3 -name "delete_me.sh" -type f)
# echo ${TARGETS}
for item in ${TARGETS[*]}
do
    # Save workspace for later use
    WORKSPACE_SAVE=${WORKSPACE}
    cd $(dirname ${item})

    # Run found script
    WORKSPACE=$(dirname ${item})/.. ${item}

    # Restore workspace
    cd ${WORKSPACE_SAVE}
    WORKSPACE=${WORKSPACE_SAVE}

    echo "delete: ${item}"
    find . -path ${item} -depth -print0 | xargs -0 /bin/rm -rf
done

echo ""
echo "+-----------------------------------+"
echo "| Source cleanup ended.             |"
echo "+-----------------------------------+"
echo ""
