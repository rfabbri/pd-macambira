This is the README file for the "extras" library, consisting of Pd
objects which are too specialized or otherwise non-canonical for
inclusion into Pd proper. Except as noted otherwise, all
included materiels are Copyright 1999 Miller Puckette and others.
Permission is granted to use this software for any purpose, commercial
or noncommercial, as long as this notice is included with all copies.

NEITHER THE AUTHORS NOR THEIR EMPLOYERS MAKE ANY WARRANTY, EXPRESS
OR IMPLIED, IN CONNECTION WITH THIS SOFTWARE!

Note that "expr" is under the GPL, which is more restrictive than Pd's own
license agreement.

This package should run in Pd under linux, MSW, or Mac OSX.
You can additionally compile fiddle~. bonk~, and paf~ for Max/MSP.

contents:

externs:
fiddle~ -- pitch tracker
bonk~ - percussion detector
choose - find the "best fit" of incoming vector with stored profiles
paf~ -- phase aligned formant generator
loop~ -- sample looper
expr -- arithmetic expression evaluation (Shahrokh Yadegari)

abstractions:
hilbert~ - Hilbert transform for SSB modulation
complex-mod~ - ring modulation for complex (real+imaginary) audio signals
rev1~ - experimental reverberator

These objects are part of the regular Pd distribution as of Pd version
0.30.  Macintosh versions of fiddle~, bonk~, and paf~ are available
from http://www.crca.ucsd.edu/~tapel
- msp@ucsd.edu
