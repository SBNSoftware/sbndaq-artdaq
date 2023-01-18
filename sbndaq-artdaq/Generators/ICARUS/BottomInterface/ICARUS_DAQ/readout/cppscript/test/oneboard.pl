use strict;
use warnings;
#use DCfunctions;
use usb_readout;
use CRT;

$run_length = 30; 		          #set runlength in seconds
$runlength = 5;
##############################################################################

$usb = 33;    		              # -1 = all
$usemultiplefile = "yes";
$filelength = 5;
$laserrate = 1000;


my $debug = 0;     # 1=debug mode, 0=mysql
my $pmt = 3;
my $mode = "debug";

my $disk = "1";                # Disk number (1 or 2)
my $DataPath = "OVDAQ/DATA/";

my $DataFolder = "../data${disk}/${DataPath}";
my $totalpmt = 1;
my $totusb = 1;
my $numbaselinetriggers = 1;

my $online_path = "/e/h.0/localdev";
my $DataPath2 = "/e/h.0/localdev/readout/DCOV";

#load data for each pmt connected
#syntax loadpmt pmtnumber;
#loadpmtdata($usb,0,$debug,"off");



system qq | ${online_path}\/readout\/script\/start_readout.sh "readout" "1" "1" "none"|;

<STDIN>;

#loadpmtdata($usb,$pmt,$debug,"off");


#loadconfig("mysql",$usb,$pmt);
#print("Done load config !\n");

test_system_time($usb,$pmt); 

<STDIN>;

#check_system_time;

#<STDIN>;

initializeboard("auto",500);

<STDIN>;

starttakedata;

tarry $run_length;

stoptakedata;

check_rate("${DataFolder}/Run_$run_number/binary",$totalpmt,$totusb,0,500);

print "Done Check rate\n";

system qq| /usr/bin/perl baselines.pl "${DataFolder}/Run_${run_number}/binary/" "baseline_${usb}" "${pmt}"|;

#######check_system_time;


generatecsv2($run_number,$usb,$pmt);



plotdatamb2($usb, $pmt);

##
##should be called for each pmt board
#generatecsv2($usb, $pmt);

#plotdatamb2($usb, $pmt,5239);

#producentuple();

#checkeff($pmt);

print "DONE ! Press enter to close this window.";
<STDIN>;
