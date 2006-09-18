#!/bin/sh

LIBS="Gem cyclone zexy creb cxc iemlib list-abs mapping markex maxlib memento mjlib motex oscx pddp pdogg pixeltango pmpd rradical sigpack smlib toxy unauthorized vbap pan freeverb hcs jmmmp ext13 deprecated flatspace"

GNULINUX_FONTPATH="/usr/X11R6/lib/X11/fonts /usr/X11R6/lib/X11/fonts/TTF /usr/lib/X11/fonts /usr/share/fonts/truetype"
MACOSX_FONTPATH="/System/Library/Fonts /Library/Fonts ~/Library/Fonts /usr/X11R6/lib/X11/fonts/TTF /System/Library/Frameworks/JavaVM.framework/Versions/1.3.1/Home/lib/fonts /sw/lib/X11/fonts/msttf /sw/lib/X11/fonts/intl/TrueType /sw/lib/X11/fonts/applettf"
WINDOWS_FONTPATH="hex(2):25,00,53,00,79,00,73,00,74,00,65,00,6d,00,52,00,6f,00,6f,00,74,00,25,00,2f,00,46,00,6f,00,6e,00,74,00,73,00,00,00"

ROOT_DIR=~/cvs/pure-data/packages

GNULINUX_FILE=${ROOT_DIR}/linux_make/.pdsettings
MACOSX_FILE=${ROOT_DIR}/darwin_app/org.puredata.pd.plist
WINDOWS_FILE=${ROOT_DIR}/win32_inno/pd-settings.reg


GNULINUX_HEADER='standardpath: 1\nverbose: 0'


MACOSX_HEADER='<?xml version="1.0" encoding="UTF-8"?>\n<!DOCTYPE plist PUBLIC "-//Apple Computer//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">\n<plist version="1.0">\n<dict>\n\t<key>defeatrt</key>\n\t<string>0</string>'
MACOSX_FOOTER='</dict>\n
</plist>\n'


WINDOWS_HEADER='Windows Registry Editor Version 5.00\n\n[HKEY_LOCAL_MACHINE\SOFTWARE\Pd]'


echo -e $GNULINUX_HEADER > $GNULINUX_FILE
echo -e $MACOSX_HEADER > $MACOSX_FILE
echo -e $WINDOWS_HEADER > $WINDOWS_FILE

# GNU/Linux -------------------------------------------------------------------#
function print_gnulinux ()
{
	 echo "loadlib$1: $2" >> $GNULINUX_FILE
}

function print_gnulinux_fontpath ()
{
	 i=0
	 for fontpath in $GNULINUX_FONTPATH; do
		  ((++i)) 
		  echo "path${i}: ${fontpath}" >> $GNULINUX_FILE
	 done
}

function print_gnulinux_footer ()
{
	 echo "nloadlib: $1" >> $GNULINUX_FILE
}

# Mac OS X --------------------------------------------------------------------#
function print_macosx () 
{
	 echo -e "\t<key>loadlib$1</key>" >> $MACOSX_FILE
	 echo -e "\t<string>$2</string>" >> $MACOSX_FILE
}

function print_macosx_fontpath ()
{
	 i=0
	 for fontpath in $MACOSX_FONTPATH; do
		  ((++i)) 
		  echo -e "\t<key>path${i}</key>" >> $MACOSX_FILE
		  echo -e "\t<string>${fontpath}</string>" >> $MACOSX_FILE
	 done
}

# Windows ---------------------------------------------------------------------#
function print_windows ()
{
	 echo "\"loadlib$1\"=\"$2\"" >> $WINDOWS_FILE
}

function print_windows_fontpath ()
{
	 j=0
	 for fontpath in $WINDOWS_FONTPATH; do
		  ((++j)) 
		  echo "\"path${j}\"=${fontpath}" >> $WINDOWS_FILE
	 done
}

function print_windows_delete ()
{
	 echo "\"${1}${2}\"=-" >> $WINDOWS_FILE
}

#==============================================================================#

i=0
for lib in $LIBS; do
	 ((++i)) 
	 echo -n "$lib "
	 print_gnulinux $i $lib
	 print_macosx $i $lib
	 print_windows $i $lib
done
echo " "

# the .pdsettings file needs a terminator with the lib count
linux_end_count=$i

# print lines to delete existing loadlib flags
echo "; delete any previous loadlib flags" >> $WINDOWS_FILE
while [ $i -lt 100 ]; do
	 ((++i)) 
	 print_windows_delete loadlib $i
done

print_windows_fontpath

# print lines to delete existing path flags
i=1
echo "; delete all existing path flags" >> $WINDOWS_FILE
while [ $i -lt 100 ]; do
	 ((++i)) 
	 print_windows_delete path $i
done

# the .pd-settings file needs an end tag for the loadlib statements
print_gnulinux_fontpath
print_gnulinux_footer $linux_end_count

print_macosx_fontpath
echo -e $MACOSX_FOOTER >> $MACOSX_FILE

