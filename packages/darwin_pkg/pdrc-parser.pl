#!/usr/bin/perl
#
# Hans-Christoph Steiner  <hans@eds.org>
#
# This script adds things to the user's .pdrc to support the 
# included external libs and a personal external/help folder
# for each user.

# this script has a bug in it: it doesn't create 
# the dirs $EXTERNALS and $HELP

#----------------------------------------------------------------------------#

sub addLineToPdrc {
	 my $addline = shift(@_);

	 $DESTFILE = "$home/.pdrc";

	 if ( ! -e $DESTFILE ) {
		  my $now = time;
		  utime $now, $now, $DESTFILE;
	 }
	 
	 if ( ! `grep -- \'$addline\' \"$DESTFILE\"` ) {
		  print "Adding: $addline\n";
		  `echo $addline >> $DESTFILE`;
	 }	 else { print "( found: $addline )\n"; }
}

#----------------------------------------------------------------------------#

# if the user has a home dir, add stuff to it
if ( -d $ENV{'HOME'} ) {	 
	 $home = $ENV{'HOME'};
	 print "Found home dir: $home\n";

# create place for users to install their own help/externals
	 $EXTERNALS="$home/Library/Pd/Externals";
	 $HELP="$home/Library/Pd/Help";
	 if ( ! -d "$EXTERNALS" ) { mkdir("$EXTERNALS"); }
	 if ( ! -d "$HELP" ) { mkdir("$HELP"); }

	 @pdrc = (
				 "-listdev",
				 "-lib Gem",
				 "-lib iemlib1",
				 "-lib iemlib2",
				 "-lib iem_mp3",
				 "-lib iem_t3_lib",
				 "-lib pdp",
				 "-lib zexy",
				 "$EXTERNALS",
				 "$HELP"
				 );
	 
	 foreach $line (@pdrc) {
		  addLineToPdrc ($line);
	 }
	 
} else {
	 print "ERROR: no home: $ENV{'HOME'}\n";
}
