FFTease - A set of Live Spectral Processors
Originally written by Eric Lyon and Christopher Penrose for the Max/MSP platform

Copyright (c)Thomas Grill (xovo@gmx.net)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  


----------------------------------------------------------------------------

You will need the flext C++ layer for PD and Max/MSP externals to compile this.
see http://www.parasitaere-kapazitaeten.net/ext

----------------------------------------------------------------------------


BUILDING:
=========


pd - Windows:
-------------
OK o Microsoft Visual C++ 6: edit "config-pd-msvc.txt" & run "build-pd-msvc.bat" 

o Cygwin: edit "config-pd-cygwin.txt" & run "sh build-pd-cygwin.sh" 
	additional settings (e.g. target processor, compiler flags) can be made in makefile.pd-cygwin


pd - linux:
-----------
o GCC: edit "config-pd-linux.txt" & run "sh build-pd-linux.sh" 
	additional settings (e.g. target processor, compiler flags) can be made in makefile.pd-linux


pd - MacOSX:
-----------
OK o GCC: edit "config-pd-darwin.txt" & run "sh build-pd-darwin.sh" 
	additional settings (e.g. target processor, compiler flags) can be made in makefile.pd-darwin


Max/MSP - MacOS 9:
------------------
OK o Metrowerks CodeWarrior V6: edit & use the "fftease.cw" project file

You must have the following "Source Trees" defined:
"flext" - Pointing to the flext main directory
"Cycling74" - Pointing to the Cycling 74 SDK



Max/MSP - MacOSX:
------------------
OK o Metrowerks CodeWarrior V6: edit & use the "fftease.cw" project file

You must have the following "Source Trees" defined:
"OS X Volume" - Pointing to your OSX boot drive
"flext" - Pointing to the flext main directory
"Cycling74 OSX" - Pointing to the Cycling 74 SDK for xmax
"MP SDK" - Pointing to the Multiprocessing SDK (for threading support)


----------------------------------------------------------------------------

PORTING NOTES:

The example audio files schubert.aiff and nixon.aiff have been taken from the original FFTease package for Max/MSP.


- pv-lib:
	- gcc (OSX) complains about _cfft being defined by pv-lib and pd.... any problems with that?

- burrow:
	- max_bin calculation: fundamental frequency seems to be wrong

- cross:
	- STRANGE: spectral amplitude in channel1 is undefined if gainer <= threshie
			-> value of previous frame is used then
	- (jmax) BUG: a2 for i == N2 is calculated from buffer1 
	- what about the class members for "correction"?! (superfluous)

- dentist:
	- tooth count ("teeth") is preserved and checked on every reshuffle
	- use different knee correction 

- disarray:
	- different frequency correction employed
	- max_bin calculation: fundamental frequency seems to be wrong
	- check whether freq oder number of bins should be selectable -> frequency!

- ether:
	- possibility to change qual?

- scrape:
	- maxamp is computed (from spectral amplitudes) before these are set!! (function frowned) -> corrected

- shapee:
	- danger of div by 0... corrected

- swinger:
	- (jmax) phase is calculated from signal1 (instead of correct signal 2)!! 

