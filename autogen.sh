#!/bin/bash -e
for sub in firmware hardware .; do
    pushd $sub
    autoreconf --force --verbose --install
    popd
done
