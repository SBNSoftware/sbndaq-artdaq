use strict;
use warnings;
use CRTfunctions;
use usb_readout;
use CRT; 

$run_length = 60;                                 #set runlength in seconds

$runlength = 2;

$usb = 22;                    # -1 = all
$usemultiplefile = "yes";
$filelength = 5;
$laserrate = 1000;

$onlinetable = "crt_icarus_v0"; #"online_force_trigger"; #Changed by Andrew Olivier on 7/10/2018 to use the table I prepared for the downstream 
                                 #CRTs.  







my @pmtnumbers = ( ); #filled by loadconfig()
my @pmtboardnum = ( ); #filled by loadconfig()

my $debug = 0;       #1=debug mode, 0=mysql
my $mode = 'mysql';

my $disk = "1";          # Disk number (1 or 2)
my $DataPath = "CRTDAQ/DATA/";

my $DataFolder = "/data${disk}/${DataPath}";

###########################################################
my $totalpmt = 7; #7; #Filled by loadconfig().  Will print a seemingly-benign error message later if this doesn't happen to be the 
                  #number of rows in the MySQL table.  
                  #TODO: I think totalpmt is a local variable, so it can't be filled by loadconfig!
my $totusb = 2;
###########################################################


my $numbaselinetriggers = 1;

$CRT::totusbs = 2;

system qq | start_readout.sh "readout" "1" "1" "none"|;


#TODO: loadconfig isn't loading any PMTs.  
loadconfig($debug);  ####need to readout everything from database to fill structure

#Print PMTs configured for debugging
foreach my $number (@CRT::pmtnumbers)
{
  print "Using PMT $number\n";
}

if(!scalar @CRT::pmtnumbers) 
{
  print "No PMTs configured for readout in sevenboard_downstream.pl!\n";
}

print "After loadconfig, totalpmt is $totalpmt.  CRT::totalpmt is $CRT::totalpmt\n";

#TODO: test_system_time calls loadconfig in debug mode for this PMT!  I want to read from MySQL instead.  
#for ( my $i = 0; $i < $CRT::totalpmt; $i++){
#    test_system_time($usb,$CRT::pmtboardnum[$i]);
#    tarry $runlength;
##    check_system_time;
#    tarry 0.1;
#}

#For debugging, print out everything in pmttousb
foreach $pmt (keys @CRT::pmttousb)
{
  print "pmt: $pmt, usb: $CRT::pmttousb[$pmt]\n";
}

tarry 1.0;

initializeboard("auto",500); #500 is number of baselines pulled off of?

starttakedata;

tarry $run_length;

stoptakedata;

check_rate("${DataFolder}/Run_$run_number/binary",$CRT::totalpmt,$CRT::totusb,0,500);

for(my $i=1; $i<=$CRT::totalpmt; $i++) { #Change by Andrew Olivier on 7/10/2018: In CRT::loadconfig(), totalpmt is incremented BEFORE each time that 
                                    #pmttousb and pmttoboard are set.  So, the key=0 entry in each of these maps is never set.  This loop 
                                    #should loop over all PMTs in those maps, so its' indexing should start at 1 to match loadconfig().  
                                    #Otherwise, I am getting an error about an unitialized value that leads to other bad things.   

        print "usb: $CRT::pmttousb[$i]\t pmt: $CRT::pmttoboard[$i]\n";

        #TODO: This seems to process baselines once for each PMT instead of once for each USB.  
        print "About to look for baseline file: ${DataFolder}/Run_${run_number}/binary/baseline_$pmttousb[$i]\n";
	system qq| /usr/bin/perl -S baselines.pl "${DataFolder}/Run_${run_number}/binary/" "baseline_$pmttousb[$i]" "1"|;

	generatecsv($DataFolder,$run,$pmttousb[$i], $pmttoboard[$i]);       # Generate summary.csv file for $usb, $pmt in $DataFolder, Run $run                                                                  
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





