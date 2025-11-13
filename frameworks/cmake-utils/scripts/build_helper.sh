# must be set before sourcing the script
: ${WORKSPACE?" not set"}
: ${PLATFORM?" not set"}
: ${PROJECTS?" not set"}
: ${CONFIG:=RelWithDebInfo}

my_script_dir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
source ${my_script_dir}/env.sh $PLATFORM

function print_header {
echo
echo
echo "######################################################"
echo "# PROJECT:  $1"
echo "# PLATFORM: $PLATFORM"
echo "# CONFIG:   $CONFIG"
echo "# COMMAND:  run_cmake -c $CONFIG $WORKSPACE/$path ${!TARGET} -- ${!OPTS} ${COMMON_OPTS}"
echo "######################################################"
}

function build {
    for path in $PROJECTS
    do
        NAME=${path##*/}            # get last component after '/'
        OPTS=${NAME//-/_}_OPTS      # replace '-' with underscore and append '_OPTS'
        TARGET=${NAME//-/_}_TARGET
        print_header $NAME
        run_cmake -c $CONFIG $WORKSPACE/$path ${!TARGET} -- ${!OPTS} ${COMMON_OPTS}
    done
}
