<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<!-- $Id: download.php,v 1.1 2003-03-14 21:55:18 eighthave Exp $  -->

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
<?php include('quote.inc'); ?>

</div>
<div id="Content">
<!-- Quote of the day -->

<h2>Download</h2>

We are aiming to make this page the one stop for all of your Pd downloads.

<h3>Pure Data</h3>

<h4><a href="http://sourceforge.net/project/showfiles.php?group_id%3D55736%26release_id%3D144081">pure-data</a></h4>
<p>Miller Puckette's distribution.  You can also get it from <a href="http://crca.ucsd.edu/~msp" >his site</a>.</p>

<h4><a href="http://sourceforge.net/project/showfiles.php?group_id%3D55736%26release_id%3D145993" >pd-extended</a></h4>
<p>The Pd developers community have added a few extensions to the core of MSP,
like colored audio cords, GUI glitch prevention, and more.  The pd-extended
distribtution includes these patches.</p>

<h3>Externals</h3>

<h4><a href="http://sourceforge.net/project/showfiles.php?group_id%3D55736%26release_id%3D146085"
>pd-externals</a></h4>
<p>The core package of pd externals.  It includes all externals that
do not depend on other libraries.</p>

<h4>pd-flext</h4>
<p>flext is a C++ layer for cross-platform development of Max/MSP and Pd
externals.  Package is coming soon... look <a href="http://www.parasitaere-kapazitaeten.net/Pd/ext/flext/" >here</a> in the meantime.</p>

<h4><a href="http://sourceforge.net/project/showfiles.php?group_id%3D55736%26release_id%3D146087" >pd-gem</a></h4>
<p>GEM is an OpenGL and video extension for Pd.</p>

<h4>pd-osc</h4>
<p>This is <a href="http://cnmat.cnmat.berkeley.edu/OSC/"
target="osc">OpenSoundControl</a> for Pd.  Package is coming soon... look <a href="http://barely.a.live.fm/pd/OSC/" >here</a> in the meantime.</p>

<BR>
<BR>
<CENTER>
<?php include('webring.inc'); ?>
</CENTER>
<BR>
<?php include('lastmodified.inc'); ?>

</div>

</BODY>