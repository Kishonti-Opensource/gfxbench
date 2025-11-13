# Make sure we're being sourced (possibly by another script). Check for bash
# since zsh sets $0 when sourcing.
if [[ -n "$BASH_VERSION" && "${BASH_SOURCE:-$0}" == "$0" ]]; then
  echo "[ERROR]: env.sh must be sourced."
  exit 1
fi

if [ $# -ne 1 ]; then
  echo "[ERROR]: argument missing."
  return 1
fi

: ${NG_INSTALL_ROOT:=$PWD/out/install}
: ${NG_BUILD_ROOT:=$PWD/out/build}
: ${NG_THIRDPARTY_ROOT:=$PWD/out/3rdparty}

export NG_INSTALL_ROOT
export NG_BUILD_ROOT
export NG_THIRDPARTY_ROOT

script_dir="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

export PATH="$script_dir":$PATH
. $script_dir/set_env $1

export PS1="[$1]\h:\W \u\$ "
