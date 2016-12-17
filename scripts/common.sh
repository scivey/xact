#!/bin/bash

pushd () {
    command pushd "$@" > /dev/null
}

popd () {
    command popd "$@" > /dev/null
}

XACT_SCRIPTS_DIR=$(dirname ${BASH_SOURCE[0]})
pushd ${XACT_SCRIPTS_DIR}/..
export XACT_ROOT_DIR=$(pwd)
popd

