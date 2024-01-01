#!/bin/bash
if ! test -f firmware/configure; then
    pushd firmware
    autoreconf --force --verbose --install
    popd
fi
autoreconf --force --verbose --install || exit
