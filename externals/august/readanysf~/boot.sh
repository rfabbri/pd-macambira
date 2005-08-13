#!/bin/sh
rm -f config.cache
rm -f config.h
set -e
echo "Initial preparation...this can take awhile, so sit tight..."
aclocal-1.6
#aclocal
#libtoolize --automake --copy --force
autoheader
autoconf
automake-1.6 --foreign --add-missing --copy
echo "You are now ready to run ./configure ..."
