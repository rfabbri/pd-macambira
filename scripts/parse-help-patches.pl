#!/usr/bin/perl

# WARNING!  This script is really ugly!

use strict;
use warnings;
use Text::CSV_PP;

my $line = "";
my @lines;
my $lineCount = 0;
my $column;
my $lastColumn = 0;
my $printText = "";

my %classnames = ();
my %xyhash = ();

my $library = "";
my $name = "";
my $fileName = "";
#------------------------------------------------------------------------------#
# THE OUTPUT FORMAT
#------------------------------------------------------------------------------#
format OBJECTCLASS =


==Inlets==



==Outlets==



==Arguments== 



==Messages==



{{objectclass-stub}}

[[Category:objectclass]]
.


#------------------------------------------------------------------------------#
# PARSE CSV
#------------------------------------------------------------------------------#
my $csvfile = '/Users/hans/Desktop/wiki_files_hacked/objectlist.csv';
my $csv = Text::CSV_PP->new();
my %csvhash = ();

open (CSV, "<", $csvfile) or die $!;
my @csvlines = split(/\012\015?|\015\012?/,(join '',<CSV>));
foreach (@csvlines) {
	 if ($csv->parse($_)) {
		  my @columns = $csv->fields();
		  $csvhash{ $columns[0] }{ $columns[2] } = "$columns[0],$columns[2],$columns[3],$columns[4],$columns[5],$columns[7]";
		  #print("$columns[0],$columns[2] | ");
	 } else {
  		  my $err = $csv->error_input;
  		  print "Failed to parse line: $err";
	 }
}
close CSV;

#------------------------------------------------------------------------------#
# PARSE HELP FILES
#------------------------------------------------------------------------------#

foreach (`/sw/bin/find /Users/hans/Desktop/wiki_files_hacked/5.reference/ -type f -name '*.pd'`) {
  chop;
  $fileName = "";
  if (m|.*/5\.reference/([a-zA-Z0-9_-]+)/(.+)-help\.pd|) {
#	 print("$1 , $2\t");
	 $library = lc($1);
	 $name = $2;
	 $fileName = $_;
  } elsif (m|.*/5\.reference/([a-zA-Z0-9_-]+)/(.+)\.pd|) {
#	 print("$1 , $2 (no -help)\t");
	 $library = lc($1);
	 $name = $2;
	 $fileName = $_;
  }

#  print "filename: $fileName\n";  
  if ($fileName) {
	 $printText = "";            # init container
	 %xyhash = ();               # init sorting hash

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

	 for ($column = -300; $column < 1501; $column += 300) {
		foreach my $yKey ( sort {$a <=> $b} keys(%xyhash) ) {
		  foreach my $xKey ( keys(%{$xyhash{$yKey}}) ) {
			 if ( ($xKey > $lastColumn) && ($xKey < $column) ) {
				$printText .= "$xyhash{$yKey}{$xKey}\n\n";
				#print("TEST $xKey,$yKey: $xyhash{$yKey}{$xKey}\n");
			 }
		  }
		}
		$lastColumn = $column;
	 }
	 
	 my $abbreviation = "";
	 my $description = "";
	 my $category = "";
	 my $datatype = "";
	 my $myColumns = $csvhash{$library}{$name};
	 my @myColumns;
	 if($myColumns) { @myColumns = split(',', $myColumns); }
#	 print("csvhash{$library}{$name}:  $csvhash{$library}{$name}\n");
	 if($myColumns[0]) {
		if($myColumns[2]) { $abbreviation = $myColumns[2] }
		if($myColumns[3]) { $description = $myColumns[3] }
		if($myColumns[4]) { $category = $myColumns[4] }
		if($myColumns[5]) { $datatype = $myColumns[5] }
#		print("MYCOLUMNS: $myColumns[0] $myColumns[1] $myColumns[2] $myColumns[3] $myColumns[4] $myColumns[5]\n");
	 }
	 
	 mkdir($library);
	 if( $classnames{$name} ) {
		open(OBJECTCLASS, ">$library/${name}_(${library}).txt");
	 } else {
		open(OBJECTCLASS, ">$library/${name}.txt");
	 }
	 print(OBJECTCLASS "{{Infobox Objectclass\n");
	 print(OBJECTCLASS "| name                   = $name\n");
	 if($abbreviation) {
		print(OBJECTCLASS "| abbreviation           = $abbreviation\n");}
	 if($description) {
		print(OBJECTCLASS "| description            = $description\n");}
	 if($datatype) {
		print(OBJECTCLASS "| data type              = $datatype\n");}
	 print(OBJECTCLASS "| library                = [[$library]]\n");
	 print(OBJECTCLASS "| author                 = {{$library author}}\n");
	 print(OBJECTCLASS "| license                = {{$library license}}\n");
	 print(OBJECTCLASS "| status                 = {{$library status}}\n");
	 print(OBJECTCLASS "| website                = {{$library website}}\n");
	 print(OBJECTCLASS "| release date           = {{$library release date}}\n");
	 print(OBJECTCLASS "| distributions          = {{$library distributions}}\n");
	 print(OBJECTCLASS "| language               = English\n");
	 print(OBJECTCLASS "| platform               = [[GNU/Linux]], [[Mac OS X]], [[Windows]]\n");
	 print(OBJECTCLASS "}}\n\n");
	 print(OBJECTCLASS "\n$printText\n");
	 write(OBJECTCLASS);
	 print(OBJECTCLASS "[[Category:$library]]\n");
	 if($category) {
		print(OBJECTCLASS "[[Category:$category]]\n");
	 }
	 print(OBJECTCLASS "\n\n");
	 close(OBJECTCLASS);

	 $classnames{$name} = 1;
  }
}
  
