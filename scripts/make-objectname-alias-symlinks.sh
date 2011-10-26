#!/bin/sh

pd_app="$1"
pd_app_guts="$pd_app/Contents/Resources/extra"

for file in `find "$pd_app_guts" -name '*0x*'`; do 
    cd `dirname $file`
    echo `dirname $file`
    /sw/bin/echo -e `basename $file`
    basename $file \
        | sed 's|0x20| |g' \
        | sed 's|0x21|!|g' \
        | sed 's|0x22|"|g' \
        | sed 's|0x23|#|g' \
        | sed 's|0x24|\$|g' \
        | sed 's|0x25|%|g' \
        | sed 's|0x26|\&|g' \
        | sed 's|0x28|(|g' \
        | sed 's|0x29|)|g' \
        | sed 's|0x2a|*|g' \
        | sed 's|0x2b|+|g' \
        | sed 's|0x2c|,|g' \
        | sed 's|0x2d|-|g' \
        | sed 's|0x2e|.|g' \
        | sed 's|0x2f|/|g' \
        | sed 's|0x3a|:|g' \
        | sed 's|0x3b|;|g' \
        | sed 's|0x3c|<|g' \
        | sed 's|0x3d|=|g' \
        | sed 's|0x3e|>|g' \
        | sed 's|0x5e|^|g' \
        | sed 's/0x7c/|/g' \
        | sed 's|0x7e|~|g'
    echo ""
    cd ..
done



