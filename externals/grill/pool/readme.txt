pool - a hierarchical storage object for PD and Max/MSP

Copyright (c) 2002-2004 Thomas Grill (xovo@gmx.net)
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "license.txt," in this distribution.  

Donations for further development of the package are highly appreciated.
Visit https://www.paypal.com/xclick/business=t.grill%40gmx.net&item_name=pool&no_note=1&tax=0&currency_code=EUR

----------------------------------------------------------------------------

You will need the flext C++ layer for PD and Max/MSP externals to compile this.
see http://www.parasitaere-kapazitaeten.net/ext


Package files:
- readme.txt: this one
- gpl.txt,license.txt: GPL license stuff
- main.cpp, pool.h, pool.cpp, data.cpp

----------------------------------------------------------------------------

Goals/features of the package:

- pool can store and retrieve key/value pairs, where a key can be any atom and 
	the value can be any list of atoms
- pool can manage folders. A folder name can be any atom.
- pool objects can be named and then share their data space
- clipboard operations are possible in a pool or among several pools
- file operations can load/save data from disk

----------------------------------------------------------------------------

The package should at least compile (and is tested) with the following compilers:

pd - Windows:
-------------
o Borland C++ 5.5 (free): edit "config-pd-bcc.txt" & run "build-pd-bcc.bat" 

o Microsoft Visual C++ 6/7: edit "config-pd-msvc.txt" & run "build-pd-msvc.bat" 

pd - linux:
-----------
o GCC: edit "config-pd-linux.txt" & run "sh build-pd-linux.sh" 

pd - darwin (MacOSX):
---------------------
o GCC: edit "config-pd-darwin.txt" & run "sh build-pd-darwin.sh" 

Max/MSP - MacOS9/X:
-------------------
o CodeWarrior: edit "pool.cw" and build 

Max/MSP - Windows:
-------------------
o Microsoft Visual C++ 7: edit "config-max-msvc.txt" & run "build-max-msvc.bat" 


----------------------------------------------------------------------------

Version history:

0.2.0:
- attributes (pool,private,echodir,absdir)
- added "geti" message for retrieval of a value at an index
- fixed bug in "get" message if key not present
- adapted source to flext 0.4.1 - register methods at class creation
- extensive use of hashing for keys and directories
- database can be saved/loaded as XML data
- fixed bug with stored numbers starting with - or +
- relative file names will be based on the folder of the current patcher
- added printall, printrec, printroot messages for console printout
- added mkchdir, mkchsub to create and change to directories at once
- change storage object only when name has changed

0.1.0:
- first public release

---------------------------------------------------------------------------

BUGS:
- pool does not handle symbols with spaces, colons or all digits


TODO list:
- check for invalid symbols (spaces, colons)

general:
- what is output as value if it is key only? (Max->nothing!)
- XML format ok?

