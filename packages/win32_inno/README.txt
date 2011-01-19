

Check this webpage for full build instructions:
(32-bit):  http://puredata.info/docs/developer/mingw 
(64-bit):  http://puredata.info/docs/developer/Windows64BitMinGWX64

------------------------------------------------------------------------------
Software Requirements
------------------------------------------------------------------------------

MinGW
	 MinGW provides a free, complete build environment for Pd.

Inno Setup - http://www.jrsoftware.org/isinfo.php
	 This package is assembled using Inno Setup, check pd.iss for details.

ogg vorbis win32k SDK - 
	 Install into C:\ to make it work with the current Makefile

Tcl/Tk
	 Compile for MinGW.

pthreads - ftp://sources.redhat.com/pub/pthreads-win32/
	 pthreads is a standard, cross-platform threading library used in the pd 
	 core and externals.  You can use the version included with Pd.

MinGW/gcc
	 Pd is free software, and can be compiled using free tools.  MinGW is the
	 preferred way of compiling Pd on Windows.

Microsoft Visual Studio - 
	 You can use MS Visual Studio 6.0 or better to compile Pd and some


------------------------------------------------------------------------------
Microsoft Visual Studio Makefile
------------------------------------------------------------------------------

You will need to do this to compile:

nmake /f Makefile.nmake

Currently, the Makefile.nmake only compiles the 'externals' collection.  It
can also compile flext if you manually check the flext config and uncomment
things from the Makefile.nmake.

------------------------------------------------------------------------------
Directory Layout
------------------------------------------------------------------------------

This directory is for files that are used in the creation of the Windows
installer.  In order to use this to compile/assemble Pd and externals.
 http://puredata.info/docs/developer/devlayout
        
The recommended way to get all this source is:
 http://puredata.info/docs/developer/GettingPdSource

-Hans-Christoph Steiner <hans@eds.org>

