#!/bin/sh

pd_base="$1"
pd_base_extra="$pd_base/extra"

for file in `find "$pd_base_extra" -name '*0x*'`; do 
    cd `dirname $file`
    dir=`dirname $file`
    if [ x$dir != x$olddir ]; then
 	echo $dir
    fi
    filename=`basename $file`
    linkname=`basename $file \
        | sed 's|0x20| |g' \
        | sed 's|0x21|\\!|g' \
        | sed 's|0x22|\\"|g' \
        | sed 's|0x23|\\#|g' \
        | sed 's|0x24|\\$|g' \
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
        | sed 's|0x3f|?|g' \
        | sed 's|0x40|@|g' \
        | sed 's|0x5e|^|g' \
        | sed 's/0x7c/|/g' \
        | sed 's|0x7e|~|g'`
#        | sed "s|\(.*\)|'\1'|g"`
#    echo "linkname == $linkname =="
    ln -s $filename $linkname
    olddir=$dir
    cd ..
done



