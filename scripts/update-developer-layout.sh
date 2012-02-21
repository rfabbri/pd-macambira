#!/bin/sh

# this script updates all code from subversion in the standard developer's layout.
# <hans@at.or.at>

# Usage: just run it and it should find things if you have your stuff layed
# out in the standard dev layout, or used checkout-developer-layout.sh to
# checkout your pd source tree

cd "$(echo $0 | sed 's|\(.*\)/.*$|\1|')/.."
svn_root_dir=`pwd`

SVNOPTIONS="--ignore-externals"

cd $svn_root_dir
echo "Running svn update in $svn_root_dir:"
svn update ${SVNOPTIONS}
echo "Running svn update for svn-externals individually:"
for subsection in $svn_root_dir/externals/*; do
    test -d $subsection || continue
    echo "Subsection: $subsection"
    cd "$subsection"
    svn update ${SVNOPTIONS}
done

cd "$svn_root_dir"
echo "Running svn update for other sections:"
for section in abstractions doc externals packages pd scripts; do
    echo "Section: $svn_root_dir/$section"
    cd "$svn_root_dir/$section"
         svn update ${SVNOPTIONS}
         cd ..
done

if [ -e $svn_root_dir/pd/.git ]; then
	echo "cd $svn_root_dir/pd && git pull origin master"
    cd $svn_root_dir/pd && git pull origin master
else
	echo "no git found at $svn_root_dir/pd/.git"
fi
