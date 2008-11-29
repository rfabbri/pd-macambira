#!/bin/sh

GFSMDRAW=./gfsmdraw
DOTGV=dotgv.sh

exec $GFSMDRAW "$@" | $DOTGV
