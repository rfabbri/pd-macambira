#!/usr/bin/perl

# WARNING!  This script is really ugly!

use strict;
use warnings;

my $line = "";
my @lines;
my $lineCount = 0;
my $column;
my $lastColumn = 0;
my $printText = "";

my %xyhash = ();

my $library = "";
my $name = "";
my $fileName = "";

#------------------------------------------------------------------------------#
# THE OUTPUT FORMAT
#------------------------------------------------------------------------------#
format OBJECTCLASS =
{{Infobox Objectclass
| name                   = ^*
                           $name
| library                = ^*
                           $library
| author                 = {{^* author}}
                           $library
| status                 = {{^* status}}
                           $library
| website                = {{^* website}}
                           $library
| release date           = {{^* release date}}
                           $library
| license                = {{^* license}}
                           $library
| platform               = [[GNU/Linux]], [[Mac OS X]], [[Windows]]
| language               = English
| distributions          = {{^* distributions}}
                           $library
}}

@*
$printText

==Inlets==

==Outlets==

==Arguments== 

==Messages==

{{objectclass-stub}}
[[Category:objectclass]]
[[Category:^*]
$library
.


#------------------------------------------------------------------------------#
# THE PROGRAM
#------------------------------------------------------------------------------#

foreach (`/sw/bin/find /Applications/Pd-extended.app/Contents/Resources/doc/5.reference/ -type f -name '*.pd'`) {
  chop;
  $fileName = "";
  if (m|.*/doc/5\.reference/([a-zA-Z0-9_-]+)/(.+)-help\.pd|) {
	 print("library: $1  name: $2\n");
	 $library = lc($1);
	 $name = $2;
	 $fileName = $_;
  } elsif (m|.*/doc/5\.reference/([a-zA-Z0-9_-]+)/(.+)\.pd|) {
	 print("library: $1  name: $2 \t\t(no -help)\n");
	 $library = lc($1);
	 $name = $2;
	 $fileName = $_;
  }

#  print "filename: $fileName\n";  
  if ($fileName) {
	 open(HELPPATCH, "$fileName");
	 undef $/;						  # $/ defines the "end of record" character
	 $_ = <HELPPATCH>;			  # read the whole file into the scalar 
	 close HELPPATCH;
	 $/ = "\n";						  # Restore for normal behaviour later in script
  
	 s| \\||g;						  # remove Pd-style escaping
	 s|([^;])\n|$1 |g;			  # remove extra newlines
	 s|\(http://.*\)\([ \n]\)|[$1]$2|g;
  
	 @lines = split(';\n', $_);


	 foreach (@lines) {
		if (m|^#X text ([0-9]+) ([0-9]+) (.*)|) {
		  $xyhash{ $2 }{ $1 } = $3;
		  #	 print("$lineCount @ $1,$2: $3\n");
		}
		$lineCount++;
	 }

	 $printText = "";
	 for ($column = -300; $column < 1501; $column += 300) {
		foreach my $yKey ( sort {$a <=> $b} keys(%xyhash) ) {
		  foreach my $xKey ( keys(%{$xyhash{$yKey}}) ) {
			 if ( ($xKey > $lastColumn) && ($xKey < $column) ) {
				$printText .= "$xyhash{$yKey}{$xKey}\n\n";
				#$printText .= "$xKey,$yKey: $xyhash{$yKey}{$xKey}\n";
			 }
		  }
		}
		$lastColumn = $column;
	 }
	 #print("\n\n\nPRINTTEXT:\n$printText\n\n");
  
	 mkdir($library);
	 open(OBJECTCLASS, ">$library/${name}.txt");
	 write(OBJECTCLASS);
	 close(OBJECTCLASS);
  }
}
  
