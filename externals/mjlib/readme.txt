mjLib 

by 
mark williamson 
mailto:mark@junklight.com
http://www.junklight.com

The code is free for anyone to use provided you mention me somewhere - its not 
like its going to cost you anything :-). If you need support you can try
mailing me at the address above - I can be quite busy but I will try and 
deal with any queries.

Linux

It is built under windows but I have included the various build files needed 
for linux - delete the file "makefile" and use the configure script to 
make a new one for linux. The files needed by autoconf are there anyway 
if that doesn't work. I can't run PD on the linux machine I have got 
access to (only telnet access) so I am not sure about installing it but all the 
stuff should be there. 

Windows 

There is a VC++ 6 project file included an it builds fine with that. I haven't 
tried anyother tools as yet. However there is a binary version included 
in case you haven't got the compiler.

To install - add mjLib.dll to your pd library path: 

	-lib C:\pd\mjLib\mjLib
	
and copy the contents of doc\mjLib  into 

	[pd home]\docs\5.reference\mjLib 
	
that should be you done.

General notes

This library will grow a bit - there are a few more objects that I want to 
put into it. 

There are currently five objects: 

	pin~	- randomly delivers the input signal to either the right or left outlet with a given probability
	metroplus - allows complex timing bangs to be delivered
	prob - generates random events with a given probability
	monorhythm - basic rhythm pattern building blocks that allows polyrhthms to be generated quickly and easily	
	about - delivers a number that is "about" the same as the input number.
	
	
mark williamson 
January 2002

___________________________________________________________

history: 

1st February release 2

added new mode to monorhythm (exclusive - allows the beat and accent bangs to be mutually exclusive)
added about object

1st february release 1

added linux build files - not properly tested

31st January 2002

added prob and monorythm

30th January 2002

mods to metroplus to allow it to work just like metro is complex time mode not needed

29th january 2002

first release containing pin~ and metroplus
