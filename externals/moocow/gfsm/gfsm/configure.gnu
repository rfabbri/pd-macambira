#!/bin/sh
exec ./configure "$@" --disable-doc --disable-programs --disable-shared --prefix="$PWD/../../extended/build.moo/noinstall"

