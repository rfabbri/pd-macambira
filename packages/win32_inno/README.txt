
Package Maker

This package is assembled using Inno Setup
(http://www.jrsoftware.org/isinfo.php ).  Check pd.iss for what's happening.

Makefile

Currently, the Makefile.nmake only compiles the 'externals' collection.  It
can also compile flext if you manually check the flext config and uncomment
things from the Makefile.nmake.  Ideally, everything would be compiled and
built from a Makefile using MinGW, so that only free software would be
needed.  Currently, to compile Pd on Windows you need MS Visual Studio 6.0 or
better.

Directory Layout

This directory is for files that are used in the creation of the Windows
installer.  In order to use this to compile/assemble Pd and externals.

 +-|
   +-abstractions
   |
   +-packages-|
   |          +-win32_inno
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


-Hans-Christoph Steiner <hans@at.or.at>

