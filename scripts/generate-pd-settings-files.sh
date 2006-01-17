#!/bin/sh

LIBS="Gem cyclone zexy cxc ext13 iemabs iemmatrix markex maxlib memento mjlib motex oscx pddp pdogg pdp pidip pixeltango pmpd rradical sigpack smlib toxy unauthorized vbap xsample"


ROOT_DIR=~/cvs/pure-data/packages

GNULINUX_FILE=${ROOT_DIR}/linux_make/pdsettings
MACOSX_FILE=${ROOT_DIR}/darwin_app/org.puredata.pd.plist
WINDOWS_FILE=${ROOT_DIR}/win32_inno/pd-settings.reg


MACOSX_HEADER='<?xml version="1.0" encoding="UTF-8"?>\n<!DOCTYPE plist PUBLIC "-//Apple Computer//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">\n<plist version="1.0">\n<dict>'
MACOSX_FOOTER='</dict>\n
</plist>\n'


WINDOWS_HEADER='Windows Registry Editor Version 5.00\n\n[HKEY_LOCAL_MACHINE\SOFTWARE\Pd]'


#echo -e $GNULINUX_HEADER > $GNULINUX_FILE
echo -e $MACOSX_HEADER > $MACOSX_FILE
echo -e $WINDOWS_HEADER > $WINDOWS_FILE

function print_macosx () 
{
	 echo -e "\t<key>loadlib$1</key>" >> $MACOSX_FILE
	 echo -e "\t<string>$2</string>" >> $MACOSX_FILE
}

function print_windows ()
{
	 echo "\"loadlib$1\"=\"$2\"" >> $WINDOWS_FILE
}


i=1
for lib in $LIBS; do
	 echo -n "$lib "
	 print_macosx $i $lib
	 print_windows $i $lib
	 ((++i)) 
done
echo " "

echo -e $MACOSX_FOOTER >> $MACOSX_FILE
