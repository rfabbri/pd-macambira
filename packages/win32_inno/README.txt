
This package is assembled using Inno Setup
(http://www.jrsoftware.org/isinfo.php ).  Check pd.iss for what's happening.

Currently, the Makefile.nmake only compiles the 'externals' collection.  It
can also compile flext if you manually check the flext config and uncomment
things from the Makefile.nmake.  Ideally, everything would be compiled and
built from a Makefile using MinGW, so that only free software would be
needed.  Currently, to compile Pd on Windows you need MS Visual Studio 6.0 or
better.

-Hans-Christoph Steiner <hans@at.or.at>

