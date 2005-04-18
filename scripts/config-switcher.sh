#!/bin/sh

# location of plist that Pd reads
PLIST_ROOT=~/Library/Preferences/org.puredata.pd
PLIST=$PLIST_ROOT.plist

# which config to use (first argument)
CONFIG=$1

cp -f "$PLIST_ROOT.$CONFIG.plist" "$PLIST"
