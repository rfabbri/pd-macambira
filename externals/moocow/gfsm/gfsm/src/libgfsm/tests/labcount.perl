#!/usr/bin/perl -w

use Gfsm;

if (!@ARGV) {
  print STDERR "Usage: $0 LABFILE [DATA_FILE(s)...]\n";
  exit 1;
}

$labfile = shift;
$labs = Gfsm::Alphabet->new();
$labs->load($labfile) or die("$0: load failed for labels file '$labfile': $!");
$sym2id = $labs->asHash;

##-- read data
%labf   = qw();
$ftotal = 0;
while (defined($line=<>)) {
  chomp($line);
  @labs  = grep {defined($_)} @$sym2id{split(//,$line)};
  $ftotal += scalar(@labs);
  foreach (@labs) { ++$labf{$_}; }
}

##-- write data vector
#print map { pack('d', (defined($_) ? $_ : 0)/$ftotal) } @labf;

print map {pack('Sd',$_,$labf{$_}/$ftotal)} sort(keys(%labf));
