ann: Artificial Neural Networks

Authors: 
 IOhannes m zmölnig - forum::für::umläute
 Davide Morelli - www.davidemorelli.it

this software is under the GnuGPL that is provided with these files

ann_som: self-organized maps
ann_mlp: multi layer perceptrons
ann_td: time delay


FANN libraries are required for ann_mlp and ann_td, see
http://fann.sourceforge.net/


---------- Windows users (MSVC):

go to http://fann.sourceforge.net/ and download fann libs
(current is http://prdownloads.sourceforge.net/fann/fann-1.2.0.zip?download)
go to MSVC++ folder and open all.dsw
compile everything
go in the ann/src folder
edit makefile.msvc with the correct PATHs
run the visual studio command prompt and execute:
nmake makefile.msvc

---------- Linux users:

go to http://fann.sourceforge.net/ and download fann libs
compile fann libs from source
if you don't compile from source edit ann/src/makefile.linux 
and set correct PATHs

read ann/src/makefile.linux for more info and edit PATHs

---------- Irix users:

ann_mlp and ann_td have never been compiled on irix

---------- OsX users:

go to http://fann.sourceforge.net/ and download fann libs
compile fann libs from source
if you don't compile from source edit ann/src/makefile.darwin 
and set correct PATHs

read ann/src/makefile.darwin for more info and edit PATHs

compile with
make -f makefile.darwin
