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
while (<>) {
  if (m|[Ee]ntering directory\b.*\/moocow\/extended(?!\/)|) { #/
    $ismoo=1;
    next;
  }
  next if (!$ismoo);

  if (m/^\(moocow/i || m/MOOCOW_BUILD_VERSION/) {
    print "DEBUG: $_";
  }
  elsif (m|[Ee]ntering directory\b.*\/moocow\/([^\/]*)$|) {
    $extdir = $1;
    chomp($extdir);
    $extdir =~ s/\'$//;
    #print "DIR: $extdir\n";
  }
  elsif (m|install(?:.*?)\s+(\S+)\s+(?:\S*)/moocow/extended/build.moo/(.*)$|) {
    ($file,$instdir) = ($1,$2);
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
  elsif (/[Ll]eaving directory\b.*\/moocow\/extended(?!\/)/) { $ismoo=0; } #/
}

##-- summarize
%class2name = (EXT=>'externals', PAT=>'patches', DOC=>'docs', UNK=>'unknown');
@summary =
  (sprintf("%-32s: ", $arch),
   join(', ',
	(sprintf("%2d packages", scalar(keys(%packages)))),
	(map { sprintf("%2d", ($class2n{$_}||0))." ".($class2name{$_}||$_) } qw(EXT PAT DOC UNK)),
       ),
   "\n",
  );
print STDERR @summary;
print @summary;
