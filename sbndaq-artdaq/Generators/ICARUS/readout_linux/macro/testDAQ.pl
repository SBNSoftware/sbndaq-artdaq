# Columbia University 8/20/10
# This utility script exercise the OV DAQ by taking baselines and data 
# for any  number of sequentially numbered pmt boards and triggerboxes.
# It can also generate rate statistics and print them to stdout

use lib $ENV{DCONLINE_PATH} . "/DCOV/readout/macro";

use strict;
use warnings;
use DCfunctions;
use DCFOV;
use usb_readout;

my $rate = 0;
my $temp_pmt;
my @dacs;
my $usball = -1;    # -1 = all

my @usb = (15,16,5,17,18,19);
my @pmtinuse = (1,2,3,4,5,6,7,8,9,10);
my $triggerusb = 28; 
my $triggerpmt = 62;

my ($k,$current_usb);

my $db_config_file = "DCDatabase.config";     
load_db_config($db_config_file);

$disk = 2;
$DataPath = "OVDebug/DATA/"; # Use OVDebug/DATA for tests


system qq|${online_path}\/DCOV\/readout\/script\/start_readout.sh readout ${DataPath} 1|;

tarry 2;

##############################################################################

$usemultiplefile = "yes";
$filelength = 20;
$laserrate = 1000;		
$DebugDAC = 650;
$onlinetable = "online400"; # online400: pulsing only 1,11,21,31,41 at 1msec
                             # online800: pulsing 1,11,21,31,41 at 1msec, other at 16ms
                             # online1200: pulsing everything at 1msec

$run_length = 3;	     # set runlength in seconds
my $debug = '';              # empty string = mysql mode, 'debug' = debug mode

# There are three ways of invoking loadconfig below
# 1. loadconfig(''): config is loaded for all boards from mysql according to $db_config_file and $onlinetable
# 2. loadconfig('debug',$usb,$pmt): load default values for this $pmt attached to this $usb (see DCFOV.pm for defaults)
# 3. loadconfig('debug',$usb,$pmt,$IsTrigBox): load default values for this trigbox identified by $pmt,$usb

##########################################################################
# Use this block with $debug = 'debug' to load default configs individually
#	for(my $i = 0; $i<=$#usb; $i++) {
#            for(my $j = 1; $j<=$#pmtinuse+1; $j++) {
#       	        $pmt = $j+10*$i;
#		loadconfig($debug,$usb[$i],$pmt);
#            }
#	}
#       loadconfig($debug,$triggerusb,$triggerpmt,1);
##########################################################################


        loadconfig($debug);  # Just one line if $debug='' 

# There are 2 ways of calling initializeboard: 
# 1. initializeboard("auto",$trigger_num,$pmtini,$pmtfin)
#    a. Automatically set the run number by incrementing the max run number found by sorting the file names in $DataPath
#    b. Generate $trigger_num baseline triggers
#    c. Take baselines and initialize boards sequentially numbered from $pmtini through $pmtfin. 
#         If debug mode $pmtfin - $pmtini + 1 must equal $totalpmt (number of boards whose config was loaded)
# 2. initializeboard("auto",$trigger_num)
#    a. Same as above
#    b. Same as above
#    c. Take baselines and initialize boards 1 through $totalpmt where $totalpmt is the number of pmt boards whose config have been loaded
#         If debug mode $pmtfin - $pmtini + 1 must equal $totalpmt (number of boards whose config was loaded)
        initializeboard("auto",50,1,70);  #RC, Trigger number, pmtini, pmtfin

# initializetriggerbox($boxini,$boxfin): same logic as (c) above but for trigger boxes
        initializetriggerbox();

# starttakedata($pmtini,$pmtfin,$boxini,$boxfin) same logic as (c) ahove for pmt boards and for trigger boxes
        starttakedata(1,70);
        tarry $run_length;
# stoptakedata($pmtini,$pmtfin,$boxini,$boxfin) same logic as (c) ahove for pmt boards and for trigger boxes
# Should be called with the same arguments as starttakedata for proper shutdown of data streams
        stoptakedata(1,70);
        system qq | du -sh /data${disk}/${DataPath}Run_${run}/binary|; # Print out total data volume

for(my $i=0; $i<$totalpmt; $i++) {
    if($i % 40 == 1) {
	print "usb: $DCFOV::pmttousb[$i]\t pmt: $DCFOV::pmttoboard[$i]\n";
    	    generatecsv($DataFolder,$run,$DCFOV::pmttousb[$i], $DCFOV::pmttoboard[$i]);       # Generate summary.csv file for $usb, $pmt in $DataFolder, Run $run
    }
	    #$average = getavg($pmt,3);	                        # getavg($pmt,$col) returns the average of column $col in summary.csv (see below)
            #checkeff($pmt);                                     # writes statistics about x packets and timestamps to TimeStamps.txt in data folder
	    #$rate = 64*$average/$elapsed_time;                  # Total rate = $num_channels*$avg_hits/$elapsed_time
	    #printf("PMT %3.0d:\t Rate:%6.0d\n",${pmt},${rate}); # Print info
}

generatesummaryplot($DataFolder,$run);

#for(my $i = 0; $i<=$#usb; $i++) {
#    for(my $j = 0; $j<=0; $j++) {
#	    $rate = 0;
#	    $pmt = $j+10*$i;
#	    generatecsv($DataFolder,$run,$usb[$i], $pmt);       # Generate summary.csv file for $usb, $pmt in $DataFolder, Run $run
#	    $average = getavg($pmt,3);	                        # getavg($pmt,$col) returns the average of column $col in summary.csv (see below)
#            checkeff($pmt);                                     # writes statistics about x packets and timestamps to TimeStamps.txt in data folder
#	    $rate = 64*$average/$elapsed_time;                  # Total rate = $num_channels*$avg_hits/$elapsed_time
#	    printf("PMT %3.0d:\t Rate:%6.0d\n",${pmt},${rate}); # Print info
#            <STDIN>;
#    }
#}

# Column definitions in summary.csv
# ($col ==1 ) { $colname = "pulse height"; }
# ($col ==2 ) { $colname = "pulse height sigma"; }
# ($col ==3 ) { $colname = "hits per channel"; }
# ($col ==4 ) { $colname = "hits per packet"; }
# ($col ==5 ) { $colname = "number of photo-electrons"; }
# ($col ==6 ) { $colname = "gain"; }
# ($col ==7 ) { $colname = "rate per channel"; }


