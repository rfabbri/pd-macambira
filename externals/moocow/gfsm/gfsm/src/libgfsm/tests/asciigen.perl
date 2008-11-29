#!/usr/bin/perl -w

@ascii = (ord('A')..ord('Z'),
	  ord('a')..ord('z'),
	  ord('!')..ord('/'),
	  ord('0')..ord('9'),
	  ord(':')..ord('?'));

foreach $c (@ascii) {
  print chr($c);
}
print "\n";

foreach $c (161..255) {
  printf("\\%o", $c);
}
print "\n";
