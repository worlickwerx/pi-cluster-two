#!/bin/sh
echo "Running autoreconf --verbose --install"
autoreconf --force --verbose --install || exit
echo "Now run ./configure."
