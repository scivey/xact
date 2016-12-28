#!/bin/bash

. $(dirname ${BASH_SOURCE[0]})/common.sh

XACT_EXT_DIR=${XACT_ROOT_DIR}/external


function cm-build () {
    local dir_name="$1"
    if [[ "${dir_name}" == "" ]]; then
        echo "specify dir name" >&2
        return 1
    else
        pushd ${dir_name}
        if [[ -d "build" ]]; then
            echo "${dir_name}/build exists; not rebuilding" >&2
        else
            mkdir build
            pushd build
            cmake ../ && make -j8
            popd
        fi
        popd
    fi
}

function cm-clean() {
    local dir_name="$1"
    if [[ "${dir_name}" == "" ]]; then
        echo "specify dir name" >&2
        return 1
    else
        if [[ -d ${dir_name} ]]; then
            pushd ${dir_name}
            rm -rf build
            popd
        fi
    fi
}


CM_LIBS="googletest \
        benchmark"


function build-all() {
    if [[ -f ${XACT_EXT_DIR}/googletest/CMakeLists.txt ]]; then
        echo "git submodules are ok..." >&2
    else
        echo "updating submodules" >&2
        pushd ${XACT_ROOT_DIR}
        git submodule init && git submodule update
        popd
    fi
    for lib in ${CM_LIBS}; do
        cm-build ${XACT_EXT_DIR}/${lib}
    done
}

function clean-all() {
    for lib in ${CM_LIBS}; do
        cm-clean ${XACT_EXT_DIR}/${lib}
    done
}

case "$1" in
build)
    build-all
    ;;
clean)
    clean-all
    ;;
*)
    echo "usage: ${BASH_SOURCE[0]} build|clean" >& 2
    exit 1
    ;;
esac
