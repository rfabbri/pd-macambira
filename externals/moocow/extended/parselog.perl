#!/usr/bin/perl -w

use File::Basename;

##-- parse filename?
if (@ARGV) {
  $file = $ARGV[0];
  $base = File::Basename::basename($file);
  $base =~ s/\Q_pd-extended_run-automated-builder.txt\E$//;
  $base =~ s/^[^a-zA-Z]*//;
  $arch = $base;
} else {
  $arch = 'unknown';
}
$ismoo=0;
%class2n =qw();
%packages =qw();
$nfailed = 0;
while (<>) {
  if (
      m|[Ee]ntering directory\b.*\/moocow\/extended(?!\/)|
      ||
      m|^make -C .*\/moocow\/extended\s|
     )
    {
      $ismoo = 1;
      next;
    }
  next if (!$ismoo);

  if (m/MOOCOW_BUILD_VERSION/ || m/^\(moocow/i) {
    $ismoo = 1;
    print "DEBUG: $_";
    $nfailed++ if ($_ =~ /sub-target failed/);
  }
  elsif (
	 m|[Ee]ntering directory\b.*\/moocow\/([^\/]*)\s|
	 ||
	 m|^\(cd \.\./(\S+);|
	)
    {
      $extdir = $1;
      chomp($extdir);
      $extdir =~ s/\'$//;
      #print "DIR: $extdir\n";
    }
  elsif (m|install(.*?)\s+(\S+)\s+(?:\S*)/moocow/extended/build.moo/(.*)$|) {
    ($opts,$file,$instdir) = ($1,$2,$3);
    next if ($file eq '-d' || $opts =~ /\s\-d\b/);
    $file    =~ s/[\'\"]//g;
    $instdir =~ s/[\'\"]//g;
    $instdir =~ s/$file$//;
    if ($file =~ /^(.*)(\.[^\.]*)$/) {
      ($base,$ext) = ($1,$2);
    } else {
      ($base,$ext) = ($file,'');
    }
    $class = 'UNK';
    if ($instdir =~ m|\bdoc/5\.reference\b|) {
      $class = 'DOC';
    }
    elsif ($ext eq '.pd' && $instdir =~ m/\b(?:externs|extra)\b/) {
      $class = 'PAT';
    }
    elsif ($instdir =~ m/\b(?:externs|extra)\b/) {
      $class = 'EXT';
    }
    $class2n{$class}++;
    $packages{$extdir}=1;
    print sprintf("INSTALL %-3s %10s %-20s %-12s %s\n", $class, $extdir, $base, $ext, $instdir);
  }
  elsif (
	 m|[Ll]eaving directory\b.*\/moocow\/extended(?!\/)|
	 ||
	 m|^make -C (?!.*\/moocow\/)|
	)
    {
      $ismoo=0;
    } #/
}

##-- summarize
%class2name = (EXT=>'externals', PAT=>'patches', DOC=>'docs', UNK=>'unknown');
@summary =
  (sprintf("%-32s: ", $arch),
   join(', ',
	(sprintf("%2d packages, %2d failed", scalar(keys(%packages)), $nfailed)),
	(map { sprintf("%2d", ($class2n{$_}||0))." ".($class2name{$_}||$_) } qw(EXT PAT DOC UNK)),
       ),
   "\n",
  );
print STDERR @summary;
print @summary;
