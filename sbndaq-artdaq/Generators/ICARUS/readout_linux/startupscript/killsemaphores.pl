#!/usr/bin/perl
use strict;
use warnings;

open IN, "ipcs -s |" or die $!;
while(<IN>) {
    if(/(^[0-9a-fx]+)\s+/) {
	my $tmp = hex($1);
	print "Destroying semaphore $tmp...\n";
	system "ipcrm -S $tmp";
    }
}
