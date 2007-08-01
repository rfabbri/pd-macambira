    README for externals/moocow/extended/ build hacks.

    Last updated Thu, 02 Aug 2007 00:48:50 +0200

DESCRIPTION
    This directory is for pd-extended compatible builds of (some of)
    moocow's externals directly from the CVS repository.

USAGE
    Issuing the following commands to the shell:

      cd externals/moocow/extended (or wherever you extracted the distribution)
      make

    ... should result in all objects being compiled into
    extended/build/externs. This is intended to be called from
    externals/Makefile.

SUPPORTED EXTERNALS
    This makefile currently supports the following of moocow's externals:

     deque
     pdstring    (just the dummy object, not the library!)
     any2string
     string2any
     readdir
     weightmap

    The following of moocow's externals are unsupported (for various
    reasons):

     flite
     gfsm
     ratts

ACKNOWLEDGEMENTS
    Pd by Miller Puckette and others.

    Ideas, black magic, and other nuggets of information drawn from code by
    Guenter Geiger, iohannes m zmoelnig, Hans-Christoph Steiner, and others.

KNOWN BUGS
    None known.

AUTHOR
    Bryan Jurish <moocow@bbaw.de>

