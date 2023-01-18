#!/usr/bin/perl

use strict;
use warnings;

#my $path = shift;
my $path = "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA";
my $loop = "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/ROOTntuple/Loop.pl";
my $usb = shift;
#print "$ENV{'DCONLINE_PATH'}\n";
rerun:
print "Which run (e.g. Run_20081231_0) would you like to process (type 'ls' for a list of runs): "; 
chomp(my $run = <STDIN>);
if($run eq "") {goto rerun;}
elsif($run eq "ls") {
    system qq|ls ${path}|;
    goto rerun;
}

#my $folder = "${path}/${run}/USB_${usb}";
#my $folder = "/e/h.0/localdev/readout/data1/OVDAQ/DATA/${run}/USB_${usb}";
#my $folder = "/e/h.0/localdev/readout/data1/OVDAQ/DATA/${run}/binary";
my $folder = "${path}/${run}/binary";

my ($temp,@files);

if(-d "$folder") {
    print "$folder found.\n\n";
    $temp = "$folder/*.root"; # Run already being processed
    push(@files,<${temp}>); # curly braces required for some reason
    if($#files>=0) { 
	print "WARNING! Found " . ($#files+1) . " root files in $folder.\n\n";
	print "Do you want to delete all these files and reprocess ${run}? (Y/N) ";
	chomp(my $ans=<STDIN>);
	if($ans eq "Y" or $ans eq "y") { 
	    system qq|rm -f ${folder}/*.root |; #${folder}/Timestamps.txt ${folder}/Xpackets.txt ${folder}/${run}.log|;
	    print "Processing data in background.\n";
	    print "Creating log file ${folder}/${run}.log...\n";
	    #system qq|nohup /usr/bin/perl $ENV{'DCONLINE_PATH'}/DCOV/readout_winxp/ROOTntuple/Loop.pl ${folder} > ${folder}/${run}.log &|;
	    system qq|nohup /usr/bin/perl ${loop} ${folder} > ${folder}/${run}.log &|;
	}
    }
    else {
	print "Processing data in background.\n";
	print "Creating log file ${folder}/${run}.log...\n";
	#system qq|nohup /usr/bin/perl $ENV{'DCONLINE_PATH'}/DCOV/readout_winxp/ROOTntuple/Loop.pl ${folder} > ${folder}/${run}.log &|;
	system qq|nohup /usr/bin/perl ${loop} ${folder} > ${folder}/${run}.log &|;
    }
}
else {
    print "$folder not found. Exiting...\n";
}
