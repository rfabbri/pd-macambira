Fiddle is copyright (C) 1997 Regents of the University of California.  
Permission is granted to use this software for any noncommercial purpose.
For commercial licensing contact the UCSD Technology Transfer Office.

UC MAKES NO WARRANTY, EXPRESS OR IMPLIED, IN CONNECTION WITH THIS SOFTWARE!

----------------------------------------------------------------------------

This is the README file for the "fiddle" audio pitch detector.  This software
is available from http://man104nfs.ucsd.edu/~mpuckett in versions for
IRIX 5.x and for NT on Intel boxes.

Fiddle will soon be available separately for Max/MSP on Macintoshes.

TO INSTALL FOR IRIX 5.x:  download from the Web page, which will give you a
file named "fiddle-x.xx.tar.Z" (where x.xx is the version number).  Unpack this
by typing "zcat fiddle-x.xx.tar.Z | tar xf -" which will give you a directory
named "fiddle" containing the source, the object code, and the "help patch."

TO INSTALL FOR NT:  download from the Web page, which will give you a file
named "fiddle-x.xx.zip" (where x.xx is the version number).  Unpack this by
typing "unzip fiddle-x.xx.zip" which will give you a directory named "fiddle". 
The source, the object code, and the "help patch" will be there.

Pd currently has no search path facility; the object file (fiddle.pd_irix5 or
fiddle.dll) should be copied to the directorys containing whatever patches you
want to use it with.

Please note that the copyright notice on Fiddle is more restrictive than for
Pd; this is to prevent Fiddle from being incorporated in any commercial product
that could compete with IRCAM's audio analysis tools.

-Miller Puckette (msp@ucsd.edu)
