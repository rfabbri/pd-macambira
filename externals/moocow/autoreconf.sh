#!/bin/sh

subdirs=(deque flite gfsm locale pdstring readdir sprinkler weightmap)

#ar_args="--install --verbose --force --symlink"
ar_args="--install --verbose --force"
#ar_args="--install --verbose"

if test -n "$*" ; then
  dirs=("$@")
else
  case "$PWD" in
    *[/-]moocow)
      for d in "${subdirs[@]}"; do
        $0 "$d"
      done
      ;;
    */extended)
      for d in "${subdirs[@]}"; do
        $0 "../$d"
      done
      ;;
    *)
      dirs=(.)
      ;;
  esac
fi

if test -n "$dirs"; then
  echo "$0: dirs=(${dirs[@]})"
  exec autoreconf $ar_args "${dirs[@]}"
  #--symlink
fi
