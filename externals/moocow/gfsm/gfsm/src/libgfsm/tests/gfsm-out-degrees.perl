#!/usr/bin/perl -w

use Gfsm;

$fsmfile = @ARGV ? shift : '-';
$fsm = Gfsm::Automaton->new();
die("$0: load failed for '$fsmfile': $!") if (!$fsm->load($fsmfile));

foreach $qid (0..($fsm->n_states-1)) {
  print $qid, "\t", $fsm->out_degree($qid), "\n";
}
