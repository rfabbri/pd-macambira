<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
<HTML>
<HEAD>
<TITLE>Pure Data External Repository</TITLE>
<link rel="stylesheet" type="text/css" href="pd.css" media="screen">

</HEAD>
<BODY text="#000000" bgcolor="#ffffff" >

<h1>Pure Data External Repository</h1>

<div id="Menu">

<?php include('menu.inc'); ?>
<br>
<br>
<!-- Quote of the day -->
<?php include('quote.inc'); ?>

</div>
<div id="Content">

<h2>Projects at the Pure Data Ext. Repository</h2>
<p>The following projects already have put their sources inside the Pure Data
External Repository:
<pre>
	externals
	|-- <a href="#OSCx">OSCx</a>
	|-- <a href="#aenv~">aenv~</a>
	|-- <a href="#ann">ann</a>
	|-- <a href="#arraysize">arraysize</a>
	|-- <a href="#chaos">chaos</a>
	|-- <a href="#creb">creb</a>
	|-- <a href="#cxc">cxc</a>
	|-- <a href="#debian">debian</a>
	|-- <a href="#ext13">ext13</a>
	|-- <a href="#footils">footils</a>
	|-- <a href="#ggee">ggee</a>
	|-- <a href="#grill">grill</a>
	|   |-- <a href="#deljoin">deljoin</a>  
	|   |-- <a href="#delsplit">delsplit</a>  
	|   |-- <a href="#flext">flext</a>
	|   |-- <a href="#pguitest">pguitest</a>  
	|   |-- <a href="#idelay">idelay</a>  
	|   |-- <a href="#namedobjs">namedobjs</a> 
	|   |-- <a href="#pool">pool</a>
	|   |-- <a href="#prepend">prepend</a>
	|   |-- <a href="#py">py</a>
	|   |-- <a href="#vasp">vasp</a> 
	|   `-- <a href="#xsample">xsample</a>
	|-- <a href="#maxlib">maxlib</a>
	|-- <a href="#pdogg">pdogg</a>
	|-- <a href="#plugin~">plugin~</a>
	|-- <a href="#rhythm_estimator">rhythm_estimator</a>
	|-- <a href="#sprinkler">sprinkler</a>
	|-- <a href="#susloop~">susloop~</a>
	|-- <a href="#svf~">svf~</a>
	|-- <a href="#vbap">vbap</a>
	|-- <a href="#vst">vst</a>
	|-- <a href="#zexy">zexy</a>
	`-- <a href="#zhzxh~">zhzxh~</a>
</pre>
<p>
You also can browse the Repository contents at the projects <a href="http://cvs.sourceforge.net/cgi-bin/viewcvs.cgi/pure-data/">CVS-Page</a>.
<h2>Missing externals</h2>
<p>There are externals missing, I know. Please mail your descriptions to
<a href="mailto:fbar@footils.org">fbar@footils.org</a>. Thank you for your attention.

	
<a name="OSCx"></a> <h3>OSCx</h3><p>
OSC, OpenSoundControl for pd
<br>by jdl at xdv.org
<a name="aenv~"></a> <h3>aenv~</h3><p>
aenv~: asymptotic ADSR envelope generator; The output value approaches the
target values as asymptotes. 
<br> (c) Ben Saylor
<a name="ann"></a> <h3>ann</h3><p>
/* ...this is an externals for comouting Aritficial Neural Networks...
<br>   thikn aboiut this
<br>        
<br>   0201:forum::für::umläute:2001
<br>*/

<a name="arraysize"></a> <h3>arraysize</h3>
<p> arraysize -- report the size of an array

<a name="chaos"></a> <h3>chaos</h3><p>
"Chaos PD Externals" a set of objects for PD which 
calculate various "Chaotic Attractors"; including, Lorenz, Rossler, Henon 
and Ikeda. Hopefully more will be on their way. 
<br>Copyright Ben Bogart 2002
<a name="creb"></a> <h3>creb</h3><p>
This is a collection of pd externals. No fancy stuff, just my
personal bag of (ahem) tricks...
<br>(c)Tom Schouten
<a name="cxc"></a> <h3>cxc</h3>
<p>
--------------------------
<br>  cxc pd eternals library
<br>  powered by  zt0ln d4ta
<br>--------------------------
<br> many useful externals.
<a name="debian"></a> <h3>debian</h3><p>
From the README.Debian:
<br>&lt;possible notes regarding this package - if none, delete this file&gt; ;)
<p> Debian users can check out all externals and build a Debian package of 
most in one command.
<a name="ext13"></a> <h3>ext13</h3>
<p>this ist ext13, another highly useful collection of externals for pd

<a name="footils"></a> <h3>footils</h3>
<p>
externals for classic synthesis techniques like fm, granular, soundfont, ... 
<br>flext-iiwu | rx7 | shabby | syncgrain 
<a name="ggee"></a> <h3>ggee</h3><p>
Your host's Guenther Geiger's ggee externals collection.  They serve different
purposes, ranging from objects for building a simple User interface for pd
patches, to objects interfacing Perry Cooks STK, streaming audio over the LAN,
Filter implementations and other.
<a name="grill"></a> <h3>grill</h3><p>
Various externals, libraries and development tools by Thomas Grill (xovo@gmx.net). Synched to
<a
href="http://www.parasitaere-kapazitaeten.net/ext">www.parasitaere-kapazitaeten.net/ext</a>.
Please see the following descriptions:
<a name="deljoin"></a>
<h4>deljoin</h4>
<p>join a list with delimiter
<a name="delsplit"></a>
<h4>delsplit</h4>
<p> split a delimited list-in-a-symbol
<h4><a name="flext"></a>flext</h4>
<p>
flext - C++ layer for Max/MSP and pd (pure data) externals
<br>
This package seeks to encourage the development of open source software
for the pd and Max/MSP platforms.

<a name="guitest"></a>
<h4>guitest</h4>
<p> Experimental wrapper for writing GUI externals.
<a name="idelay"></a>
<h4>idelay</h4>
<p> Interpolating delay line
<a name="namedobjs"></a>
<h4>namedobjs</h4>
<p>retrieve named objects in a patcher
<h4><a name="pool"></a>pool</h4><p>
pool - a hierarchical storage object for PD and Max/MSP

<a name="prepend"></a>
<h4>prepend</h4>
<p> prepend - just like in MaxMSP
<h4><a name="py"></a>py</h4><p>
py/pyext - python script objects for PD (and MaxMSP... once, under MacOSX and Windows)
<a name="vasp"></a>
<h4>vasp</h4>
<p>VASP modular - vector assembling signal processor
<p>GOALS/FEATURES
<br>===============
<p>
VASP is a package for PD or MaxMSP consisting of a number of externals extending 
these systems with functions for non-realtime array-based audio data processing. 
VASP is capable of working in the background, therefore not influencing eventual 
dsp signal processing.


<h4><a name="xsample"></a>xsample</h4><p>
xsample - extended sample objects for Max/MSP and pd (pure data)


<a name="maxlib"></a> <h3>maxlib</h3>
<p>
maxlib - music analysis extensions library
<p> The objects can be very useful to analyse any musical performance. Some
of the objects are 'borrowed' from Max (they are not ported but
rewritten for Pd - cheap immitations).
maxib has recently been extended by objects of more general use and some 
which can be use for composition purposes. See 
<a href="http://www.akustische-kunst.org/puredata/maxlib/">http://www.akustische-kunst.org/puredata/maxlib/</a>
<br> (c) 2002 by Olaf Matthes
<a name="pdogg"></a> <h3>pdogg</h3>
<p>
Superior open source audio compression with OGG Vorbis has come to Pd.
<br> (c) 2002 by Olaf Matthes
<a name="plugin~"></a> <h3>plugin~</h3>
<p>LADSPA and VST plug-in hosting for Pd
<p></p>
This is a Pd tilde object for hosting LADSPA and VST audio plug-ins on Linux
and Windows systems, respectively.  The <a
href="http://www.ladspa.org">LADSPA</a> plug-in interface is supported
completely on Linux, while the VST 1.0 audio processing plug-in interface
(without plug-in graphics) is supported on Windows.
<br>Jarno Seppänen, jams@cs.tut.fi
<a name="rhythm_estimator"></a> <h3>rhythm_estimator</h3>
<p>This is a collection of Pd objects for doing rhythm (quantum)
estimation.
<br>Jarno Seppänen jams@cs.tut.fi and Piotr Majdak p.majdak@bigfoot.com

<a name="sprinkler"></a> <h3>sprinkler</h3>
<p>
'sprinkler' objects do dynamic control-message dissemination.
<br>
Given a list as input, a 'sprinkler' object interprets the initial
list element as the name of a 'receive' object, and [send]s the
rest of the list to that object.
<br>Bryan Jurish &lt;moocow@ling.uni-potsdam.de&gt;

<a name="susloop~"></a> <h3>susloop~</h3>
<p>sample player with various loop methods (ping-pong, ... ) think tracker.

<a name="svf~"></a> <h3>svf~</h3>
<p>This is a signal-controlled port of Steve Harris' state variable filter <a
href="http://plugin.org.uk">LADSPA plugin</a> <br>By Ben Saylor, <a
href="http://www.macalester.edu/~bsaylor">http://www.macalester.edu/~bsaylor</a>

<a name="vbap"></a> <h3>vbap</h3>
<p> Vector Based Amplitude Panning. Use, if you need to control sound locations
in space.

<a name="vst"></a> <h3>vst</h3>
<p></p>VST 2.0 support external. It supports the "string" interface for
parameters as well as providing access to the graphical interface supplied by
the plugin's creator. VSTi's can have midi information supplied to them and
export automation data. All in all it provides a very flexable mechanism to use
VST plugins outside of the Cubase environment. This download is the compiled
external DLL and PDF help file.

<a name="zexy"></a> <h3>zexy</h3>
<p>the zexy external
<p>
general::<br> the zexy external is a collection of externals. Including matrix
operations.
<a name="zhzxh~"></a> <h3>zhzxh~</h3>
<p> by Ben Saylor <a href="http://www.macalester.edu/~bsaylor">http://www.macalester.edu/~bsaylor</a>
Turns the input signal into a staticky, distorted mess. Comes with tone
control.

<BR>
<BR>
<p align="center">
<?php include('webring.inc'); ?>
<br clear="all">

<!-- REMOVE CMNTS AT PUBLISHING: -->
<A href="http://sourceforge.net"><IMG
 src="http://sourceforge.net/sflogo.php?group_id=55736&amp;type=5"
 width="210" height="62" border="0" alt="SourceForge Logo"></A>

</p>
<?php include('lastmodified.inc'); ?>

</div>

</BODY></HTML>
