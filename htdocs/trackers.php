<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">

<!-- $Id: trackers.php,v 1.2 2004-11-15 15:07:53 ggeiger Exp $  -->

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


Bugs, Patches and Requested Features can be submitted via the Sourceforge
tracker system. Please read through the sections below.

<h2>Bug reporting</h2>

<p>
Click on <a href="http://sourceforge.net/tracker/?func=add&group_id=55736&atid=478070">Submit Bug</a> to submit a bug against one of the subprojects.
</p>
<p>
It is important that you assign the bug to the correct subproject. This
is done by selecting the appropriate Category from the category tab. Before
submitting a bug report, you might check if the bug you encountered hasn't been
reported already by looking at the <a href=http://sourceforge.net/tracker/?group_id=55736&atid=478070>open bug reports</a> page.
</p>
Please, before reporting a bug, try out the latest version of the software.

<h2>Patches</h2>

<p> 
This tracker is a collection of up to date patches (patches that apply
cleanly against the latest version of Pd. The latest version can either be
found as a <a href="http://crca.ucsd.edu/~msp/Software/">downloadable file on Miller Puckettes site</a> or in the <a href="http://cvs.sourceforge.net/viewcvs.py/pure-data/pd/">HEAD branch of the CVS</a>. 
</p>
<br>
<p>
The main purpose of the patches tracker is to make it easier to incorporate
new features and bug fixes into the Pd core. Including new stuff may take some
time, and the inclusion of some features might not get accepted. The final
decision if your patch gets accepted is in the hands of Miller. If you
think your patch has been forgotten, please communicate it on the pd
developers list or to Miller directly.
</p>
<br>
<p>
If your patch is a bug fix, please choose the group "bugfix" in order
to mark it clearly.
</p>
<br>
<p>
All patches need to have a detailed description. This can later be used
to decide if the patch can go into the main Pd distribution or not, and
should also minimize the work that has to be done in order to adapt new
ideas. Click to <a href="http://sourceforge.net/tracker/?func=add&group_id=55736&atid=478072">Submit a patch</a>.
</p>
<br>
<p>
If you submit a patch, take care that you only send the parts that have to
be changed. Formatting changes and changes in whitespace will only make the
patch less readable. Send the patch in unified diff format.<br>
<br>
e.g. from a CVS directory:
<br>
<br>
cvs diff -uBw > cool_feature.patch
<br>
The patches collection might also be useful for users, as it makes it possible
to get the freshest bugfixes and new features that didn't make it into the  
main distribution yet.
<a href="http://sourceforge.net/tracker/?group_id=55736&atid=478072">Check out the patches</a>.

<h2>Feature Requests</h2>

<p>
There is also a tracker for feature requests. A feature request can be sent if
you are missing a functionality, but you don't know how to code it. This could
be an additional external or functionality of the Pd core.
</p>
Click <a href="http://sourceforge.net/tracker/?func=add&group_id=55736&atid=478073">Add Feature Request</a> to add a new feature request.
<br>
<a href="http://sourceforge.net/tracker/?group_id=55736&atid=478073">Take a look at the feature requests"</a> that are already registered.
<BR>
<BR>
<CENTER>
<?php include('webring.inc'); ?>
</CENTER>
<BR>
<?php include('lastmodified.inc'); ?>

</div>

</BODY>
