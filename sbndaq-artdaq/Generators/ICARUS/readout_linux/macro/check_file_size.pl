#!/usr/bin/perl
use strict;
use warnings;

my $filesize;
my %readfilesize;
my $usbread;

my $i=0;
my $diff;
my $disk;
my $run;
my $lasttimestamp;
my $timestamp_i = 0;
my $timestamp_i_old = 0;
my ($sth,$drh,$Data_Path,$readfile);
my $file_run;
my $file_run1;
my $max = 0;
my @files_run;
my @files_run1;


my $DataFolder="/disk1/data1/CRTDAQ/DATA";

@files_run = <${DataFolder}/Run_*>;

foreach $file_run (@files_run) {
  if($file_run =~ /Run_(\d+)$/) {
    $max = $1 if $1 > $max;
  }
}  

$run = $max;

#find last time stamp

$max = 0;

@files_run1 = <${DataFolder}/Run_$run/binary/*_*>;

foreach $file_run1 (@files_run1) {
  if($file_run1 =~ /(\d+)_(\d+)$/) {
    $max = $1 if $1 > $max;
  }
}  

$lasttimestamp = $max;

#print "Last time stamp: $lasttimestamp \n ";

$Data_Path= $DataFolder . "/Run_$run/binary";

$readfile = 0;

if (-e $Data_Path ) {
#    print "Found folder : $Data_Path \n";
    open IN, "cd $Data_Path ; ls -ltr --time-style=long-iso ${lasttimestamp}*_*|";
    while(<IN>) {
        if(/ (\S+) \d+-\d+-\d+ \d+:\d+ (\d+)_(\d+)/) { 
	    $readfile = 1;
	    $filesize=$1;
	    $readfilesize{$3} += $filesize;
	    $timestamp_i = $2;
	    if($timestamp_i != $timestamp_i_old) {
		$i++;
		$timestamp_i_old = $timestamp_i;
	    }	    
	  }
    }
    close IN;
}
else {
    print "Unable to find the Path: $Data_Path \n";
    exit 1;
}

if($readfile == 0) { exit 1; } # this help with latency of NFS or disk

my $maxdim = 0;
my $mindim = 1000000000;

foreach $usbread (keys(%readfilesize)){
        if($readfilesize{$usbread} == 0) { print "USB $usbread has 0 file size!\n"; } #TODO: Print timestamp at which filesize is 0
	if($readfilesize{$usbread} > $maxdim) { $maxdim = $readfilesize{$usbread}; }
	if($readfilesize{$usbread} < $mindim) { $mindim = $readfilesize{$usbread}; }
}

if($i) {
    $diff = ($maxdim - $mindim)/$i;
}
else {

    print "Zero file to divide to\n";
}

if($diff > 15000000) { 
	print "We got a problem with the dimension of DAQ files, difference between file dimension is $diff \n"; 
}
