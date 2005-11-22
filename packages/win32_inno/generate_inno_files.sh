#!/bin/sh

ROOT_DIR=build
#ROOT_DIR="$INSTALL_PREFIX"

function print_dir()
{
	 SED=`echo "sed 's|${ROOT_DIR}/||'"`
	 source=$1
	 dest=`echo $source | eval $SED `
	 echo "Source: ${source}/*.*; DestDir: {app}/$dest; Flags: ignoreversion" | \
		  sed 's|/|\\|g'
}

function traverse_tree() 
{
	 my_dir_root="$1"
#	 echo "ROOT: $my_dir_root"
	 FILES=`ls -1d ${my_dir_root}/* | grep -v CVS`
	 if [ "x$FILES" != "x" ]; then
		  print_dir "$dir"
		  for dir in $FILES; do
				test -d "$dir" && traverse_tree "$dir"
		  done
	 fi
}


traverse_tree "${ROOT_DIR}"

