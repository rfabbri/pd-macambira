#!/bin/sh

# this script updates all code from subversion in the standard developer's layout.
# <hans@at.or.at>

# Usage: just run it and it should find things if you have your stuff layed
# out in the standard dev layout, or used checkout-developer-layout.sh to
# checkout your pd source tree

cvs_root_dir=$(echo $0 | sed 's|\(.*\)/.*$|\1|')/..

SVNOPTIONS="--ignore-externals"

cd $cvs_root_dir
echo "Running svn update:"
svn update ${SVNOPTIONS}
echo "Running svn update for svn-externals individually:"
for section in externals/*; do
	 echo "Section: $section"
	 cd $section
         svn update ${SVNOPTIONS}
	 cd ../..
done

echo "Running svn update for other sections:"
for section in abstractions doc externals packages pd scripts; do
	 echo "Section: $section"
	 cd $section
         svn update ${SVNOPTIONS}
	 cd ..
done

test -e $dir/pd/.git && \
    cd $dir/pd && git pull origin master
