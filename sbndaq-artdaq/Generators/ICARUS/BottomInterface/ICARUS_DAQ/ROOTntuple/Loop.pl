#!/usr/bin/perl

use strict;
use warnings;
my $folder = shift;
my $binfile;
my $outfile;
my $ext="/home/nfs/icarus/DAQ_DevAreas/DAQ_17Feb2023_MM/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/ROOTntuple";

my @files = <${folder}/*_39>;
print "Found " . ($#files+1) . " files.\n";    
foreach $binfile (@files){
    $outfile = "${binfile}.root";
#   $outfile =~ s|bin$|root|g;
    print "Building NTuple...\n";
    system qq|${ext}/ChipProgram $binfile $outfile|;
}
