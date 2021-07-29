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
$DataPath = "CRTDAQ/DATA/";

$DataFolder = "/data${disk}/${DataPath}";

###########################################################
my $totalpmt = 7; #7; #Filled by loadconfig().  Will print a seemingly-benign error message later if this doesn't happen to be the 
                  #number of rows in the MySQL table.  
                  #TODO: I think totalpmt is a local variable, so it can't be filled by loadconfig!
my $totusb = 2; #TODO: Do I use this?  It's a local variable...
###########################################################

#TODO: What does numbaselinetriggers control?
my $numbaselinetriggers = 1;

#This is the "real" totusbs I use
$CRT::totusbs = 1;

loadconfig($debug);  #need to readout everything from database to fill structure

#Tell CRT.pm that all loaded USBs are taking data.  
#TODO: This is a hack that circumvents what is possibly some protection against stopping 
#      other readout jobs.  We probably need to redesign components of CRT.pm to really 
#      solve this problem.
for(my $running = 1; $running <= $CRT::totalpmt; ++$running) #PMT indices always start at 1 when working with CRT.pm.  
{
  push(@CRT::usblocal, $CRT::pmttousb[$running]); #Make sure CRT.pm knows to loop over this USB
  #print "Setting USB $CRT::pmttousb[$running] status to 1.\n";
  $CRT::structure[$CRT::pmttousb[$running]] = 1;
}

stoptakedata; 
system qq|perl -S /home/bcrt/readout_linux/script/stop.pl|; #Kill any readout processes remaining so that the run control doesn't hang.  
