dyn~ - dynamic object management for PD

Copyright (c)Thomas Grill (xovo@gmx.net)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  


----------------------------------------------------------------------------

The package should at least compile (and is tested) with the following compilers/platforms:

pd - Windows:
-------------
o Microsoft Visual C++ 6: edit "config-pd-msvc.txt" & run "build-pd-msvc.bat" 


pd - linux:
-----------
o GCC: edit "config-pd-linux.txt" & run "sh build-pd-linux.sh" 
	additional settings (e.g. target processor, compiler flags) can be made in makefile.pd-linux


pd - MacOSX:
-----------
o GCC: edit "config-pd-darwin.txt" & run "sh build-pd-darwin.sh" 
	additional settings (e.g. target processor, compiler flags) can be made in makefile.pd-darwin

----------------------------------------------------------------------------

CHANGES:
--------

0.0.1:
- send loadbangs for loaded abstractions


0.0.0 - initial cvs version



TODO:
--------
- Hash table for object tags
