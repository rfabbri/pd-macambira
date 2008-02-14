#!/bin/sh

# this script automatically generates a directory with all of the Pd code out
# of CVS in the standard developer's layout.  <hans@at.or.at>

# Usage: 
#	 - with no arguments, it will check out the code using anonymous CVS.
#	 - to check out using your SourceForge ID, add that as the argument

# Be aware that SourceForge's anonymous CVS server is generally 24 hours
# behind the authenticated CVS.

print_usage ()
{
    echo " "
    echo "Usage: $0 [sourceforge ID]"
    echo "   if no ID is given, it will check out anonymously"
    echo " "
    exit
}

URL="https://pure-data.svn.sourceforge.net/svnroot/pure-data/trunk/"

if [ $# -eq 0 ]; then
    echo "Checking out anonymously. Give your SourceForge ID if you don't want that."
	svn checkout $URL pure-data
elif [ "$1" == "--help" ]; then
    print_usage
elif [ "$1" == "-h" ]; then
    print_usage
elif [ $# -eq 1 ]; then
    svn checkout --username $1 $URL pure-data
else
    print_usage
fi

cd pure-data

# Gem is still separate
echo -e "\n\n The password to the Gem anonymous CVS access is blank, so just press Enter\n"
export CVSROOT=:pserver:anonymous@cvs.gem.iem.at:/cvsroot/pd-gem
cvs login
cvs checkout Gem GemLibs


# make the symlinks which simulate the files being installed into the packages
cd packages && make devsymlinks
