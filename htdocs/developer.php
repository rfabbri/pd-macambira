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
<?php include('quote.inc'); ?>

</div>
<div id="Content">
<!-- Quote of the day -->



<h2>Accessing the Externals via CVS</h2>

You have to install cvs. On most Linux systems this is already installed, on
Windows or Mac OS/X you will have to download it <A
HREF="http://cvsgui.sourceforge.net/">from the net</A>.  Then, if you have cvs
installed (this example is assuming the commandline version) use the following
line to login into the CVS server:
<pre>
% cvs -d:pserver:anonymous@cvs.sourceforge.net:/cvsroot/pure-data login 
</pre> 
<p>

Hit return when you are asked for a password.  In order to get the source:
<pre>
% cvs -z3 -d:pserver:anonymous@cvs.sourceforge.net:/cvsroot/pure-data co externals
</pre>
<p>

This will give you a local copy of the externals-directory in the repository.
Other modules are for example called "abstractions" (see below) or you could
also use "." to check out everything. After some time, if you want to get all
the latest additions and updates, type (in the externals directory):

<pre>
% cvs update -d
</pre>
<p>

Without the -d only existing directories on your local copy get updated.
Depending on your configuration you might have -d as default anyways.

<p>

Currently building the externals works to same way it did before, just cd into
the external directory (e.g. cd vst) and read through the compilation
instructions.  Later this process will be automated and you can build al the
externals in one go, or even download a precompiled archive for your system.
<p>
Some other modules in the repository include Pd abstractions or the developers
version of Pd. You can check these out by replacing the modulename with the
respective modulename.
<p>
For example check out the abstractions with:
<pre>
% cvs -z3 -d:pserver:anonymous@cvs.sourceforge.net:/cvsroot/pure-data co abstractions
</pre>
<p>
or Pd with 

<pre>
% cvs -z3 -d:pserver:anonymous@cvs.pure-data.sourceforge.net:/cvsroot/pure-data co pd
</pre>
<p>
To check out the current developers' branch, which is tagged with &quot;devel_0_37&quot;, use 
<pre>
% cvs -z3 -d:pserver:anonymous@cvs.sourceforge.net:/cvsroot/pure-data co -r devel_0_37 pd
</pre>
<p>
You can find out the available branches in the pulldown menu on the project's
<a href="http://cvs.sourceforge.net/cgi-bin/viewcvs.cgi/pure-data/pd/">CVS-page</a>
(or use "cvs -T ..."). 
<a href="http://cvs.sourceforge.net/cgi-bin/viewcvs.cgi/pure-data/">Browsing CVS</a>
is a good way to see what else is in the repository anyway.
<p>


<h2>Putting Your Externals in the Repository</h2>

This description is mainly about Linux, if you are working with Windows, you
might try the procedure described in this <a
href="http://sfsetup.sourceforge.net/tutorial_cvsaccess.html">tutorial</a>. Let
us know if it works.
<p>

To contribute your pd externals to the repository you'll first have to register
yourself at sourceforge (making you an official developer of the project). Go
to sourceforge.net and click on the "new user" link on the upper left corner.
Then <A HREF="mailto:geiger@xdv.org">send an email </A> with the user you
created and Guenter will add you to the pure-data developer list.
<p>

At the <A HREF="HTTP://sourceforge.net/projects/pure-data">project page</A> you
can see if you are already listed as a developer. The next step is to "import"
your source code. First make sure that your code is in a directory, and remove
everything from that directory that you don't want to put under CVS.  Compiled
code, for example, is not normally put into CVS. 
<p>

Make sure you are in this directory and issue the command:
<p>
<pre>
% export CVS_RSH=ssh
% cvs -z3 -d:ext:developername@cvs.sourceforge.net:/cvsroot/pure-data \
  import externals/dirname developername source-dist
</pre>
<p>

Exchange "developername" with your sourceforge accountname and "dirname" with
the  name of your  externals directory.  You have to import the source only
once for your external(s).
<p>

If all of this went well, move away your external directory (keep it as a
backup) and checkout the code with:
<pre>
% cvs -z3 -d:ext:developername@cvs.sourceforge.net:/cvsroot/pure-data \
  co externals
</pre>
<p>

From this point on, if you are working in your externals directory, and want to
commit your changes to the server you just have to do
<pre>
% cvs commit
</pre>
<p>

or 
<pre>
% cvs update
</pre>
<p>

to take a look at what you have changed.
<p>

All the information about where the repository is, the loginname etc, is in the
"CVS" directory, that should be part of your external directory now.
<p>

If you are still unsure about the workings, take a look at this <a
href="http://www.cvshome.org/docs/blandy.html">introduction</A> about using
CVS.


<?php include('lastmodified.inc'); ?>

</div>

</BODY></HTML>

