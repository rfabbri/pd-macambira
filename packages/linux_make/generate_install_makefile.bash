#!/bin/bash

ROOT_DIR=`echo $1 | sed 's|/*$||'`
prefix=$2

if [ $# -ne 1 ]; then
	 echo "Usage: $0 ROOT_DIR"
	 exit;
fi

SED=`echo sed "s|${ROOT_DIR}/||"`

function print_file ()
{
     local my_file=`echo $1 | sed 's/\\\$/\$\$/g'`
     echo -e "\tinstall -p '$my_file' '\$(prefix)/$my_file'"
}

function print_dir ()
{
	 echo -e "\tinstall -d -m0755 '\$(prefix)/$1'"
}

function traverse_install_tree ()
{
	 for file in `\ls -1d $1/*`; do
		  local target=`echo $file | $SED`
		  if [ -d "$file" ]; then
				print_dir "$target"
				traverse_install_tree "$file"
		  elif [ -f "$file" ]; then
				print_file "$target"
#		  else
#				echo "MYSTERY FILE: $file"
		  fi
	 done
}

function remove_file () 
{
     local my_file=`echo $1 | sed 's/\\\$/\$\$/g'`
	 echo -e "\trm -f -- '\$(prefix)/$my_file'"
}

function remove_dir ()
{
	 echo -e "\t-rmdir '\$(prefix)/$1'"
}

function uninstall_tree ()
{
	 for file in `\ls -1d $1/*`; do
		  local target=`echo $file | $SED`
		  if [ -d "$file" ]; then
				uninstall_tree "$file"
				remove_dir "$target"
		  elif [ -f "$file" ]; then
				remove_file "$target"
#		  else
#				echo "MYSTERY FILE: $file"
		  fi
	 done
}


echo ""
echo "prefix = /usr/local"
echo ""
echo "default:"
echo -e "\t@echo 'you have to run \"make install\" to install Pd-extended'"
echo ""
echo "install:"
traverse_install_tree $ROOT_DIR

echo ""
echo ""
echo ""
echo "uninstall:"
uninstall_tree $ROOT_DIR
