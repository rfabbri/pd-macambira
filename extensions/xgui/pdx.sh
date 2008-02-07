#!/bin/sh
./bin/xgui.sh &
../bin/pd -open ./main/pdx_connect.pd \
 -path ./main/ \
 -path ./adapters_in \
 -path ./adapters_out \
 -path ./behaviors \
 -path ./filters \
 -path ./objects \
 -path ./utils \
 -path ./physics \
 -path ./bin

 
