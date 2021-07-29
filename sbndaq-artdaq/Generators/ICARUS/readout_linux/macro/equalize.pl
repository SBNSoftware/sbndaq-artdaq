use strict;
use warnings;
use DCsubroutinembmysql;
use usb_readout;

$usb = 4;    		              # -1 = all

$runlength = 5; 		          #set runlength in seconds
$usemultiplefile = "yes";
$filelength = 20;
$laserrate = 1000;		

$pmt = 7;

##############################################################################

#load data for each pmt connected
##syntax loadpmt pmtnumber;
setpmtdata $usb, $pmt;

#equalization
adjustpulseheight 350;


print "DONE ! Press enter to close this window.";
<STDIN>;