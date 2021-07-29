#!/usr/bin/perl

#allboard.pl is the script that Matt Strait's artdaq process will run to load ProtoDUNE-SP Cosmic Ray Tagger (CRT) 
#configurations, start taking data, and write data as it is taken to a well-defined directory.  Adapted from 
#sevenboard.pl so that I have a starting point that works.  
#
#Usage: 
#allboard.pl <configuration table name>
#
#TODO: Does Matt want this script to just start "the DAQ" and let it keep running?  If so, maybe I could remove the 
#      function that stops data taking and Matt could call it himself. 

use strict;
use warnings;
use CRTfunctions;
use usb_readout;
use CRT; 

$run_length = 60;                                 #set runlength in seconds

$runlength = 2; #Not used locally.  Global variable, so potentially used by any other perl code that is "loaded with" this script.

#TODO: What does this do?  Can I remove it to make this script simpler?
$usb = 22;                    # -1 = all
$usemultiplefile = "yes"; #TODO: I still seem to get multiple binary files when this is set to "yes".  What does it really do?
$filelength = 5; #TODO: Understand the units and usage of this parameter.
$laserrate = 1000; #TODO: Where is this used?  What does it do?

#TODO: Eventually, Camillo wants to use the sole argument to this script to decide on a "run configuration".  After being corrupted by the 
#      I/O philosophy of gtkmm 3.22, I am tempted to argue for the MySQL table (or database) read here being the sole "configuration mode". 
#      Users would have a well-defined way to interact with and create new configurations (SQL), and it might be easier to keep a consistent 
#      record of configurations used by managing database(s).  Also, separating the configuration format from this script's implementation 
#      removes the temptation to couple configuration more closely to potentially complicated logic encoded in perl.  
$CRT::onlinetable = shift; #Use the sole argument to this script as the table from which to read PMT configuration.  Right now, this table 
                           #doesn't provide baselines.  There might one day be a second argument for a baseline table.   
              #"crt_downstream"; #"online_force_trigger"; #Changed by Andrew Olivier on 7/10/2018 to use the table I prepared for the downstream 
                                 #CRTs.  

#TODO: These are local variables, so they probably CAN'T be filled by loadconfig.  Do I even need them?  Simpler is better, especially when 
#      someone might have to adapt this for another experiment some day...
my @pmtnumbers = ( ); #filled by loadconfig()
my @pmtboardnum = ( ); #filled by loadconfig()

#TODO: Do I use both debug and mode?  I kind of think mode might not be used.
my $debug = 0;       #1=debug mode, 0=mysql
my $mode = 'mysql';

#TODO: Make a symbolic link that points to DataPath and DataFolder so Matt can retrieve the data from a well-defined location
my $disk = "1";          # Disk number (1 or 2)
my $DataPath = "CRTDAQ/DATA/";

my $DataFolder = "/data${disk}/${DataPath}";

###########################################################
my $totalpmt = 7; #7; #Filled by loadconfig().  Will print a seemingly-benign error message later if this doesn't happen to be the 
                  #number of rows in the MySQL table.  
                  #TODO: I think totalpmt is a local variable, so it can't be filled by loadconfig!
my $totusb = 2; #TODO: Do I use this?  It's a local variable...
###########################################################

#TODO: What does numbaselinetriggers control?
my $numbaselinetriggers = 1;

#This is the "real" totusbs I use
$CRT::totusbs = 2;

system qq | start_readout.sh "readout" "1" "1" "none"|;

loadconfig($debug);  #need to readout everything from database to fill structure

if($debug)
{
  #Print PMTs configured for debugging
  foreach my $number (@CRT::pmtnumbers)
  {
    print "Using PMT $number\n";
  }
  
  #Print a warning if there are NO PMTs configured because the debugging loop above would not print anything
  if(!scalar @CRT::pmtnumbers) 
  {
    print "No PMTs configured for readout in sevenboard_downstream.pl!\n";
  }

  #TODO: This is just a demonstration for my own sanity that totalpmt is a "local" variable and CRT::totalpmt is a 
  #      perl-module-scope variable.  
  print "After loadconfig, totalpmt is $totalpmt.  CRT::totalpmt is $CRT::totalpmt\n";

#TODO: I am not using this loop anymore, so remove it
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
}

#TODO: Why wait 1.0s here?  Ask Camillo
tarry 1.0;

initializeboard("auto",500); #TODO: 500 is number of baselines pulled off of?

#At this point, the directory $CRT::Data_Folder/Run_$CRT::run has been created according to initializeboard in CRT.pm.  Replace the Data_Folder 
#symbolic link so that it points to the place where the binary directory now exists.  This gives Matt a well-defined place to find the current 
#run's binary data.
#TODO: CRT::DataFolder is only set in disk space checks!  The information that I think it should contain is always kept in local variables.  
#system qq |ln -sf $CRT::DataFolder/Run_$CRT::run Data_Folder|;
system qq |ln -sf /disk1/data1/CRTDAQ/DATA/Run_$CRT::run Data_Folder|;

starttakedata;

#TODO: In the "ultimate production version", make Matt call stoptakedata?  That way, artdaq, rather than allboard.pl, would control how 
#      long data taking lasts.  From discussions with Camillo and Matt, it sounds like letting the CRTs run as long as possible will 
#      probably be our final operating mode.
tarry $run_length;

stoptakedata; 

#I think the rest of this code just makes (very useful) monitoring plots.  The decoded signal file produced in generatecsv might also 
#be useful for Sarah's calibration analysis and/or online monitoring plots that are decoupled from LArSoft.  In fact, these plots 
#probably do most of the online monitoring work for me...

#TODO: If stoptakedata becomes someone else's responsibility one day, everything below starttakedata needs to dissappear.  Maybe I could 
#      move all of this plotting code to its' own perl script.  I'd have to find a clever way to come up with tables like pmttousb though. 
check_rate("${DataFolder}/Run_$run_number/binary",$CRT::totalpmt,$CRT::totusbs,0,400);

for(my $i=1; $i<=$CRT::totalpmt; $i++) { #Change by Andrew Olivier on 7/10/2018: In CRT::loadconfig(), totalpmt is incremented BEFORE each time that 
                                    #pmttousb and pmttoboard are set.  So, the key=0 entry in each of these maps is never set.  This loop 
                                    #should loop over all PMTs in those maps, so its' indexing should start at 1 to match loadconfig().  
                                    #Otherwise, I am getting an error about an unitialized value that leads to other bad things.   

        print "usb: $CRT::pmttousb[$i]\t pmt: $CRT::pmttoboard[$i]\n";

        #TODO: This seems to process baselines once for each PMT instead of once for each USB.  
        #print "About to look for baseline file: ${DataFolder}/Run_${run_number}/binary/baseline_$pmttousb[$i]\n" if $debug;
	#system qq| /usr/bin/perl -S baselines.pl "${DataFolder}/Run_${run_number}/binary/" "baseline_$pmttousb[$i]" "1"|;

	generatecsv($DataFolder,$run,$pmttousb[$i], $pmttoboard[$i]);       # Generate summary.csv file for $usb, $pmt in $DataFolder, Run $run                                                                  
        tarry 0.5;    
        $average = getavg($pmttoboard[$i],3);                         # getavg($pmt,$col) returns the average of column $col in summary.csv (see below)                                                                                  
        checkeff($pmttoboard[$i],"${DataFolder}/Run_${run_number}");         # writes statistics about x packets and timestamps to TimeStamps.txt in data folder                                                                               
        $rate = 64*$average/$elapsed_time;                  # Total rate = $num_channels*$avg_hits/$elapsed_time                                                                                                              
        printf("PMT %3.0d:\t Rate:%6.0d\n",$pmttoboard[$i],${rate}); # Print info                                     
}

#generatesummaryplot($DataFolder,$run); #TODO: Put this back later

#TODO: The window no longer seems to close on the Linux CRT DAQ machine.  It just stops the script from finishing, so commenting it.  
#print "DONE ! Press enter to close this window.\n";
#<STDIN>;

# Column definitions in summary.csv                                                                               
# ($col ==1 ) { $colname = "pulse height"; }                                                                      
# ($col ==2 ) { $colname = "pulse height sigma"; }                                                                
# ($col ==3 ) { $colname = "hits per channel"; }                                                                  
# ($col ==4 ) { $colname = "hits per packet"; }                                                                   
# ($col ==5 ) { $colname = "number of photo-electrons"; }                                                         
# ($col ==6 ) { $colname = "gain"; }                                                                              
# ($col ==7 ) { $colname = "rate per channel"; }                         
