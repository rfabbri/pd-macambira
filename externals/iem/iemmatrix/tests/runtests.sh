#!/bin/sh

ls -1 */*.pd | sed 's/\.pd/;/' > runtests.txt

function run_nogui() {
 pd -lib ../iemmatrix -nogui runtests_nogui.pd > runtests.log.$$ 2>&1 
cat runtests.log.$$ | egrep "^regression-test: " | sed -e 's/^regression-test: //'
}

function run_withgui() {
 pd -lib ../iemmatrix runtests.pd > runtests.log 2>&1
}

if test "x$1" = "x-gui"; then
 run_withgui
else
 run_nogui
fi


