#!/bin/bash
if ! test -f firmware/configure; then
    pushd firmware
    autoreconf --force --verbose --install
fi
autoreconf --force --verbose --install || exit
