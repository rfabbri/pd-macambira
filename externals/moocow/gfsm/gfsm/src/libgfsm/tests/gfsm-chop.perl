#!/usr/bin/perl -w

use Gfsm;

our @want_states =
  (
   5,
   4,
   11,
   98,
   5257,
   45623,
   290132,
   0
  );

push(@ARGV,'-') if (!@ARGV);

$fsmfile = shift(@ARGV);
our $fsm = Gfsm::Automaton->new();
$fsm->load($fsmfile) or die("$0: load failed for fsm file '$fsmfile': $!");

##-- chop it
my %q2i       = map { $want_states[$_]=>$_ } (0..$#want_states);
my $qid_dummy = scalar(@want_states);
$fsm2 = $fsm->shadow;
$ai=Gfsm::ArcIter->new;

foreach $qid_dst (0..$#want_states) {
  $fsm2->ensure_state($qid_dst);
  $qid_src = $want_states[$qid_dst];
  for ($ai->open($fsm,$qid_src); $ai->ok; $ai->next) {
    $fsm2->add_arc($qid_dst, $qid_dummy, $ai->lower, $ai->upper, $ai->weight);
  }
  $fsm2->add_arc($qid_dummy,$qid_dst,0,0,0);
}
$fsm2->root($qid_dummy);
$fsm2->final_weight($qid_dummy,0);

$fsm2->save('-');
