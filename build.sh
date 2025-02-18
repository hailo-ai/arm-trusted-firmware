#!/bin/bash

SCRIPT_PATH=$(realpath $(dirname "$0"))

if [ -f "${SCRIPT_PATH}/build.cfg" ]; then
    source "${SCRIPT_PATH}/build.cfg"
else
    source "${SCRIPT_PATH}/build.cfg.default"
fi

source ${TOOLCHAIN}

MAKE="make CROSS_COMPILE=aarch64-poky-linux- -j16"
export LDFLAGS=""

extract_plat() {
    local name=$(find ${DEPLOY_DIR}/ -type l -name 'bl31-*.bin' -printf %P)
    if [ -z "$name" ]; then
        if [ -f "${DEPLOY_DIR}/machine_name.txt" ]; then
            name=$(cat "${DEPLOY_DIR}/machine_name.txt")
        fi
    fi

    if [[ $name == *hailo15l* ]]; then
        echo "hailo15l"
    elif [[ $name == *hailo15* ]]; then
        echo "hailo15"
    elif [[ $name == *hailo10h2* ]]; then
        echo "hailo10h2"
    else
        echo "Unknown platform!"
        exit 1
    fi
}

bl31_dir() {
    if [[ $TFA_DEBUG == 1 ]]; then
        echo "${SCRIPT_PATH}/build/${TFA_PLATFORM}/debug"
    else
        echo "${SCRIPT_PATH}/build/${TFA_PLATFORM}/release"
    fi
}

if [ -z "${TFA_PLATFORM}" ]; then
    TFA_PLATFORM=$(extract_plat)
fi

# Require success of commands
set -e

# print each command executed
set -o xtrace

make_all(){
	echo $MAKE PLAT=${TFA_PLATFORM} DEBUG=${TFA_DEBUG} LOG_LEVEL=${LOG_LEVEL}
	$MAKE PLAT=${TFA_PLATFORM} DEBUG=${TFA_DEBUG} LOG_LEVEL=${LOG_LEVEL}
    cp $(bl31_dir)/bl31.bin $(bl31_dir)/bl31/bl31.elf ${DEPLOY_DIR}
}

if [ $# -eq 0 ]
	then
		make_all
	else
		$MAKE $1
fi
