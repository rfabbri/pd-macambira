
------------------------------------------------------------------------------
Software Requirements
------------------------------------------------------------------------------

Inno Setup - http://www.jrsoftware.org/isinfo.php
	  This package is assembled using Inno Setup, check pd.iss for details.

ogg vorbis win32k SDK - 
	 Install into C:\ to make it work with the current Makefile

pthreads - ftp://sources.redhat.com/pub/pthreads-win32/
	 pthreads is a standard, cross-platform threading library used in the pd 
	 core and externals.  You can use the version included with Pd.

Microsoft Visual Studio - 
    Sad but true, Pd is free software, but you need some very unfree software
	 to compile it on Windows.  You need MS Visual Studio 6.0 or better.

------------------------------------------------------------------------------
Makefile
------------------------------------------------------------------------------

Currently, the Makefile.nmake only compiles the 'externals' collection.  It
can also compile flext if you manually check the flext config and uncomment
things from the Makefile.nmake.  Ideally, everything would be compiled and
built from a Makefile using MinGW, so that only free software would be
needed.

------------------------------------------------------------------------------
Directory Layout
------------------------------------------------------------------------------

This directory is for files that are used in the creation of the Windows
installer.  In order to use this to compile/assemble Pd and externals.

 +-|
   +-abstractions
   |
   +-packages-|
   |          +-win32_inno-|
   |                       +-noncvs-|
   |                                +-extra
   |                                +-doc-|
   |                                      +-5.reference
   |
   +-doc-|
   |     +-additional
   |     +-pddp
   |     +-tutorials
   |
   +-externals-|
   |           +-...
   |           +-ext13
   |           +-ggee
   |           +-maxlib
   |           +-unauthorized
   |           +-zexy
   |           +-...
   |
   +-pd-|
        +-src
        +-doc
        +-etc...

        
The recommended way to do this is (these are probably somewhat wrong):

         mkdir pure-data && cd pure-data
         setenv CVSROOT :pserver:anonymous@cvs.sourceforge.net:/cvsroot/pure-data
         unzip pd source
         cvs checkout packages
         cvs checkout doc
         cvs checkout externals
         cd packages/win32_inno
		 make clean && make

Binary Sources I Used (I haven't tested them all, I just downloaded them):

cyclone: http://suita.chopin.edu.pl/~czaja/miXed/externs/cyclone.html
freeverb~: http://www.akustische-kunst.org/puredata/freeverb/index.html
iemlibs: http://iem.kug.ac.at/~musil/iemlib/
maxlib: http://www.akustische-kunst.org/puredata/maxlib/index.html
OSC: http://barely.a.live.fm/pd/OSC/
percolate: http://www.akustische-kunst.org/puredata/percolate/index.html
toxy: http://suita.chopin.edu.pl/~czaja/miXed/externs/toxy.html
xeq: http://suita.chopin.edu.pl/~czaja/miXed/externs/xeq.html
zexy: ftp://iem.kug.ac.at/pd/Externals/ZEXY

all of T.Grill's code: http://www.parasitaere-kapazitaeten.net/ext/




-Hans-Christoph Steiner <hans@at.or.at>

