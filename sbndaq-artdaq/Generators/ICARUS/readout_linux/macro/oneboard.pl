use strict;
use warnings;
use CRTfunctions;
use usb_readout;
use CRT;

$run_length = 5; 		          #set runlength in seconds
$runlength = 5;
##############################################################################

$usb = 39;    		              # -1 = all
$usemultiplefile = "yes";
$filelength = 5;
$laserrate = 1000;


my $debug = 0;     # 1=debug mode, 0=mysql
my $pmt = shift;
my $mode = "mysql";

my $disk = "1";                # Disk number (1 or 2)
$DataPath = "CRTDAQ/DATA/";

$DataFolder = "/data${disk}/${DataPath}";
my $totalpmt = 1;
my $totusb = 1;
my $numbaselinetriggers = 1;

$onlinetable = "";


#load data for each pmt connected
#syntax loadpmt pmtnumber;
#loadpmtdata($usb,0,$debug,"off");

system qq | start_readout.sh "readout" "1" "1" "none"|;


#loadpmtdata($usb,$pmt,$debug,"off");

#loadpmtdata_auto($usb,$pmt,"5281",$onlinetable);

loadconfig("mysql",$usb,$pmt);
print("Done load config !\n");

#test_system_time($usb,$pmt); 

tarry 2.0; # how long it will take force trigger data

loadpmtdata_auto($usb,$pmt,"5182",$onlinetable);

initializeboard("auto",500);

starttakedata;

tarry $run_length;

stoptakedata;

check_rate("${DataFolder}/Run_$run_number/binary",$totalpmt,$totusb,0,500);


system qq| /usr/bin/perl -S baselines.pl "${DataFolder}/Run_${run_number}/binary/" "baseline_${usb}" "${pmt}"|;





#######test 1 ############################


generatecsv($DataFolder,$run_number,$usb,$pmt);


$average = getavg($pmt,3);                         # getavg($pmt,$col) returns the average of column $col in summary.csv (see below)                                    
#checkeff($pmt,"${DataFolder}/Run_${run_number}");         # writes statistics about x packets and timestamps to TimeStamps.txt in data folder                      
$rate = 64*$average/$elapsed_time;                  # Total rate = $num_channels*$avg_hits/$elapsed_time                               \
                                                                                                                                                
printf("PMT %3.0d:\t Rate:%6.0d\n",$pmt,${rate}); # Print info    

# Column definitions in summary.csv                                                                                                             
# ($col ==1 ) { $colname = "pulse height"; }                                                                                                    
# ($col ==2 ) { $colname = "pulse height sigma"; }                                                                                              
# ($col ==3 ) { $colname = "hits per channel"; }                                                                                                
# ($col ==4 ) { $colname = "hits per packet"; }                                                                                                 
# ($col ==5 ) { $colname = "number of photo-electrons"; }                                                                                       
# ($col ==6 ) { $colname = "gain"; }                                                                                                            
# ($col ==7 ) { $colname = "rate per channel"; }           



###########################################



#<STDIN>;





#generatecsv2($run_number,$usb,$pmt);


#plotdatamb2($usb, $pmt);

##
##should be called for each pmt board
#generatecsv2($usb, $pmt);

#plotdatamb2($usb, $pmt,5239);

#producentuple();

#checkeff($pmt);

#print "DONE ! Press enter to close this window.";
#<STDIN>;
