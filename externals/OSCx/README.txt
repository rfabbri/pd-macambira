OSC, OpenSoundControl for pd
============================
OSC: http://cnmat.cnmat.berkeley.edu/OSC
pd: http://lena.ucsd.edu/~msp/

ok, merged the windows and linux trees.
for linux do the usual makes etc, for window either use extra/OSC.dll,
.dsw and .dsp files are also included.


files:

OSC/		contains the code for OSC pd objects (send,dump,route)
README.txt	this file
doc/		pd help files
extra/		OSC.dll, the windows binary
libOSC/		CNMAT's OSC library
send+dump/	CNMAT's OSC commandline utils
                http://cnmat.cnmat.berkeley.edu/OpenSoundControl/


log:

  20020417: 0.16-2: more changes by raf + jdl (send with no argument fix, send / fix,
            ...)

  20020416: added bundle stuff to sendOSC

  200204: 0.15b1: windowified version and implied linux enhancements
          by raf@interaccess.com
	  for now get it at http://207.208.254.239/pd/win32_osc_02.zip
	  most importantly: enhanced connect->disconnect-connect behaviour
          (the win modifications to libOSC are still missing in _this_
	   package but coming ..)


  200203: 0-0.1b1: all the rest
	  ost_at_test.at + i22_at_test.at, 2000-2002
      	  modified to compile as pd externel




INSTALL:
 (linux)

tar zxvf OSCx.tgz
cd OSCx
cat README
cd libOSC && make
cd ../OSC && "adjust makefile" && make OSC && make install
cd ../..
pd -lib OSC OSCx/doc/OSC-help.pd

 PITFALLS:
make sure you compile libOSC before OSC objects
maybe adjust include path so pd include files will be found


 (windo$)

unzip and put .dll file in a pd-searched folder.


TYPETAGS:
supported and on by default. can be swtiched off with the "typetags 0"
message and on with 1.


TODO
====
-timetags: output timetag when receiving a bundle for scheduling
-TCP mode
-address space integration with pd patch/subpatch/receive hierarchy ?

see also TODO.txt in OSC/

--
jdl at xdv.org, http://barely.a.live.fm/pd/OSC
windows version:
raf at interaccess.com, http://207.208.254.239/pd/win32_osc_02.zip
