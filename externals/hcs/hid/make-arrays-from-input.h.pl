#!/usr/bin/perl -w

use Switch;

# BUGS: FF_STATUS, REP, SND, SYN are not building properly. Its still dropping the first element


#========================================================================
# GLOBAL VARS
#========================================================================

$FILENAME = "linux/input.h";

#========================================================================
# FUNCTIONS
#========================================================================

#------------------------------------------------------------------------
# parse out the types and codes from a line from the header 
# usage:  @dataArray getDataFromHeaderLine($lineFromHeader);
sub getDataFromHeaderLine 
{
	 $_ = shift;

	 my @returnArray;

	 if ( m/#define ([A-Z0-9_]*)\s+(0x)?([0-9a-f]+)/) 
	 { 
		  if ($2) { $index = hex($3); } else { $index = $3; }
		  if ($index >=0) 
		  {
				$returnArray[0] = $index;
				$returnArray[1] = "$1";
				return @returnArray;
		  }
#		  print "$1 \t\t\t $index   $#returnArray\n "; 
#		  if ($index == 0) { print("index: $index   3: $3  2: $2   1: $1   value: $returnArray[1]\n"); }
	 } 
}

#------------------------------------------------------------------------
# print an array out in C format
#
sub printCArray
{
	 my @arrayToPrint = @_;

#	 print("$arrayToPrint[0] $#arrayToPrint \n");

	 print("int ${arrayToPrint[0]}_TOTAL = $#arrayToPrint;  /* # of elements in array */\n");
	 print("char *${arrayToPrint[0]}[$#arrayToPrint] = {");

	 for($i = 1; $i < $#arrayToPrint; $i++)
	 {
		  # format nicely in sets of 6
		  if ( ($i+4)%6 == 5 ) { print("\n       "); }
		  # if the array element's data is null, print NULL
		  if ($arrayToPrint[$i]) { print("\"$arrayToPrint[$i]\","); }
		  else { print("NULL,"); }
	 }

	 print("\"$arrayToPrint[$#arrayToPrint]\"\n };\n\n\n");
}

#========================================================================
# MAIN
#========================================================================

$FILENAME = "linux/input.h";

open(INPUT_H, "<$FILENAME");

while(<INPUT_H>)
{
	 if (m/#define (FF_STATUS|[A-Z_]*?)_/)
	 {
# filter EV_VERSION and *_MAX
		  m/#define\s+(EV_VERSION|[A-Z_]+_MAX)\s+/;
#		  print "$1 \n";
		  switch ($1) 
		  {
		  # types
				case "EV"        { ($index, $value) = getDataFromHeaderLine($_); $EV[$index] = $value; }
        # codes
				case "SYN"       { ($index, $value) = getDataFromHeaderLine($_); $SYN[$index] = $value; }
				case "KEY"       { ($index, $value) = getDataFromHeaderLine($_); $KEY[$index] = $value; }
# BTN codes are actually part of the KEY type
				case "BTN"       { ($index, $value) = getDataFromHeaderLine($_); $KEY[$index] = $value; }
				case "REL"       { ($index, $value) = getDataFromHeaderLine($_); $REL[$index] = $value; }
				case "ABS"       { ($index, $value) = getDataFromHeaderLine($_); $ABS[$index] = $value; }
				case "MSC"       { ($index, $value) = getDataFromHeaderLine($_); $MSC[$index] = $value; }
				case "LED"       { ($index, $value) = getDataFromHeaderLine($_); $LED[$index] = $value; }
				case "SND"       { ($index, $value) = getDataFromHeaderLine($_); $SND[$index] = $value; }
				case "REP"       { ($index, $value) = getDataFromHeaderLine($_); $REP[$index] = $value; }
				case "FF"        { ($index, $value) = getDataFromHeaderLine($_); $FF[$index] = $value; }
# there doesn't seem to be any PWR events yet...
				case "PWR"       { ($index, $value) = getDataFromHeaderLine($_); $PWR[$index] = $value; }
				case "FF_STATUS" { ($index, $value) = getDataFromHeaderLine($_); $FF_STATUS[$index] = $value; }
#				else { print " none $_"; } 
		  }
	 }
}

printCArray("EV",@EV);
printCArray("SYN",@SYN);
printCArray("KEY",@KEY);
printCArray("REL",@REL);
printCArray("ABS",@ABS);
printCArray("MSC",@MSC);
printCArray("LED",@LED);
printCArray("SND",@SND);
printCArray("REP",@REP);
printCArray("FF",@FF);
printCArray("PWR",@PWR);
printCArray("FF_STATUS",@FF_STATUS);

# print array of arrays
print("char **EVENTNAMES[",$#EV+1,"] = {");
for($i = 0; $i < $#EV; $i++)
{
	 # format nicely in sets of 6
	 if ( ($i+4)%6 == 5 ) { print("\n       "); }

	 # if the array element's data is null, print NULL
	 if ($EV[$i]) 
	 { 
		  $_ = $EV[$i];
		  m/EV_([A-Z_]+)/;
		  print("$1,");  
	 }
	 else { print("NULL,"); }
}
$_ = $EV[$#EV];
m/EV_([A-Z_]+)/;
print("$1\n };\n");


# print "EV: $#EV \n";
# print "SYN: $#SYN \n";
# print "KEY: $#KEY \n";
# print "REL: $#REL \n";
# print "ABS: $#ABS \n";
# print "MSC: $#MSC \n";
# print "LED: $#LED \n";
# print "SND: $#SND \n";
# print "REP: $#REP \n";
# print "FF: $#FF \n";
# #print "PWR: $#PWR \n";
# print "FF_STATUS: $#FF_STATUS \n";


close(INPUT_H);

