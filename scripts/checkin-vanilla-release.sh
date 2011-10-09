#!/bin/sh
#
# this script is for checking in a Pd release tarball into the pure-data SVN

cd ~/code/pure-data/trunk/pd
rm -rf bin/*
rm -rf src/autom4te.cache/
find ~/code/pure-data/trunk/pd -type f | grep -v svn | xargs rm
tar --strip-components 1 -xzf ~/Downloads/pd-0.43-1test4.src.tar.gz
rm -f configure src/configure src/makefile.dependencies
svn rm `svn st| sed -n 's|^!\(.*\)|\1|p'`
svn add `svn st| sed -n 's|^?\(.*\)|\1|p'`
