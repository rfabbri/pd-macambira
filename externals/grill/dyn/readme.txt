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

0.0.2:
- give the dyn~ subcanvas a name (hard to access - for the wild scripters out there),
	so that it is different from the canvas where dyn~ is in
- corrected names of message in- and out-proxies.
- manually retrigger DSP after loading an abstraction

0.0.1:
- send loadbangs for loaded abstractions
- now use "dsp" message to enable dsp in sub-canvas (no need of canvas_addtolist, canvas_takefromlist any more)


0.0.0 - initial cvs version



TODO:
--------
- support message boxes (do we need them?)
- Hash table for object tags
