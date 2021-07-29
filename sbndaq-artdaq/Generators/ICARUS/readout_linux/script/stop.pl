#!/usr/bin/perl

use strict;
use warnings;

my @marks = (
    "\\.\\/readout"
);


print "Started stop.pl.  Looking for ./readout processes to kill...\n";
open IN, "ps -eo pid,cmd |" or die $!;
while(<IN>) {
    if(/^\s*(\d+)\s+(.*)/) {
        my $pid = $1;
        my $cmd = $2;
        my $kill_it = 0;
        for (@marks) { if ($cmd =~ /$_/) { $kill_it = 1; } }
        if ($kill_it) {
            print "Killing process $pid...\n"; 
            kill 9, $pid or print "Error killing $pid: $!\n";
        }
    }
}

open IN, "ipcs -q |" or die $!;
while(<IN>) {
    if(/^([0-9a-fx]+)\s+/) {
        my $tmp = hex($1);
        #print "Destroying queue $tmp...\n";
        system "ipcrm -Q $tmp";
    }
}

