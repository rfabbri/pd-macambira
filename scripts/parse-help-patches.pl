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

my $libraryName = "";
my $objectclassName = "";
my $fileName = "";

my $pageName = "";
my $abbreviationName = "";
my $descriptionName = "";
my $categoryName = "";
my $datatypeName = "";

#------------------------------------------------------------------------------#
# TRANSLATION VARIABLES
#------------------------------------------------------------------------------#

# wikipedia terms
my $stub = "";
my $template = "";
my $category = "";
my $infobox = "";
my $topLevel = "";  # 

# pdpedia terms
my $objectclass = "";

# page headers
my $inlets = "";
my $outlets = "";
my $arguments = "";
my $messages = "";

# infobox
my $name = "";
my $description = "";
my $abbreviation = "";
my $library = "";
my $author = "";
my $developer = "";
my $releaseVersion = "";
my $releaseDate = "";
my $dependencies = "";
my $license = "";
my $website = "";
my $programmingLanguage = "";
my $platform = "";
my $operatingSystem = "";
my $language = "";
my $dataType = "";
my $distributions = "";
my $status = "";


#------------------------------------------------------------------------------#
# Portuguese

# wikipedia terms
$stub = "Esboço";
$template = "Template";
$category = "Categoria";
$infobox = "Infobox";

# pdpedia terms
$objectclass = "Classe do objecto";

# page headers
$inlets = "Entradas";
$outlets = "Saídas";
$arguments = "Argumentos";
$messages = "Mensagens";

# infobox
$name = "Nome";
$description = "Descrição";
$abbreviation = "Abreviatura";
$library = "Biblioteca";
$author = "Autor";
$developer = "Programador";
$releaseVersion = "Versão actual";
$releaseDate = "Data de lançamento";
$dependencies = "Dependências";
$license = "Licença";
$website = "Website";
$programmingLanguage = "Linguagem de programação";
$platform = "Plataforma";
$operatingSystem = "Sistema operativo";
$language = "Linguagem";
$dataType = "Tipo de dado";
$distributions = "Distribuições";
$status = "Estado";

#------------------------------------------------------------------------------#
# PARSE CSV
#------------------------------------------------------------------------------#
my $csvfile = '/Users/hans/Desktop/TODO/wiki_files_hacked/objectlist.csv';
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

foreach (`/sw/bin/find /Users/hans/Desktop/TODO/wiki_files_hacked/5.reference/ -type f -name '*.pd'`) {
  chop;
  $fileName = "";
  if( (m|.*/5\.reference/([a-zA-Z0-9_-]+)/(.+)-help\.pd|) ||  (m|.*/5\.reference/([a-zA-Z0-9_-]+)/(.+)\.pd|) ) {
#	 print("$1 , $2\t");
	 if( $1 eq 'zflatspace' ) { $libraryName = "flatspace";}
	 else {$libraryName = lc($1); }
	 $objectclassName = $2;
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
	 
	 my $myColumns = $csvhash{$libraryName}{$objectclassName};
	 my @myColumns;
	 if($myColumns) { @myColumns = split(',', $myColumns); }
	 if($myColumns[2]) { $abbreviationName = $myColumns[2]; }	 if( $classnames{$objectclassName} ) {
		$pageName = "${objectclassName}_(${libraryName})";
	 } else {
		$pageName = "${objectclassName}";
	 }

	 
	 mkdir($libraryName);
	 if( $classnames{$objectclassName} ) {
		$pageName = "${objectclassName}_(${libraryName})";
	 } else {
		$pageName = "${objectclassName}";
	 }
	 open(OBJECTCLASS, ">$libraryName/${pageName}.txt");
	 print(OBJECTCLASS "{{Infobox $objectclass\n");
	 print(OBJECTCLASS "| $name                   = $objectclassName\n");
	 if($abbreviationName) {
		print(OBJECTCLASS "| $abbreviation           = $abbreviationName\n");}
	 print(OBJECTCLASS "| $description            = \n");
	 print(OBJECTCLASS "| $dataType               = \n");
	 print(OBJECTCLASS "| $library                = [[$libraryName]]\n");
	 print(OBJECTCLASS "| $author                 = {{$libraryName " . lc(${author}) . "}}\n");
	 print(OBJECTCLASS "| $license                = {{$libraryName " . lc(${license}) . "}}\n");
	 print(OBJECTCLASS "| $status                 = {{$libraryName " . lc(${status}) . "}}\n");
	 print(OBJECTCLASS "| $website                = {{$libraryName " . lc(${website}) . "}}\n");
	 print(OBJECTCLASS "| $releaseVersion         = {{$libraryName " . lc(${releaseVersion}) . "}}\n");
	 print(OBJECTCLASS "| $releaseDate            = {{$libraryName " . lc(${releaseDate}) . "}}\n");
	 print(OBJECTCLASS "| $distributions          = {{$libraryName " . lc(${distributions}) . "}}\n");
	 print(OBJECTCLASS "| $platform               = [[GNU/Linux]], [[Mac OS X]], [[Windows]]\n");
	 print(OBJECTCLASS "}}\n\n");
#	 print(OBJECTCLASS "\n$printText\n\n\n");
	 print(OBJECTCLASS "==$inlets==\n\n\n");
	 print(OBJECTCLASS "==$outlets==\n\n\n");
	 print(OBJECTCLASS "==$arguments==\n\n\n");
	 print(OBJECTCLASS "==$messages==\n\n\n");
	 print(OBJECTCLASS "{{${objectclass}-${stub}}} \n\n");
	 print(OBJECTCLASS "[[$category:$objectclass]]\n");
	 print(OBJECTCLASS "[[$category:$libraryName]]\n");
#	 if($category) {
#		print(OBJECTCLASS "[[$category:$categoryName]]\n");
#	 }
	 print(OBJECTCLASS "\n\n");
	 print(OBJECTCLASS "[[en:$pageName]]\n");
	 print(OBJECTCLASS "\n\n");
	 close(OBJECTCLASS);

	 $classnames{$objectclassName} = 1;
  }
}
  
