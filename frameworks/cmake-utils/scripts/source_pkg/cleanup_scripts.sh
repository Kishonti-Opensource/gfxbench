#!/bin/bash
set -e

function exact_delete {
    TARGET=$(find . -path "$1")
    if [[ ${TARGET} == "" ]]; then
        echo "WARNING - Path not found: $1"
    fi

    for item in ${TARGET[*]}
    do
       echo "delete: ${item}"
       find . -depth -path ${item} -print0 | xargs -0 /bin/rm -rf
    done
}

function exact_invert_delete {
	set +e
    fullpath=$1
    path=${fullpath%/*}
    file=${fullpath##*/}

    if [ -n "$file" ];then
      TARGET=$( find ${path} ! -path *${file}* |grep -vs ^${path}$ )
    else
      TARGET=$( find ${path} $path |grep -vs ^${path}$ )
    fi

    for item in ${TARGET[*]}
    do
       echo "delete: ${item}"
       /bin/rm -rf ${item}
    done
} 

function smart_delete {
    TARGET=$(find . -depth -name "$1")

    for item in ${TARGET[*]}
    do
       echo "delete: ${item}"
       find . -depth -path ${item} -print0 | xargs -0 /bin/rm -rf
    done
}
