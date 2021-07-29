use strict;
use warnings;
use CRTfunctions;
use usb_readout;
use CRT;

$run_length = 5;                                 #set runlength in seconds

$runlength = 2;

$usb = 39;                    # -1 = all
$usemultiplefile = "yes";
$filelength = 5;
$laserrate = 1000;

$onlinetable = "online400"; #"online_force_trigger"; #Changed by Andrew Olivier on 7/10/2018 to use the table I prepared for the downstream 
                                 #CRTs.  







#my @pmtnumbers = (5199);
#my @pmtboardnum = (3);

my @pmtnumbers = (5199,5138,5264,5188,5200,5228,5220);
my @pmtboardnum = (0,1,2,3,4,5,6);

my $debug = 0;       #1=debug mode, 0=mysql

my $mode = "debug";

my $disk = "1";          # Disk number (1 or 2)
my $DataPath = "CRTDAQ/DATA/";

my $DataFolder = "/data${disk}/${DataPath}";

###########################################################
my $totalpmt = 7; #7;
my $totusb = 1;
###########################################################


my $numbaselinetriggers = 1;

my $totusbs = 1;

system qq | /home/bcrt/readout_linux/script/start_readout.sh "readout" "1" "1" "none"|;


#loadconfig($debug);  ####need to readout everything from database to fill structure



#for ( my $i = 0; $i < $totalpmt; $i++){
    #test_system_time($usb,$pmtboardnum[$i]);
    #tarry $runlength;
#    check_system_time;
    #tarry 0.1;
#}

#tarry 1.0;

#print"System time check finished. Press ENTER to continue\n";
#<STDIN>;

#load mysql for each pmt connected
for ( my $i = 0; $i < $totalpmt; $i++){
  loadpmtdata_auto($usb, $pmtboardnum[$i], $pmtnumbers[$i],"online400");
#  loadpmtdata_auto($usb, $pmtboardnum[$i], $pmtnumbers[$i],"online_normal_running");
}

initializeboard("auto",500);

starttakedata;

tarry $run_length;

stoptakedata;

check_rate("${DataFolder}/Run_$run_number/binary",$totalpmt,$totusb,0,500);


for(my $i=0; $i<$totalpmt; $i++) {

        print "usb: $CRT::pmttousb[$i]\t pmt: $CRT::pmttoboard[$i]\n";

	<STDIN>;

	system qq| /usr/bin/perl baselines.pl "${DataFolder}/Run_${run_number}/binary/" "baseline_{$pmttousb[$i]}" "1"|;

	generatecsv($DataFolder,$run,$pmttousb[$i], $pmttoboard[$i]);       # Generate summary.csv file for $usb, $pmt in $DataFolder, Run $run                                                                  
#    }
        tarry 0.5;    
        $average = getavg($pmttoboard[$i],3);                         # getavg($pmt,$col) returns the average of column $col in summary.csv (see below)                                                                                  
        checkeff($pmttoboard[$i],"${DataFolder}/Run_${run_number}");         # writes statistics about x packets and timestamps to TimeStamps.txt in data folder                                                                               
        $rate = 64*$average/$elapsed_time;                  # Total rate = $num_channels*$avg_hits/$elapsed_time                                                                                                              
        printf("PMT %3.0d:\t Rate:%6.0d\n",$pmttoboard[$i],${rate}); # Print info                                     
}

generatesummaryplot($DataFolder,$run);



print "DONE ! Press enter to close this window.\n";
<STDIN>;

# Column definitions in summary.csv                                                                               
# ($col ==1 ) { $colname = "pulse height"; }                                                                      
# ($col ==2 ) { $colname = "pulse height sigma"; }                                                                
# ($col ==3 ) { $colname = "hits per channel"; }                                                                  
# ($col ==4 ) { $colname = "hits per packet"; }                                                                   
# ($col ==5 ) { $colname = "number of photo-electrons"; }                                                         
# ($col ==6 ) { $colname = "gain"; }                                                                              
# ($col ==7 ) { $colname = "rate per channel"; }                         





