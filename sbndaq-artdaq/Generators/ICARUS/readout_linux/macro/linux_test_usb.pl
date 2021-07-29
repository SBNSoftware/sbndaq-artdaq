use strict;
use warnings;
use usb_readout;
use DCfunctions;  #Only necessary of you wish to use the checkeff() function below

#init_readout "127.0.0.1";       # or "localhost"

# print "Make sure you have a loopback setup.\n";
# print "Enter USB number: ";
# my $usb;
# chomp($usb = <STDIN>);

#Specify path information
my $data_disk = "1";                # Disk number (1 or 2)
my $DataPath = "OVDAQ/DATA/";       # Path to data on chosen disk
my $run_number = "0000100";         # Run number (set so as not to interfere with data taking)

my $runpath = "../data" . $data_disk . "/" . $DataPath . "Run_" . $run_number . "/";
my $binarypath = $runpath . "binary";
my $decodedpath = $runpath . "decoded";

#Set path to readout macros
#my $dcmacropath = $ENV{DCONLINE_PATH} . "/DCOV/readout/macro";
my $dcmacropath = "../macro";

#Delete any previous data in the specified run folder
#BEWARE if $DataPath is set to OVDAQ/DATA/
print "About to execute: \n rm " . $binarypath . "/* \n";
print "rm " . $decodedpath . "/* \n";
print "Press any key to continue, or CTRL+C to stop";
<STDIN>;
system "rm " . $binarypath . "/*";
system "rm " . $decodedpath . "/*";


tarry 2;

my $timelength;
my $usb = 33;

print "Starting loop\n";
for(my $i=1;$i<=1;$i++){
	#usb $usb, 0, 0;     				# turn off everything
        usb $usb, 0;     				        # turn off everything
	$timelength = 1; #2*$i; 
	print "Opening file...\n";
	set_data_path($DataPath);                       # SET DATA PATH
        set_data_disk($data_disk);                              # SET THE DISK NUMBER
        set_run_number($run_number);                       # SET RUN NUMBER


        set_inhibit_usb $usb, -3;                       # set up file structure
	usb $usb, 5096;                     # max speed = 0; #was 1024
        #usb 0, 2;
	#tarry 5;
	#usb 0, 0;
	#tarry 0.5;
        #set_inhibit_usb $usb, -1;                       #inhibit data taking
        #tarry 0.5;

	set_inhibit_usb $usb, 0;                        # start the data taking 
        usb $usb, 2048;
	usb 0, 2;        

	tarry $timelength;
	usb $usb, 0;                                       # turn off far debug
	tarry 0.5;                                      # wait for late packets

	set_inhibit_usb $usb, -1;                       # stop data taking

	tarry 0.5;

	###################################################
	#Check data for the proper number of packets. 
	#This requires first decoding the data.
	#
	###################
	print "Decoding binary data...\n";
	my $tempfile;
	my $outfilename = ${decodedpath}."/signal.dec";

	my @files = <${binarypath}/1*_${usb}>; 
	foreach $tempfile (@files) {
	    system qq|/usr/bin/perl ${dcmacropath}/decode.pl ${tempfile} > ${outfilename}|;
	}
	
	print "Checking file timelength=$timelength...\n";
	#system qq|/usr/bin/perl ${dcmacropath}/check.pl ${outfilename}|;
	
	#Print and store data for the breakdown of different packet types.
	#To use this you must include DCfucnitons at the top of this page.
	#Note: argument is an arbitrary pmt board number.
	#
	checkeff(1,$decodedpath);

	
	##################################################


	print "Hit any key to continue...";
	<STDIN>;
}
#print "Hit any key...";
#<STDIN>;
