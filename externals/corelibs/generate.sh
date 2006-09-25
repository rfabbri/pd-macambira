#!/bin/sh

# delete all .c-files in order to not duplicate entries in there
## LATER: ask hcs why he is using "touch" and ">>" instead of
## an initial ">"
rm -f lib_?_*.c

# put these at the top of the file
touch lib_d_fft.c
echo '#include "../../pd/src/d_fftroutine.c"' >> lib_d_fft.c
if [ -e "../../pd/src/d_fft_mayer.c" ]
then
 echo '#include "../../pd/src/d_fft_mayer.c"' >> lib_d_fft.c
else
 echo '#include "../../pd/src/d_mayer_fft.c"' >> lib_d_fft.c
fi


for file in ../../pd/src/[dx]_*.c; do 
	 newfile=`echo $file | sed 's|.*/src/\([dx]_\)|lib_\1|'`
	 touch $newfile
	 echo -n '#include "' >> $newfile
	 echo -n $file >> $newfile
	 echo '"' >> $newfile
	 echo "void "`echo $newfile|sed 's|\(.*\)\.c|\1|'`"_setup(void)" >> $newfile 
	 echo "{" >> $newfile
	 echo $file | sed 's|.*src/\(.*\)\.c|    \1_setup();|' >> $newfile
	 echo "}" >> $newfile
done

# these files hold code for other classes, but no classes
rm -f lib_d_fftroutine.c lib_d_fft_mayer.c lib_d_mayer_fft.c lib_d_resample.c


