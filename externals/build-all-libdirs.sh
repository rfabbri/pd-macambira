#!/bin/sh
#
# this is run by the Jenkins build automation server

for dir in `find . -name \*-meta.pd |  sed -n 's|\(.*\)/[a-zA-Z0-9_-]*-meta\.pd|\1|p' `
do
  (test -e "$dir/Makefile" && echo "Building $dir") || continue
  make -C $dir dist
  make -C $dir distclean
  make -C $dir
  rm -rf -- $dir/destdir
  mkdir $dir/destdir
  make -C $dir DESTDIR=$dir/destdir objectsdir="" install
done
