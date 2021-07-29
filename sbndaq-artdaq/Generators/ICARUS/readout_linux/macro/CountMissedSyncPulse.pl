#!/usr/bin/perl

use strict;
use warnings;

my (%misses,%average,%xpackets); 
my ($run,$key,$value,$xpacketcnt,$hi,$runlength);
my $readtime = 0;

open IN, "/var/log/crackle/user.log" or die $!;

while(<IN>) {
    next if ($_ !~ /^Jul 20/);
    if($_ =~ /STATE RunControlServer RUN_S/) { $readtime = 1; }
    elsif($_ =~ /State manager sent: SET RemainingTime (\d+)/) { if($readtime) { $runlength = $1; $readtime = 0; } }
    elsif($_ =~ /Found MySQL run summary entry for run: (\d{7})$/) { 
	if($run) { # && %misses) {
	    print "Run: $run Run length: $runlength\n";
	    print "X packets found: $xpacketcnt\n";
	    if(%xpackets) {
		foreach (sort {$a <=> $b} keys %xpackets) { 
		    #while(($key,$value) = each(%misses)) {
		    #print $key . ", " . $value . "\n";
		    print "USB $_ x packet count: $xpackets{$_}.\n";
		    #print "Average hi: $average{$_}.\n";
		}
	    }

	    if(%misses) {
		foreach (sort {$a <=> $b} keys %misses) { 
		    #while(($key,$value) = each(%misses)) {
		    #print $key . ", " . $value . "\n";
		    print "Module $_ missed $misses{$_} time stamps.\n";
		    #print "Average hi: $average{$_}.\n";
		}
	    }
	    
	}
	%misses = ();
	%average = ();
	%xpackets = ();
	$xpacketcnt = 0;
	$run = $1; 
	$runlength = 0;
    }
    elsif($_ =~ /Module (\d+) max clock count hi: (\d{5})/) {
	$misses{$1}++;
	#$average{$1} = ($average{$1}*($misses{$1}-1) + $2)/$misses{$1};
    }
    elsif($_ =~ /Found x packet in file.*_(\d{2})/) {
	$xpacketcnt++;
	$xpackets{$1}++;
    }
}

print "Run: $run Run length: $runlength\n";
print "X packets found: $xpacketcnt\n";
if(%xpackets) {
    foreach (sort {$a <=> $b} keys %xpackets) { 
	#while(($key,$value) = each(%misses)) {
	#print $key . ", " . $value . "\n";
	print "USB $_ x packet count: $xpackets{$_}.\n";
	#print "Average hi: $average{$_}.\n";
    }
}
if(%misses) {
    foreach (sort {$a <=> $b} keys %misses) { 
    #while(($key,$value) = each(%misses)) {
	print "Module $_ missed $misses{$_} time stamps.\n";
	#print "Average hi: $average{$_}.\n";
    }
}
