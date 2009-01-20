#!/bin/sh

if test -n "$*" ; then
  dirs=("$@")
elif test "`basename \"$PWD\"`" = "moocow" ; then
  dirs=(deque flite gfsm pdstring readdir sprinkler weightmap)
elif test "`basename \"$PWD\"`" = "extended" ; then
  dirs=(../deque ../flite ../gfsm ../pdstring ../readdir ../sprinkler ../weightmap)
else
  dirs=(.)
fi
echo "$0: dirs=(${dirs[@]})"

exec autoreconf --install --force --verbose "${dirs[@]}"
#--symlink
