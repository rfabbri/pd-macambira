#!/bin/sh

dir=`pwd -P`
progdir="$dir/../../src/programs"

for f in *.tfst ; do
  b=`basename $f .tfst`
  echo "$f -> $b.gfst"
  ${progdir}/gfsmcompile "$f" -F "$b.gfst"
done

  