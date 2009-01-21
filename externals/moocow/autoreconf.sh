#!/bin/sh

subdirs=(deque flite gfsm pdstring readdir sprinkler weightmap)

if test -n "$*" ; then
  dirs=("$@")
elif test "`basename \"$PWD\"`" = "moocow" ; then
  for d in "${subdirs[@]}"; do
    $0 "$d"
  done
elif test "`basename \"$PWD\"`" = "extended" ; then
  for d in "${subdirs[@]}"; do
    $0 "../$d"
  done
else
  dirs=(.)
fi

if test -n "$dirs"; then
  echo "$0: dirs=(${dirs[@]})"
  exec autoreconf --install --force --verbose "${dirs[@]}"
  #--symlink
fi
