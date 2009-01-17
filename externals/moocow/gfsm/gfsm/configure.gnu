#!/bin/sh
exec "`dirname $0`"/configure "$@" FLEX=no BISON=no --disable-doc --disable-programs --disable-shared --prefix="$PWD/../../extended/build.moo/noinstall"

