#!/bin/bash

OS_TYPE=$(uname -s)
MODE=$1 # whether to install using sudo or not

if [[ $OS_TYPE = 'Darwin' ]]
then
    OS='macos'
else
    VERSION=$(grep '^VERSION_ID=' /etc/os-release | sed 's/"//g')
    VERSION=${VERSION#"VERSION_ID="}
    OS_NAME=$(grep '^NAME=' /etc/os-release | sed 's/"//g')
    OS_NAME=${OS_NAME#"NAME="}
    [[ $OS_NAME == 'Rocky Linux' ]] && VERSION=${VERSION%.*} # remove minor version for Rocky Linux
    [[ $OS_NAME == 'Alpine Linux' ]] && VERSION=${VERSION%.*.*} # remove minor and patch version for Alpine Linux
    OS=${OS_NAME,,}_${VERSION}
    OS=$(echo $OS | sed 's/[/ ]/_/g') # replace spaces and slashes with underscores
fi
echo $OS

source ${OS}.sh $MODE
# Install Rust here since it's needed on all platforms and
# the installer doesn't rely on any platform-specific tools (e.g. the package manager)
source install_rust.sh

git config --global --add safe.directory '*'
