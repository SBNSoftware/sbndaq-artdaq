package DCsubroutinembmysql;

our (@ISA, @EXPORT, @EXPORT_OK, %EXPORT_TAGS, $VERSION);
use Exporter;
$VERSION = 1.00;
@ISA = qw(Exporter);

@EXPORT = qw(baseline initializefilestructure initializeusb loadgainmysql initializetriggerbox signal
takedata takedatamb plotdata plotdatamb dacadc dacadcfit generatecsv loadrunsummarymysql
copyrunsummarywebpage movedata baselinemb writecharacterizationsummary countrun
adjustpulseheight getavg getratio800 writeonfile checkentries writelog setpmtdata getpmtdata checkeff
loadpmtdata loadpmtdata_auto loadnewpmtdata printsummary sendmail takedatamu producentuple $pmt $usb $pmtserialnumber $HVsetting $lasersinglepe
$DACt $thresh $hdelay $runlength $meanpulseheight $gate $gateonoff
$date $run $full_path $file $baselinedir $baselines @averagehitchannel
$gaindirectory $savedgaindirectory $filename $dacdirectory $comments
$newdacdirectory $dacfile $average $repeat $ADCpe @pmtdata @pmt_board_array @gaindata $inispread
$meanph $meanpe $meangain $meanentries $laserrate @dacvalue $takebaseline $mysqlload
$pmtmb $usemaroc2gainconstantsmb $boardnumber $gateuse $trigger_mode $newrunlength
$logfile $newlogfile $laserintensity $homedir $usemaroc2gainconstants $filelength
$usemultiplefile $filebase $hostnamepc @noiseratePE $cornercorrection $noisesummaryrun $username
$spread_pulse_height $maxminratio $gainratio $phratio $sync);


@EXPORT_OK = ();
%EXPORT_TAGS = ();

use IO::Socket;
use Time::HiRes;
use File::Path;
use Cwd;
use strict;
use warnings;
use usb_readout;
use DBI;
use Sys::Hostname;
use Net::SMTP;
use Getopt::Std;	


our ($pmt,$usb,$pmtserialnumber,$HVsetting,$lasersinglepe,$mysqlload,
$DACt,$thresh,$hdelay,$runlength,$meanpulseheight,$gate,$gateonoff,
$date,$run,$full_path,$file,$baselinedir,$baselines,$ADCpe,$comments,
$gaindirectory,$savedgaindirectory,$filename,$dacdirectory,$newdacdirectory,
$dacfile,$average,$repeat,$logfile,$newlogfile,$laserintensity,@pmtdata,@gaindata,
$meanph,$meanpe,$meangain,$meanentries,$laserrate,@dacvalue,$homedir,$boardnumber,
$takebaseline,$usemaroc2gainconstants,$pmtmb,$usemaroc2gainconstantsmb,
$gateuse,$trigger_mode,$newrunlength,$filelength,$dir,$usemultiplefile,$filebase,@gain,
$hostnamepc,$hostnamepclc,$equalization,$passwd,$choice,@noiseratePE,$cornercorrection,$noisesummaryrun,
$newpmtdatafound,$username,$inispread,@averagehitchannel,$spread_pulse_height,
$gainratio,$maxminratio,$phratio,$sync,@pmt_board_array);

our ($gain1, $gain2, $gain3, $gain4, $gain5, $gain6, $gain7, $gain8, $gain9, 
			$gain10, $gain11, $gain12, $gain13, $gain14, $gain15, $gain16, $gain17, $gain18, $gain19, 
			$gain20, $gain21, $gain22, $gain23, $gain24, $gain25, $gain26, $gain27, $gain28, $gain29, 
			$gain30, $gain31, $gain32, $gain33, $gain34, $gain35, $gain36, $gain37, $gain38, $gain39, 
			$gain40, $gain41, $gain42, $gain43, $gain44, $gain45, $gain46, $gain47, $gain48, $gain49, 
			$gain50, $gain51, $gain52, $gain53, $gain54, $gain55, $gain56, $gain57, $gain58, $gain59, 
			$gain60, $gain61, $gain62, $gain63, $gain64);

our($p0,$p1);

our $database="doublechooz_ov_far";
#our $hostname="riverside.nevis.columbia.edu:3306";
our $hostname="localhost:3306";
our $user="dcdaq";
our $password="dcdaq";  # the root password
our ($dsn,$dbh,$drh,$sth);


$homedir = getcwd;
$homedir =~ s|Scripts_mb/macro||;
$homedir =~ s|\/|\\|g;

$hostnamepc = hostname;
$hostnamepclc = hostname;
$hostnamepclc =~ tr/A-Z/a-z/;


#$filelength = 5;
#$laserrate = 100;
$meanpulseheight  = 350;
$mysqlload = 0;
$equalization = "false";
$choice = "";
$cornercorrection = "off";
$sync = 'on';
###########################################################################
sub setpmtdata {

my $usbboard = shift;
my $pmtnumber = shift;

#print "Getting info for PMT board $pmtnumber \n";

pmtserialinput:
print "Enter PMT Serial Number (for example: 4662): ";
chomp($pmtserialnumber = <STDIN>);                             # read line from stdin and remove newline
if($pmtserialnumber eq "") {goto pmtserialinput};

$boardnumber = $pmtnumber;

if($pmtserialnumber eq "none") {
	# $boardnumber = $pmt;
	$HVsetting = "none";
	$usemaroc2gainconstantsmb = "no";
}
else{
	# boardnumberinput:
	# print "Enter Board Number for $pmtserialnumber: ";
	# chomp($boardnumber = <STDIN>);                             
	# if($boardnumber eq "") {goto boardnumberinput};

	hvsettinginput:
	print "Enter the High Voltage setting: ";
	chomp($HVsetting = <STDIN>);                                    
	if($HVsetting eq "") {goto hvsettinginput};

	marocuse:
	print "Use maroc2 gain constants to take data? (yes or no) ";
	chomp($usemaroc2gainconstantsmb = <STDIN>);                                    
	if($usemaroc2gainconstantsmb eq "") {goto marocuse};
}

DACinput:
print "Enter the DAC threshold: ";
chomp($DACt = <STDIN>);                                   
if($DACt eq "") {goto DACinput};

gate:
print "Which gate (op73)? (on or off or alladc or alladcoff) ";
chomp($gate = <STDIN>);                                    # read line from stdin and remove newline
if($gate eq "") {goto gate};

$trigger_mode = 1;

# trigger:
print "Which trigger (op75)? (0,1,2) ";
chomp($trigger_mode = <STDIN>);                                    # read line from stdin and remove newline
if($trigger_mode eq "") {goto trigger};

hdelay:
print "Which hdelay? (push enter will set the default value of 5) ";
chomp($hdelay = <STDIN>);                                    # read line from stdin and remove newline
if($hdelay eq "") {$hdelay = 5};

if($choice ne "manual") {
print "Any comments \n";
chomp($comments = <STDIN>);                                    # read line from stdin and remove newline
}

$pmtdata[$usbboard][$pmtnumber][0] = $pmtserialnumber;
$pmtdata[$usbboard][$pmtnumber][1] = $boardnumber;
$pmtdata[$usbboard][$pmtnumber][2] = $HVsetting;
$pmtdata[$usbboard][$pmtnumber][3] = $DACt;
$pmtdata[$usbboard][$pmtnumber][4] = $usemaroc2gainconstantsmb;
$pmtdata[$usbboard][$pmtnumber][5] = $gate;
$pmtdata[$usbboard][$pmtnumber][6] = $hdelay;
$pmtdata[$usbboard][$pmtnumber][7] = $trigger_mode;
$pmtdata[$usbboard][$pmtnumber][8] = $comments;

print "\n";
print "\n";
}

###########################################################################
###########################################################################

sub loadnewpmtdata{

my $pmtserialnumber1;

$usb = shift;
$pmt = shift;

username:
print "Enter your name:\n";
chomp($username = <STDIN>);                                   
if($username eq "") {goto username};


pmtserialnumber:
print "Enter pmt serial number without PA (example 4673) for board $pmt:\n";
chomp($pmtserialnumber = <STDIN>);                                    # read line from stdin and remove newline
if($pmtserialnumber eq "") {goto pmtserialnumber};

pmtserialnumberbis:
print "Renter pmt serial number without PA (example 4673) for board $pmt:\n";
chomp($pmtserialnumber1 = <STDIN>);                                    # read line from stdin and remove newline
if($pmtserialnumber1 eq "") {
	goto pmtserialnumberbis;
	}
else{
	if($pmtserialnumber1 ne $pmtserialnumber){
		print "PMT serial number mismatch\n\n";
		print "Now exit !\n";
		exit 0;
	}
}

#code for retrieving info form mysql database

$newpmtdatafound = 0;

eval {
	$dsn = "dbi:mysql:database=$database;host=$hostname"; 
	$dbh = DBI->connect($dsn, $user, $password); 

	print "Failed to connect to database: \n" if $@;
	
	if($@ ne "") {
		print "Connection with mysql database not available, program will abort and email to expert will be sent\n";
		sendmail("PMT characterization mysql server connection broken !");
	    exit 0;
	}
	elsif($@ eq "") {
		print "Connected to MySQL database\n\n";
	}

	$drh = DBI->install_driver("mysql");

	$sth = $dbh->prepare("SELECT PMT_Serial FROM onlinenew");
  
    $sth->execute();
	  while (my $ref = $sth->fetchrow_arrayref()) {
	    if($ref ->[0] eq "$pmtserialnumber") { 
			#print "Found info in mysql database for pmt = $ref->[0] \n"; 
			$newpmtdatafound = 1;
		}
	   }
	   
	$sth->finish();
	
	if($newpmtdatafound == 1){
		print "PMT already characterized, make sure of the serial number, email notification will be sent to expert\n\n";
		print "To continue with the characterization, enter the password. Otherwise press enter.\n";
		chomp(my $passwordovr = <STDIN>);                                    # read line from stdin and remove newline
		if($passwordovr ne $password) {
			print "This procedure will be terminated ! \n\n";
			exit 0;
		}
		sendmail("PMT characterization for pmt = $pmtserialnumber overwritten into mysql by $username");
	}
	elsif($newpmtdatafound == 0){
		print "Exporting info to mysql database for pmt = $pmtserialnumber \n";
		$dbh->do("INSERT INTO online VALUES ($usb, '$pmtserialnumber', $pmt, 'no', 
		0, 5, '800', 'on', 0,
		16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,
		16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,
		16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,
		16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16)");
	}	
	
	  $dbh->disconnect();
};

$mysqlload = 0;

$pmtdata[$usb][$pmt][0] = $pmtserialnumber;
$pmtdata[$usb][$pmt][1] = $pmt;
$pmtdata[$usb][$pmt][2] = 800;
$pmtdata[$usb][$pmt][3] = 1000;
$pmtdata[$usb][$pmt][4] = "no";
$pmtdata[$usb][$pmt][5] = "on";
$pmtdata[$usb][$pmt][6] = 5;
$pmtdata[$usb][$pmt][7] = 0;
$pmtdata[$usb][$pmt][8] = "PMT characterization started by $username";

}

#############################################################################

sub loadpmtdata {

$usb = shift;
$pmt = shift;
my $switch =  shift;
my $gate_override = shift;


if(!$switch){
pmtserialnumber:
print "Enter pmt serial number without PA (example 4673) for board $pmt:\n";
chomp($pmtserialnumber = <STDIN>);                                    # read line from stdin and remove newline
if($pmtserialnumber eq "") {goto pmtserialnumber};
print "Re-enter pmt serial number without PA (example 4673) for board $pmt:\n";
chomp(my $tempserial = <STDIN>);
if($tempserial ne $pmtserialnumber) { # read line from stdin and remove newline
	print "Serial numbers did not match.  Please try again.\n";
	goto pmtserialnumber;
}


#code for retrieving info form mysql database

my $foundcount = 0;

my $choicefirst;
my $choicesecond;
my $DACt1; 
my $gate1;
my $maroc2;
my $board1;


eval {
	$dsn = "dbi:mysql:database=$database;host=$hostname"; 
	$dbh = DBI->connect($dsn, $user, $password); 

print "Failed to connect to database: $@\n" if $@;
if($@ eq "") {print "Connected to MySQL database\n\n"; }

	$drh = DBI->install_driver("mysql");

	$sth = $dbh->prepare("SELECT USB_serial, PMT_Serial, board_number, HV, DAC_threshold,
	 use_maroc2gain, gate, hdelay, trigger_mode, gain1, gain2, gain3, gain4, 
	gain5, gain6, gain7, gain8, gain9, gain10, 
	gain11, gain12, gain13, gain14, gain15, gain16 , 
	gain17, gain18, gain19, gain20, gain21, gain22 , 
	gain23, gain24, gain25, gain26, gain27, gain28 , 
	gain29, gain30, gain31, gain32, gain33, gain34 , 
	gain35, gain36, gain37, gain38, gain39, gain40 , 
	gain41, gain42, gain43, gain44, gain45, gain46, 
	gain47, gain48, gain49, gain50, gain51, gain52, 
	gain53, gain54, gain55, gain56, gain57, gain58, 
	gain59, gain60, gain61, gain62, gain63, gain64 FROM onlinenew");

	$sth->execute();
	while (my $ref = $sth->fetchrow_arrayref()) {		
			if($ref->[1] eq $pmtserialnumber){ #check for matching pmt_serial number and board_address
				$pmtdata[$usb][$pmt][0] = $ref->[1];
				$pmtdata[$usb][$pmt][1] = $ref->[2];
				$pmtdata[$usb][$pmt][2] = $ref->[3];
				$pmtdata[$usb][$pmt][3] = $ref->[4];
				$pmtdata[$usb][$pmt][4] = $ref->[5];
				$pmtdata[$usb][$pmt][5] = $ref->[6];
				$pmtdata[$usb][$pmt][6] = $ref->[7];
				$pmtdata[$usb][$pmt][7] = $ref->[8]; 
			
				for(my $index=0;$index<64;$index++){
					$gaindata[$usb][$pmt][$index] =  $ref->[9+$index];
				}
			#if($ref->[0] == $usb and $ref->[1] eq $pmtserialnumber and $ref->[2] == $pmt){				#check for matching pmt_serial number and board_address
		        print "Found info for: PMT_serial = $ref->[1], board_number = $pmt \n\n";
				print "Please check that your setup has the following setting:\n";
				print "USB address = $usb \n";
				print "Pmt serial number=$pmtdata[$usb][$pmt][0]\n";
				print "Board number=$pmtdata[$usb][$pmt][1]\n";
				print "HV setting=$pmtdata[$usb][$pmt][2]\n";
				print "DAC threshold = $pmtdata[$usb][$pmt][3]\n";
				print "Use of maroc2 gain constants = $pmtdata[$usb][$pmt][4]\n";
				print "Gate = $pmtdata[$usb][$pmt][5]\n";
				print "hdelay = $pmtdata[$usb][$pmt][6]\n";
				print "trigger_mode = $pmtdata[$usb][$pmt][7]\n\n";
				$DACt1 = $pmtdata[$usb][$pmt][3];
				$gate1 = $pmtdata[$usb][$pmt][5];
				$maroc2 = $pmtdata[$usb][$pmt][4];
				$foundcount = 1;
				$mysqlload = 1;
				print "Press enter if you want to take data with that settings. If instead you would like to change any settings type change.\n\n";
				chomp($choicefirst = <STDIN>);
			}
			if($ref->[1] eq $pmtserialnumber){
				$board1 = $pmtdata[$usb][$pmt][1];
			}				
	}
	$sth->finish(); 
	$dbh->disconnect();
};

print "Reading from MySQL failed: $@\n" if $@;
#if($@ eq "") {print "Setting read out from MySQL server @ Nevis\n\n";}

  
  if($foundcount == 0){
	print " \t\t !!!!!! Warning !!!!!!!\n";
	print "Info for your PMT <-> board not present in the mysql database or do not match.\n\n";
	print "For PMT = $pmtserialnumber the board address in the MySQL database is not $pmt\n";
	$mysqlload = 0;
	restartchoice:
	print "If you want to quit just type quit and press enter, if you want to continue and put the run info manually type manual\n";
	chomp($choice = <STDIN>);                                    
		if($choice ne "quit" && $choice ne "manual") {goto restartchoice};
		if($choice eq "quit") {exit()};
		if($choice eq "manual") {
			setpmtdata($usb,$pmt);
			if($pmtdata[$usb][$pmt][4] eq "yes") {
				print "I will use if founded the MARCOC2 gain constants stored in this machine\n"; 
			}
		goto goend	
		}
  }
  else{
  if($choicefirst eq "change"){
    $choice = "manual";
	setpmtdata($usb,$pmt);
	if($pmtdata[$usb][$pmt][4] eq "yes") {
		print "Please type which gain constants you would like to use (mysql or local)\n";
		chomp($choicesecond = <STDIN>);
		if($choicesecond eq "local"){ $mysqlload = 0;};	
	}
  }
}
  
print "\n\n"; 

}
else {
print "Loading setting as default for pmt board $pmt\n";

$pmtdata[$usb][$pmt][0] = "none";
$pmtdata[$usb][$pmt][1] = $pmt;
$pmtdata[$usb][$pmt][2] = 0;
$pmtdata[$usb][$pmt][3] = 1000;
$pmtdata[$usb][$pmt][4] = "no";
$pmtdata[$usb][$pmt][5] = "off";
$pmtdata[$usb][$pmt][6] = 5;
$pmtdata[$usb][$pmt][7] = 0b00010000;
$pmtdata[$usb][$pmt][8] = "test with default configuration";

}

push(@pmt_board_array, $pmt); ##camillo added this on 02/06/17

goend:
if(!$switch){
print "Please enter comments for this run (max 50 characters), press enter when done\n";
chomp($comments = <STDIN>);
$pmtdata[$usb][$pmt][8] = $comments;
}
print "\n";
print "\n";

if(defined($gate_override)) {
	print "$gate_override\n";
	if(!($gate_override eq "off" or $gate_override eq "alladcoff" or $gate_override eq "on"
		or $gate_override eq "trigger")) {
		print "Gate override option $gate_override not defined. Available options: off, on, alladcoff, or trigger\n";
		exit(1);
	}
	$pmtdata[$usb][$pmt][5] = $gate_override;
	print "Warning! Gate overridde initiated by user! Now using gate = $gate_override\n";
 }
}
###########################################################################
sub getpmtdata{

my $usbboard = shift;
my $pmtnumber = shift;

$pmtserialnumber = $pmtdata[$usbboard][$pmtnumber][0];
$boardnumber = $pmtdata[$usbboard][$pmtnumber][1];
$HVsetting = $pmtdata[$usbboard][$pmtnumber][2];
$DACt = $pmtdata[$usbboard][$pmtnumber][3];
$usemaroc2gainconstantsmb = $pmtdata[$usbboard][$pmtnumber][4];
$gate = $pmtdata[$usbboard][$pmtnumber][5];
$hdelay = $pmtdata[$usbboard][$pmtnumber][6];
$trigger_mode = $pmtdata[$usbboard][$pmtnumber][7];
$comments = $pmtdata[$usbboard][$pmtnumber][8];

if($equalization eq "true") { $usemaroc2gainconstantsmb = "yes"; };

if($mysqlload == 1) {
	for(my $index=0;$index<64;$index++){
				$gain[$index+1] = ${gaindata[$usbboard][$pmtnumber][$index]};
				#print "$gain[$index+1]\n";
	}
}
 #print "pmt serial number=$pmtserialnumber\n";
 #print "board number=$boardnumber\n";
 #print "HV setting=$HVsetting\n";
 #print "DAC threshold in use = $DACt\n";
 #print "use of maroc2 gain constants = $usemaroc2gainconstantsmb\n";
 #print "gate = $gate\n";
 #print "hdelay = $hdelay\n";
 #print "comments = $comments\n";
}

###########################################################################
sub baseline {

#print "Initializing pmt boards ... be patient ...\n";
#print "\n";

my $fileb;

init_readout "127.0.0.1";           # or "localhost"

my $usb = shift;    # -1 = all
my $pmt1 = shift;    # 63 = all

our $filebase = "baselines.bin";         # name of file to generate

my $fullpath = "$full_path\\USB_${usb}";
dir $fullpath;  
            
#print "Starting  USB = $usb,  PMT = $pmt,  Run = $run\n";
com $usb, $pmt1, 110, 1;                # turn led on/off
com $usb, $pmt1, 109, 1;                # vdd_fsb on
com $usb, $pmt1, 73, 0b00000;           # set up pmt module
#print "Configuring...\n";
com $usb, $pmt1, 74, 0b0100000;      # default gain
com $usb, $pmt1, 70, 0;              # load default
dac $usb, $pmt1, 1000;                # dac threshold
com $usb, $pmt1, 71, 0;              # rst_g to maroc
com $usb, $pmt1, 72, 0;              # write config
com $usb, $pmt1, 72, 0;              # write config
com $usb, $pmt1, 73, 0b00110;         # set up pmt module
com $usb, $pmt1, 75, 0b00010000;     #set up trigger mode
com $usb, $pmt1, 254, 0;                 # enable triggers

tarry 1;

print "Taking baseline data...\n";
for(1..10) {						#avoid first packets
    com $usb, $pmt1, 81, 0;         # test trigger
}

tarry 1;

#new $usb, "$dir\\$filebase";        # open new file

for(1..500) {
    com $usb, $pmt1, 81, 0;         # test trigger
}

#print "Finishing up...\n";
com $usb, $pmt1, 255, 0;
com $usb, $pmt1, 73, 0b00000;       # set up pmt module
tarry 0.5;                          # wait for late packets
#shut $usb;                          # close file


#print "Baseline data taken completed.\n\n";
}

###########################################################################
sub initializefilestructure {

my $fileb;
my @lineb;
my @line;
my $max = -1;

our $subrun = "";
our $dir;
my $pmtserialnumber;
my $HVsetting;
my $usb1;

$usb = shift;    # -1 = all
$usb1 = shift;
#$pmt = shift;    # 63 = all

init_readout "127.0.0.1";           # or "localhost"

my ($sec,$min,$hour,$day,$month,$yr19,@rest) =   localtime(time);
$yr19 = $yr19 + 1900;
$month = sprintf("%02d",${month}+1);
$day = sprintf("%02d",$day);
our $date = "${yr19}${month}${day}";

my @files = <..\\..\\DATA\\Run_${date}_*>;

foreach $fileb (@files) {
  if($fileb =~ /Run_(\d+)_(\d+)/) {
    $max = $2 if $2 > $max;
  }
}  
  $max = $max + 1;
  
$subrun = $max; 
our $run = "${date}_${subrun}";
our $full_path = $homedir . "DATA\\Run_${run}";

$dir = $homedir . "DATA\\Run_${run}\\USB_${usb}";  # change this to create your own directory structure

our $filebase = "baselines.bin";         # name of file to generate

my $fullpath = "$full_path\\USB_${usb}";
dir $fullpath;
#print "Opening file: $filebase\n";
new $usb, "$dir\\$filebase";        # open new file


}
###########################################################################

sub initializeusb {

my $usb = shift;
my $pmt = shift;

#baseline($usb,$pmt);  # taking baseline data

getpmtdata $usb, $pmt;

#our $dir1 = $homedir . "DATA\\Run_${run}\\USB_${usb}\\PMT_${pmt}";  # change this to create your own directory structure
#dir $dir1;     

my $dir2 = $homedir . "DATA\\Run_${run}\\USB_${usb}"; 
my $full_path2 = $homedir . "DATA\\Run_${run}\\USB_${usb}";
                 

#system qq|cmd /C baselines.pl "$dir2" "$filebase" "$pmt"|;

#print "Baseline decoded for pmt $pmt...\n\n";

if ($gate eq "on"){
$gateonoff = 0b11010;
}
elsif( $gate eq "off" ){
$gateonoff = 0b01010;
$trigger_mode = 0b00010000;
}
elsif( $gate eq "trigger" ){ 
$gateonoff = 0b01011;
$trigger_mode = 0b01010000;  
}
elsif( $gate eq "triggera" ){
$gateonoff = 0b01111;  
}
elsif( $gate eq "alladc" ){
$gateonoff = 0b11110;  
}
elsif( $gate eq "alladcoff" ){
$gateonoff = 0b01110;
$trigger_mode = 0b00010000;
}
elsif( $gate eq "test" ){
$gateonoff = 0b01010;
$trigger_mode = 0b01100000;  
}
elsif( $gate eq "testhit" ){
$gateonoff = 0b01010;
$trigger_mode = 0b00000110;  
}
elsif( $gate eq "allhit" ){
$gateonoff = 0b01001; 
}

######################################################
init_readout "127.0.0.1";           # or "localhost"
#send_out "Greetings\n";             # write message to get current dir

#print "Initializing  USB = $usb,  PMT = $pmt on run $run\n";
com $usb, $pmt, 110, 1;				#turn off the three led on the PMT's board
com $usb, $pmt, 109, 1;             # vdd_fsb on
com $usb, $pmt, 73, 0b00000;        # set up pmt module
#com $usb, $pmt, 255, 0;             # clear buffers
#com $usb, $pmt, 84, 255;            # buffer size limit  
#tarry 2.5;                          # seconds
com $usb, $pmt, 74, 0b0100000;      # default gain
com $usb, $pmt, 70, 0;              # load default
dac $usb, $pmt, $DACt;              # threshold value
com $usb, $pmt, 67, 0b000010;       # statea
com $usb, $pmt, 68, 0b000000;       # stateb  40f 25k 50k 100k 50f 100f
com $usb, $pmt, 69, 0b000000;       # statec
com $usb, $pmt, 71, 0;              # rst_g to maroc 
com $usb, $pmt, 72, 0;              # write config
com $usb, $pmt, 72, 0;              # write config
com $usb, $pmt, 73, $gateonoff;     # $gateonoff;     # gate
com $usb, $pmt, 75, $trigger_mode;  # trigger mode $trigger_mode see document for details
com $usb, $pmt, 80, 5;              # hdelay
com $usb, $pmt, 85, 0b0000;         # set up pipe delay
com $usb, $pmt, 86, 1;              # edge strip mode
com $usb, $pmt, 87, 0b10;           # force trigger -> 01: 1msec, 10: 16ms, 11:256ms

if($usemaroc2gainconstantsmb eq "no") {
        com $usb, $pmt, 74, 0b0100000;      # default gain 
}
elsif($usemaroc2gainconstantsmb eq "yes" && $mysqlload == 0 && $equalization eq "false"){ 

$gaindirectory = $homedir . "Scripts_mb\\marocgainsetting\\gain_PA${pmtserialnumber}_${boardnumber}_${HVsetting}.txt";
$savedgaindirectory = $homedir . "Scripts_mb\\marocsavedgainsetting\\gain_PA${pmtserialnumber}_${boardnumber}_${HVsetting}_${date}.txt";

    if(-e $gaindirectory) {
        open IN, $gaindirectory or die $!;
#		print "Local file with the Maroc2 gain constants found, now reading and applying settings...\n\n";
		open OUT, ">$full_path\\PMT_${pmt}\\gain.txt" or die $!;	
		while(<IN>) {
	            chomp; s/#.*//;    # delete comments
	            next if /^\s*$/;    # skip blank lines
	            my($channel, $value) = split /\s+/;
	            my $happy = 2 * $value;
	            com $usb, $pmt, $channel - 1, $happy;
				print OUT "$channel \t $value \n";
				$gain[$channel] = $value;
	        }
        close IN or die $!;
		close OUT or die $!;
		} else {
		    print "Local file with the Maroc2 gain constants not found, use default=16 for all channels\n";
			print "$gaindirectory\n";
	        open OUT, ">$gaindirectory" or die $!;
			open OUT1, ">$full_path2\\PMT_${pmt}\\gain.txt" or die $!;	
	        for(1..64) {
				print OUT "$_\t16\n"; 
				print OUT1 "$_\t16\n";
				}
	        close OUT or die $!;
			close OUT1 or die $!;
		}
		#system qq|cmd /C copy "$gaindirectory" "$full_path/PMT_${pmt}/gain.txt" >> logfile_${pmtserialnumber}_${date}.txt|;
	}
elsif($usemaroc2gainconstantsmb eq "yes" && $mysqlload == 1 && $equalization eq "false"){
	open OUT, ">$full_path2\\PMT_${pmt}\\gain.txt" or die $!;
	for(my $index=1;$index<65;$index++){
			com $usb, $pmt, $index - 1, 2*$gain[$index];
			#print "MySQL gain constants found: $index, $gain[$index+1] \n";
	        print OUT "$index \t $gain[$index] \n";       
	}
	print "MySQL gain constants applied. \n";
	close OUT or die $!;
}
elsif($equalization eq "true"){ 

$gaindirectory = $homedir . "Scripts_mb\\marocgainsetting\\gain_PA${pmtserialnumber}_${boardnumber}_${HVsetting}.txt";
$savedgaindirectory = $homedir . "Scripts_mb\\marocsavedgainsetting\\gain_PA${pmtserialnumber}_${boardnumber}_${HVsetting}_${date}.txt";

    if(-e $gaindirectory) {
        open IN, $gaindirectory or die $!;
#		print "Local file with the Maroc2 gain constants found, now reading and applying settings...\n\n";
		open OUT, ">$full_path2\\PMT_${pmt}\\gain.txt" or die $!;	
		while(<IN>) {
	            chomp; s/#.*//;    # delete comments
	            next if /^\s*$/;    # skip blank lines
	            my($channel, $value) = split /\s+/;
	            my $happy = 2 * $value;
	            com $usb, $pmt, $channel - 1, $happy;
				print OUT "$channel \t $value \n";
				$gain[$channel] = $value;
	        }
        close IN or die $!;
		close OUT or die $!;
		} else {
		    print "Local file with the Maroc2 gain constants not found, use default=16 for all channels\n";
			print "$gaindirectory\n";
	        open OUT, ">$gaindirectory" or die $!;
			open OUT1, ">$full_path2\\PMT_${pmt}\\gain.txt" or die $!;	
	        for(1..64) {
				print OUT "$_\t16\n"; 
				print OUT1 "$_\t16\n";
				}
	        close OUT or die $!;
			close OUT1 or die $!;
		}
		#system qq|cmd /C copy "$gaindirectory" "$full_path/PMT_${pmt}/gain.txt" >> logfile_${pmtserialnumber}_${date}.txt|;
}
 
com $usb, $pmt, 109, 0;             		# vdd_fsb off
com $usb, $pmt, 255, 0;                            # disable trigger

#print "Creating summary file for PMT $pmt...append info if file already exists\n";

my $summaryfile = "$full_path2\\summary.txt";
my $summaryfile1 = "$full_path2\\summary_new.txt";


if($usemaroc2gainconstantsmb eq "yes"){
	open IN1,"$full_path2\\PMT_${pmt}\\gain.txt" or die $!;
}

if(-e $summaryfile) {
        open IN, $summaryfile or die $!;
		open OUT, ">$summaryfile1" or die $!;
        while(<IN>) {
            chomp;
			print OUT "$_\t";
		}
		print OUT "\n";
		print OUT "${pmt} \t ${pmtserialnumber} \t ${pmt} \t ${HVsetting} \t  ${DACt} \t ${gate} \t ${trigger_mode} \t ${hdelay} \t ${filelength} \t ${runlength} \t ${usemaroc2gainconstantsmb} \t ${comments} ";
		if($usemaroc2gainconstantsmb eq "yes") {
			while(<IN1>) {
				chomp;
				my($channel, $gainconstant) = split /\t/;
				my $gainconstantreal = $gainconstant; 
				print OUT "$gainconstantreal \t";
			}
			close IN1 or die $!;	
		}
		close OUT or die $!;
        close IN or die $!;	
		system qq|cmd /C move "$summaryfile1" "$summaryfile"|;		
    } else {
        open OUT, ">$summaryfile" or die $!;
        print OUT "${pmt} \t ${pmtserialnumber} \t ${pmt} \t ${HVsetting} \t  ${DACt} \t ${gate} \t ${trigger_mode} \t ${hdelay} \t ${filelength} \t ${runlength} \t ${usemaroc2gainconstantsmb} \t ${comments} \t";
		if($usemaroc2gainconstantsmb eq "yes") {
			while(<IN1>) {
				chomp;
				my($channel, $gainconstant) = split /\t/;
				my $gainconstantreal = $gainconstant; 
				print OUT "$gainconstantreal \t";
			}
			close IN1 or die $!;	
		}
        close OUT or die $!;
    }

	if($equalization ne "true"){ loadrunsummarymysql(); };         #copy info into the mysql database
}

###########################################################################
sub initializetriggerbox {

my $fileb;
my @lineb;
my @line;
my $max = -1;

our $subrun = "";
our $dir;

$usb = shift;    # -1 = all
$pmt = shift;


my ($sec,$min,$hour,$day,$month,$yr19,@rest) =   localtime(time);
$yr19 = $yr19 + 1900;
$month = sprintf("%02d",${month}+1);
$day = sprintf("%02d",$day);
our $date = "${yr19}${month}${day}";

my @files = <..\\..\\DATA\\Run_${date}_*>;

foreach $fileb (@files) {
  if($fileb =~ /Run_(\d+)_(\d+)/) {
    $max = $2 if $2 > $max;
  }
}  
  $max = $max + 1;
  
$subrun = $max; 
our $run = "${date}_${subrun}";
our $full_path = $homedir . "DATA\\Run_${run}\\USB_$usb";

dir $full_path;									#create directory 

init_readout "127.0.0.1";       				# or "localhost"
usb 0, 1;										# auto token on;
#send_out "Greetings\n";             							# write message to get current dir

#print "Initializing  USB = $usb,  PMT = $pmt on run $run\n";
com $usb, $pmt, 255, 0;     					# buffers clear if called
#com $usb, $pmt, 251, 0;                        # in a for loop will force the trigger to the module
#com $usb, $pmt, 251, 1;     					# software sync pulse to trigger module if called
#com $usb, $pmt, 252, 0b0000; 					# flag[3:0] spare trigger control flags (flag == 1### test)
												# flag 1### = test
												# flag 0000 = fan in/out
												# flag 0001 = x&Y
												# flag 0010 = edge strip mode

$filename = "$full_path\\trigger.bin";

if($filelength > $runlength){ $filelength = $runlength; }

$newrunlength = $runlength / $filelength;

if($usemultiplefile eq "no") { 					#not splitting the file in many file
	$newrunlength = 1;
	$filelength = $runlength; 
}
	
my $filenametemp;

for(my $i=0;$i<$newrunlength;$i++){
	print "Starting file $i...\n";
	$filenametemp = "$full_path\\trigger_$i.bin";
	new $usb, $filenametemp;                    # open new file
	if($i==0){
		com $usb, $pmt, 254, 1;                 # enable trigger 
	}	
	tarry $filelength;
	tarry 0.75;                            		# wait for late packets
	shut $usb;                          		# close file
}

com $usb, $pmt, 255, 0;		   	                # reset module		
usb 0, 1;                                       #auto token off					

$filename = "$full_path\\trigger";

for(my $i=0;$i<$newrunlength;$i++){
	system qq|cmd /C decode.pl "${filename}_${i}.bin" >> "${filename}.dec"|;
}

}
###########################################################################



######################################################################################################################

sub takedatamb {

$usb = shift;
$runlength = shift;

print "Preparing to take data ... be patient ...\n";
print "\n";
     
$filename = "$full_path\\USB_${usb}\\signal.bin";

if($filelength > $runlength){ $filelength = $runlength; }

$newrunlength = $runlength / $filelength;

if($usemultiplefile eq "no") { #not splitting the file in many file
	$newrunlength = 1;
	$filelength = $runlength; 
}
	
my $filenametemp;

for(my $i=0;$i<$newrunlength;$i++){
	print "Starting file $i...\n";
	$filenametemp = "$full_path\\USB_${usb}\\signal_$i.bin.part";
	new $usb, $filenametemp;                    # open new file
	if($i==0){
		com $usb, 63, 254, 0; 
	}	
	tarry $filelength;
	tarry 0.75;                            		# wait for late packets
	shut $usb;                          		# close file
	tarry 0.75;
	system qq|cmd /C move "${filenametemp}" "${full_path}\\USB_${usb}\\signal_${i}.bin"|;
}
#just at the end
com $usb, 63, 73, 0b00000;		                # gate				
tarry 0.5;
com $usb, 0, 255, 0;                            # disable trigger
tarry 0.5;                		


# print "Data taken completed...\n";
# print "\n";
}
################################################################

sub takedatamu {

my $usb1 = shift;
my $usb2 = shift;
$runlength = shift;

my $usb = -1;

print "Preparing to take data ... be patient ...\n";
print "\n";
     
$filename = "$full_path\\signal.bin";

if($filelength > $runlength){ $filelength = $runlength; }

$newrunlength = $runlength / $filelength;

if($usemultiplefile eq "no") { #not splitting the file in many file
	$newrunlength = 1;
	$filelength = $runlength; 
}
	
my $filenametemp1;
my $filenametemp2;

my $full_path = $homedir . "DATA\\Run_${run}";

for(my $i=0;$i<$newrunlength;$i++){
	print "Starting file $i...\n";
	$filenametemp1 = "$full_path\\USB_${usb1}\\signal_$i.bin";
	$filenametemp2 = "$full_path\\USB_${usb2}\\signal_$i.bin";
	new $usb1, $filenametemp1;                    # open new file
	new $usb2, $filenametemp2;                    # open new file
	if($i==0){
		com $usb, 0, 254, 0; 
	}	
	tarry $filelength;
	tarry 0.75;                            		# wait for late packets
	shut $usb;                          		# close file
}
#just at the end
com $usb, 63, 73, 0b0000;
tarry 0.5;
com $usb, 0, 255, 0;
tarry 0.5;                		# clear  buffers


# print "Data taken completed...\n";
# print "\n";
}

##################################################################################################################

sub generatecsv {

$usb = shift;
$pmt = shift;

getpmtdata $usb, $pmt;

# print "Processing data for USB=$usb and PMT=$pmt\n";
# print "\n";

my $full_path_temp = "$full_path\\USB_${usb}\\PMT_${pmt}";

dir $full_path_temp;                              # set usb to (create and) write to this folder

$baselines = "$full_path\\USB_${usb}\\PMT_${pmt}\\baselines.dat";
$filename = "$full_path\\USB_${usb}\\signal";

for(my $i=0;$i<$newrunlength;$i++){
	system qq|cmd /C decode.pl "${filename}_${i}.bin" >> "${filename}.dec"|;
}

my $temp = "$filename.dec";
my $temp1 = "$full_path\\USB_${usb}\\PMT_${pmt}\\signal.dec";

system qq|cmd /C move "$temp" "$temp1"|;

signal (${full_path_temp},$temp1,$baselines,$pmt,${homedir});
#system qq|cmd /C signal.pl "${full_path_temp}" "$temp1" "$baselines" "@gain" "$pmt" ${homedir} >> log_${pmtserialnumber}_${date}.txt|;

}

######################################################################################################################

sub plotdatamb {

$usb = shift;
$pmtmb = shift;

getpmtdata $usb, $pmtmb;

print "Plotting data for USB=$usb and PMT=$pmtmb\n";

# make histograms
system qq|cmd /C histogram.pl "$full_path\\USB_${usb}\\PMT_${pmtmb}" "summary.csv" 1 "Pulse heights" >> log_${pmtserialnumber}_${date}.txt|;
system qq|cmd /C histogram.pl "$full_path\\USB_${usb}\\PMT_${pmtmb}" "summary.csv" 2 "Sigma" >> log_${pmtserialnumber}_${date}.txt|;
system qq|cmd /C histogram.pl "$full_path\\USB_${usb}\\PMT_${pmtmb}" "summary.csv" 3 "Hits by channel" >> log_${pmtserialnumber}_${date}.txt|;
system qq|cmd /C histogram.pl "$full_path\\USB_${usb}\\PMT_${pmtmb}" "summary.csv" 5 "Photoelectrons" >> log_${pmtserialnumber}_${date}.txt|;
system qq|cmd /C histogram.pl "$full_path\\USB_${usb}\\PMT_${pmtmb}" "summary.csv" 6 "Gains" >> log_${pmtserialnumber}_${date}.txt |;
system qq|cmd /C histogram.pl "$full_path\\USB_${usb}\\PMT_${pmtmb}" "summary.csv" 7 "Rate" >> log_${pmtserialnumber}_${date}.txt|;
system qq|cmd /C histogram.pl "$full_path\\USB_${usb}\\PMT_${pmtmb}" "summary.csv" 8 "Gain Constants" >> log_${pmtserialnumber}_${date}.txt|;
system qq|cmd /C histogram.pl "$full_path\\USB_${usb}\\PMT_${pmtmb}" "summary.csv" 9 "Normalized Hits by channel" >> log_${pmtserialnumber}_${date}.txt|;
}

####################################################################################################
sub dacadc {

$thresh=shift;

$dacdirectory = $homedir . "Scripts_mb\\dacsetting\\dac_${date}_${pmtserialnumber}_${HVsetting}.txt";

$newdacdirectory = $homedir . "Scripts_mb\\dacsetting\\newdac_${date}_${pmtserialnumber}_${HVsetting}.txt";

$dacfile = "$full_path\\PMT_${pmtmb}\\hist.csv";


my(@dline, @dval);
my @dcount;

#####################################
# M. Toups 9/18/2008
# Modify this script to find DAC/ADC conversion for all channels
# Added big for loop and made dindex an aray
######################################
my ($j, $k, $dflag);
my @dindex;
for(my $cnt =1; $cnt<=64; $cnt++) {
	$j=0;
	open IN, $dacfile or die $!;
	while(<IN>){
		chomp;
		s/#.*//;
		next if /^\s*$/;
		@dline = split ',';
		$dval[$j]=$dline[0];
		$dcount[$j]=$dline[$cnt]; # changed from $dline[36]
		$j++;
	}
	close IN;

	$dindex[$cnt]=1;
	$dflag=0;
	for $k (0..$j-2) {
		if(($dcount[$k] > 0) && ($dcount[$k+1] > 0) && ($dflag==0)) {
		$dindex[$cnt] = $k;
		$dflag = 1;
		}
	}
}
########################################

open OUT, ">$newdacdirectory";
if(-e $dacdirectory) {
    open IN, $dacdirectory or die $!;
    while(<IN>) {
        chomp;
		print OUT "$_\n"
    }
	close IN or die $!;
} 
print OUT "${thresh}";

#####################################
# M. Toups 9/18/2008
# Modify this script to find DAC/ADC conversion for all channels
# Changed output file
######################################
my $davg = 0;
for (my $dcnt =1; $dcnt<=64; $dcnt++) { 
	print OUT " $dval[$dindex[$dcnt]]"; 
	$davg+=$dval[$dindex[$dcnt]]/64.;
	} 
close OUT or die $!;
#######################################

system qq|cmd /C move "$newdacdirectory" "$dacdirectory"|;

#####################################
# M. Toups 9/18/2008
# Modify this script to find DAC/ADC conversion for all channels
# Changed this to average dac
######################################
print "DAC value of " . ${thresh} . " corresponds to an average ADC value of " . $davg . "\n"; #${dval}[${dindex}]
#########################################
}

######################################################################################################
sub dacadcfit {

my $dacdirectory1="\"\"\"\"$dacdirectory\"\"\"\"";

$dacdirectory1=~s/\\/\//g;

#####################################
# M. Toups 9/18/2008
# Modify this script to find DAC/ADC conversion for all channels
# Changed DACfit.C and definition of $ADCpe
######################################
our $ADCpe = $meangain/1.45e4; #our $ADCpe = $meanph/$meanpe;
system qq|cmd /C root.exe -b -q -l DACfit.C($dacdirectory1,$ADCpe) >> logfile_${pmtserialnumber}_${date}.txt|;
########################################

print "dacdirectory $dacdirectory\n";
open IN, $dacdirectory or die $!;
while(<IN>){
	chomp;
	next if /^\d+.*$/;
	if ($_ =~ /^#(.*)\s(.*)$/) {$p0 = $1; $p1=$2;}    # delete comments
	}
close IN;
print "p0 is $p0 and p1 is $p1\n";

open OUT, ">$newdacdirectory";
if(-e $dacdirectory) {
    open IN, $dacdirectory or die $!;
    while(<IN>) {
        chomp;
		print OUT "$_\n"
    }
	close IN or die $!;
} 
#compute
print OUT "## " . int(0.5 + $p0 + 1* $p1 * $ADCpe/3) . "\n";
print OUT "## " . int(0.5 + $p0 + 2* $p1 * $ADCpe/3) . "\n";
print OUT "## " . int(0.5 + $p0 + 3* $p1 * $ADCpe/3) . "\n";
print OUT "## " . int(0.5 + $p0 + 6* $p1 * $ADCpe/3) . "\n";
close OUT or die $!;

$dacvalue[0]=int(0.5 + $p0 + 1* $p1 * $ADCpe/3);
$dacvalue[1]=int(0.5 + $p0 + 2* $p1 * $ADCpe/3);
$dacvalue[2]=int(0.5 + $p0 + 3* $p1 * $ADCpe/3);
$dacvalue[3]=int(0.5 + $p0 + 6* $p1 * $ADCpe/3);

system qq|cmd /C move "$newdacdirectory" "$dacdirectory"|;

#camillo

#calling mysql database function update to store the gain constants and the dac threshold

	if($newpmtdatafound == 1){
	    print "Would you like to upload the characterization parameters into the mysql database (yes or no)?\n";
	
	    chomp(my $answer = <STDIN>);
	
	    if($answer eq "yes") {loadgainmysql($pmtserialnumber, $boardnumber, $HVsetting, $dacvalue[1]);};
    }
	else{
		loadgainmysql($pmtserialnumber, $boardnumber, $HVsetting, $dacvalue[1]);
	}


}

######################################################################################################

sub getavg {

my $sumfile = "summary.csv";
my $pmtmb = shift;
my $col = shift;

my @data;

# read summary file
if($pmtmb eq ""){
open IN, "$full_path\\$sumfile" or die $!;
}
else {
open IN, "$full_path\\PMT_${pmtmb}\\$sumfile" or die $!;
}

while(<IN>) {
    chomp;
    s/\s*#.*//;
    next if /^\s*$/;
    my @line = split /\s*,\s*/;
    my $d = $line[$col];
    push(@data, $d);
}
close IN;

# calculate average
my $n = @data;
exit 0 if !$n;
$average = 0;
for(@data) {$average += $_;}
$average = $average / $n;
my $colname = "";
if($col ==1 ) { $colname = "pulse height"; }
elsif($col ==2 ) { $colname = "pulse height sigma"; }
elsif($col ==3 ) { $colname = "hits per channel"; }
elsif($col ==4 ) { $colname = "hits per packet"; }
elsif($col ==5 ) { $colname = "number of photo-electrons"; }
elsif($col ==6 ) { $colname = "gain"; }
elsif($col ==7 ) { $colname = "rate per channel"; }
print "\n Average ${colname} is $average \n\n";
}


#####################################################################################################
sub writeonfile {

open OUT, ">$newdacdirectory";
if(-e $dacdirectory) {
    open IN, $dacdirectory or die $!;
    while(<IN>) {
        chomp;
		print OUT "$_\n"
    }
	close IN or die $!;
} 
#compute
print OUT "### " . (shift) . " " . (shift) . "\n";
close OUT or die $!;

system qq|cmd /C move "$newdacdirectory" "$dacdirectory"|;
}

###################################################################################################

sub checkentries {

	getavg($pmt, 3);

	$meanentries=$average;
	print "Checking....Number of entries is $meanentries\n";
	$repeat = 0;
	if($meanentries<.70*$runlength*$laserrate) {
		print "Warning: Number of entries ($meanentries) less than 70% of expected number.\n";
		#print "Enter a new DAC threshold (currently ${DACt}): ";
		print "Error: Consult the trouble shooting procedure. Press enter when finished ! \n\n";
		sendmail("Low number of entries $pmtserialnumber, $username");
		<STDIN>;
		#chomp($pmtdata[$usb][$pmt][3]=<STDIN>);
		$repeat = 1;
	}
}

###################################################################################################

sub writelog {

my $info1 = shift;
my $info2 = shift;

our $logfile = $homedir . "Scripts_mb\\pmtlogfile\\log_${date}_${pmtserialnumber}_${HVsetting}.txt";
our $newlogfile = $homedir . "Scripts_mb\\pmtlogfile\\log_${date}_${pmtserialnumber}_${HVsetting}.txt.new";

open OUT, ">$newlogfile";
if(-e $logfile) {
    open IN, $logfile or die $!;
    while(<IN>) {
        chomp;
		print OUT "$_\n"
    }
	close IN or die $!;
} 
print OUT $info1 . "  Run number:" . $info2 . "\n";
close OUT or die $!;

system qq|cmd /C move "$newlogfile" "$logfile"|;
}

########################################################################################################
sub adjustpulseheight {

my $meanpulseheight = shift;

my $max_iterations = 10;
my $iteration = 0;
my $target_pulse_height = 0;     # percent
my $spread_old = 10000;


again:

$equalization = "true";

initializefilestructure($usb);
initializeusb($usb, $pmt); 
takedatamb($usb, $runlength); 
generatecsv($usb, $pmt);
($usb, $pmt);
checkeff($pmt);

system qq|cmd /C adjust.pl "$full_path\\PMT_${pmtmb}" "$gaindirectory" "$meanpulseheight"|;           #program that adjust the gain !!!!

system qq|cmd /C histogram.pl "$full_path\\PMT_${pmtmb}" "summary.csv" 2 "Sigma"|;

$spread_pulse_height = `cmd /C histogram.pl "$full_path\\PMT_${pmtmb}" "summary.csv" 1 "Pulse heights"`;
$iteration++;

print "Spread(%): $spread_pulse_height     Iteration: $iteration\n\n";
if($iteration == 1){ $inispread = $spread_pulse_height; }
if(($iteration == 1) || (($iteration != $max_iterations) 
                        && ($spread_pulse_height > $target_pulse_height) 
                        && ($spread_pulse_height < $spread_old))) {
    $spread_old = $spread_pulse_height;
    goto again; 
}

if ($spread_pulse_height >= $spread_old) { print "Reach the minimum target spread...exiting \n"; }
system qq|cmd /C copy "$gaindirectory" "$savedgaindirectory"|;

getavg($pmt, 1);
$meanph = $average;
getavg($pmt, 5);
$meanpe = $average;

#####################################
# M. Toups 9/18/2008
# Modify this script to find DAC/ADC conversion for all channels
# Changed definition of $ADCpe
######################################
getavg($pmt, 6);
$meangain = $average; #$ADCpe = $meanph/$meanpe;
$ADCpe = $meangain/1.45e4;
getavg($pmt, 5);
####################################

print "Mean pulse height $meanph \n\n";

print "Mean number of pe $meanpe \n\n";


# #calling mysql database function update to store the gain constants

	# print "To load the gain constants into mysql please type the passwd (if you do not know the passwd or you do not want to load the constants into the mysql database just press enter.\n";
	
	# chomp($passwd = <STDIN>);
	
	# if($passwd eq "$password") {loadgainmysql($pmtserialnumber, $boardnumber, $HVsetting);};

	loadrunsummarymysql();
	movedata();
	$equalization = "false";
	$pmtdata[$usb][$pmt][4] = "yes";
	$mysqlload = 0;
}
######################################################################################################
sub checkeff {

#my $file = shift;
my $pmt3 = shift;
my $file = "$full_path\\USB_${usb}\\PMT_${pmt3}\\signal.dec";
my $TimeStampPerSecond = 1;  # Take into account the time interval between timestamps as well as the number of timestamps sent each time interval
my $countert = 0;
my $counterx = 0;
my $counterp = 0;
my $initial = 0;
my $final = 0;
my $flag = 0;
my $dummy = 0;

open IN, "$file" or die $!;
while(<IN>) {
	if(/^(t,)(\d+)/) {
		$dummy = $2;
		if(!$flag) {
			$flag=1;
			$initial=$dummy;
		}
		elsif($flag) {
			$final=$dummy;
		}
		$countert++;
	}
	elsif(/^(x,)/) {$counterx++;}
	elsif(/^(p,)/) {$counterp++;}
}
close IN;

$dummy = $final-$initial+1;
$dummy *= $TimeStampPerSecond;

open OUT, ">>$full_path\\USB_${usb}\\PMT_${pmt3}\\Timestamps.txt" or die $!;
print OUT "\nData for $file\n";
print OUT "Initial time:               $initial\n";
print OUT "Final time:                 $final\n";
print OUT "Total timestamps expected:  $dummy\n";
print OUT "Total timestamps found:     $countert\n";
print OUT "Total x packets found:      $counterx\n";
print OUT "Total p packets found:      $counterp\n";

if($counterx >= 3) { sendmail("Found $counterx x packets in file $file."); }

if($dummy-$countert) {
	$final = $counterx/($dummy-$countert)*100;
}
else {
	$final = "N/A";
}
$initial = $counterx/($counterp+$countert)*100;
$dummy = ($dummy-$countert)/$dummy*100;

if($dummy > 15.0)  { sendmail("Percentage timestamps lost is larger than expected at $dummy percent in file $file."); }
print OUT "Percentage timestamps lost: $dummy\n\n";
print OUT "Extra packets as a percentage of:\n\n";
print OUT "Timestamps lost:           $final\n";
print OUT "Time + data packets found:  $initial\n";
close OUT;

}

#######################################################################################

sub loadgainmysql {

$pmtserialnumber = shift;
$boardnumber = shift;
$HVsetting = shift;
my $dac23pe = shift;

print "Loading gain constants and dac threshold into MySQL database for PMT=$pmtserialnumber, board=$boardnumber, HV=$HVsetting \n\n";

eval {
  $dsn = "dbi:mysql:database=$database;host=$hostname";

  $dbh = DBI->connect($dsn, $user, $password);

  $drh = DBI->install_driver("mysql");

  $sth = $dbh->prepare("SELECT PMT_Serial FROM onlinenew");
  
  $sth->execute();
  while (my $ref = $sth->fetchrow_arrayref()) {
    # if($ref ->[0] eq "$pmtserialnumber") { 
		# print "Found info in mysql database for pmt = $ref->[0] \n"; };
	# }
  $sth->finish();
  
	$gaindirectory = $homedir . "Scripts_mb\\marocgainsetting\\gain_PA${pmtserialnumber}_${boardnumber}_${HVsetting}.txt";
    #print $gaindirectory;
	if(-e $gaindirectory) {
        open IN, $gaindirectory or die $!;
		#print "file with gain constant found for $pmtserialnumber\n";
        while(<IN>) {
            chomp; s/#.*//;    # delete comments
            next if /^\s*$/;    # skip blank lines
            my($channel, $value) = split /\s+/;
            my $happy = $value;
			if($cornercorrection eq "on"){
				if($channel == 1 || $channel == 8 || $channel == 57 || $channel == 64){
					$happy = $happy * 0.6;
				}
			}
			#code to alter a row in mysql
			my $index = "gain" . ${channel};
			$dbh->do("UPDATE online SET ${index}=${happy} WHERE PMT_serial=$pmtserialnumber");
        }
        close IN or die $!;
		$dbh->do("UPDATE online SET board_number=$boardnumber WHERE PMT_serial=$pmtserialnumber");
		$dbh->do("UPDATE online SET HV=$HVsetting WHERE PMT_serial=$pmtserialnumber");
		$dbh->do("UPDATE online SET DAC_threshold=$dac23pe WHERE PMT_serial=$pmtserialnumber");
		$dbh->do("UPDATE online SET use_maroc2gain='yes' WHERE PMT_serial=$pmtserialnumber");
	}

  $dbh->disconnect();
};

print "Failed to load gain constants into MySQL: $@\n" if $@;
#if($@ eq ""){print "Gain constants imported correctly into MySQL \n";};

sendmail("Gain constants for for PMT=$pmtserialnumber, board=$boardnumber, HV=$HVsetting updated MySQL database");

}
}

#####################################################################################

sub movedata {

system qq|cmd /C copy move.txt movecopy.txt|;

open IN, "move.txt" or die $!;
open OUT, ">movecopy.txt" or die $!;

while(<IN>) {
	if(/mkdir /) {                                 
        print OUT "mkdir Run_${run}\n";        	
    }
	elsif (/cd \/misc\/data3\/scratch\/DATA/) {                                 
        print OUT "cd /misc/data3/scratch/DATA/${hostnamepc}\n";        	
    } 
    elsif (/cd Run_/) {                                 
        print OUT "cd Run_${run}\n";        	
    } 
    elsif (/put /) {                                 
        print OUT "put $full_path\\*.*\n";        	
    } 
    else  {
        print OUT $_;
    }
}
close IN;
close OUT;

system qq|cmd /C move movecopy.txt move.txt|;
#print "Modifying move.txt...\n";
system qq|cmd /C WinSCP.exe /console /script=move.txt > logwinscp.txt|;
#print "DATA moved to riverside \n";

#copyrunsummarywebpage();		#file on webpage uploaded

}

###########################################################################

sub loadrunsummarymysql {

if($equalization eq "true"){ $comments = $comments . "  after equalization" ; };

eval {
  $dsn = "dbi:mysql:database=$database;host=$hostname";

  $dbh = DBI->connect($dsn, $user, $password);

  $drh = DBI->install_driver("mysql");  
		
  if(${usemaroc2gainconstantsmb} eq "no"){
			($gain[1], $gain[2], $gain[3], $gain[4], $gain[5], $gain[6], $gain[7], $gain[8], $gain[9], 
			$gain[10], $gain[11], $gain[12], $gain[13], $gain[14], $gain[15], $gain[16], $gain[17], $gain[18], $gain[19], $gain[20], 
			$gain[21], $gain[22], $gain[23], $gain[24], $gain[25], $gain[26], $gain[27], $gain[28], $gain[29], $gain[30], $gain[31], 
			$gain[32], $gain[33], $gain[34], $gain[35], $gain[36], $gain[37], $gain[38], $gain[39], $gain[40], $gain[41], $gain[42], 
			$gain[43], $gain[44], $gain[45], $gain[46], $gain[47], $gain[48], $gain[49], $gain[50], $gain[51], $gain[52], $gain[53], 
			$gain[54], $gain[55], $gain[56], $gain[57], $gain[58], $gain[59], $gain[60], $gain[61], $gain[62], $gain[63], $gain[64]) 
			= (16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,
			16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,
			16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16);
		}
		
		if($pmtserialnumber eq "none"){
			$dbh->do("INSERT INTO runsummary_$hostnamepclc VALUES ('Run_${run}', $boardnumber, '$pmtserialnumber', $pmt, '$HVsetting', $DACt,  
			'$gate', '$comments', $hdelay, $filelength, $runlength, '$usemaroc2gainconstantsmb', 
			$gain[1], $gain[2], $gain[3], $gain[4], $gain[5], $gain[6], $gain[7], $gain[8], $gain[9], 
			$gain[10], $gain[11], $gain[12], $gain[13], $gain[14], $gain[15], $gain[16], $gain[17], $gain[18], $gain[19], $gain[20], 
			$gain[21], $gain[22], $gain[23], $gain[24], $gain[25], $gain[26], $gain[27], $gain[28], $gain[29], $gain[30], $gain[31], 
			$gain[32], $gain[33], $gain[34], $gain[35], $gain[36], $gain[37], $gain[38], $gain[39], $gain[40], $gain[41], $gain[42], 
			$gain[43], $gain[44], $gain[45], $gain[46], $gain[47], $gain[48], $gain[49], $gain[50], $gain[51], $gain[52], $gain[53], 
			$gain[54], $gain[55], $gain[56], $gain[57], $gain[58], $gain[59], $gain[60], $gain[61], $gain[62], $gain[63], $gain[64])");
		}
		else{
			$dbh->do("INSERT INTO runsummary_$hostnamepclc VALUES ('Run_${run}', $boardnumber, 'PA$pmtserialnumber', $pmt, '$HVsetting', $DACt,  
			'$gate', '$comments', $hdelay, $filelength, $runlength, '$usemaroc2gainconstantsmb', 
			$gain[1], $gain[2], $gain[3], $gain[4], $gain[5], $gain[6], $gain[7], $gain[8], $gain[9], 
			$gain[10], $gain[11], $gain[12], $gain[13], $gain[14], $gain[15], $gain[16], $gain[17], $gain[18], $gain[19], $gain[20], 
			$gain[21], $gain[22], $gain[23], $gain[24], $gain[25], $gain[26], $gain[27], $gain[28], $gain[29], $gain[30], $gain[31], 
			$gain[32], $gain[33], $gain[34], $gain[35], $gain[36], $gain[37], $gain[38], $gain[39], $gain[40], $gain[41], $gain[42], 
			$gain[43], $gain[44], $gain[45], $gain[46], $gain[47], $gain[48], $gain[49], $gain[50], $gain[51], $gain[52], $gain[53], 
			$gain[54], $gain[55], $gain[56], $gain[57], $gain[58], $gain[59], $gain[60], $gain[61], $gain[62], $gain[63], $gain[64])");
		}

  $dbh->disconnect();
};

 print "Failed to import into MySQL info for this run: $@\n" if $@;

 #if($@ eq ""){ print "Info for this run imported into the mysql database \n";};
  
}

######################################################################################################################

sub copyrunsummarywebpage {

eval {

$dsn = "dbi:mysql:database=$database;host=$hostname";

$dbh = DBI->connect($dsn, $user, $password);

$drh = DBI->install_driver("mysql");

my $summaryfile = "$homedir\\Scripts_mb\\macro\\run_summary_${hostnamepc}.txt";

open OUT, ">$summaryfile" or die $!;
print OUT "Run_number \t PMT_serial_number \t\t Board_number \t HV \t DAC_threshold \t file_length(s) \t run_length(s) \t gate \t maroc2_gc_used \t comments \n\n\n";	

 $sth = $dbh->prepare("SELECT Runnumber, PMT_serial, PMT_board_serial, HV, DAC_threshold, file_length, run_length, gate, use_maroc2gain, comments FROM runsummary_${hostnamepclc}");

 $sth->execute();
  while (my $ref = $sth->fetchrow_arrayref()) {
	  if($ref->[1] eq "none"){
		print OUT "$ref->[0] \t\t $ref->[1] \t\t\t $ref->[2] \t\t $ref->[3] \t\t $ref->[4] \t\t $ref->[5] \t\t $ref->[6] \t\t $ref->[7] \t\t $ref->[8] \t\t $ref->[9] \n";
	  }
	  else{
	    print OUT "$ref->[0] \t\t $ref->[1] \t\t $ref->[2] \t\t $ref->[3] \t\t $ref->[4] \t\t $ref->[5] \t\t $ref->[6] \t\t $ref->[7] \t\t $ref->[8] \t\t $ref->[9] \n\n";    
	  }
	}
  $sth->finish();
    
$dbh->disconnect();
  
close OUT or die $!;

system qq|cmd /C copy move.txt movecopy.txt|;

open IN, "move.txt" or die $!;
open OUT, ">movecopy.txt" or die $!;

while(<IN>) {
    if (/put /) {                                 
        print OUT "put run_summary_${hostnamepc}.txt \n";        	
    } 
    else  {
        print OUT $_;
    }
}
close IN;
close OUT;

system qq|cmd /C move movecopy.txt move.txt|;

system qq|cmd /C WinSCP.exe /console /script=move.txt > logwinscp.txt|;

};

 print "Uploading file on webpage failed: $@\n" if $@;
 #if($@ eq ""){print "File on the webpage uploaded \n";};
 
}

#####################################################################################

sub sendmail {

my $thisServer = "$hostnamepc.nevis.columbia.edu";	#the hostname of the server this script is installed on
my $smtpServer = "smtp.nevis.columbia.edu";	#the smtp server we want to connect to
my $debug = 0;		

my $from = "dcdaq\@nevis.columbia.edu";
my $to = "mariani\@nevis.columbia.edu";
my $cc = "mtoups\@nevis.columbia.edu";
my $body = shift;
my $subject = "MySQL database: online table updated";

#send an email
	if(my $smtp = Net::SMTP->new($smtpServer, Hello => $thisServer, Debug => $debug, Timeout => 10)){
		$smtp -> auth("dcdaq","doublechooz");	#trying to authenticate
		$smtp -> mail($from);   #send server sending email address
		$smtp -> to($to);  #send server recieving email address
		$smtp -> cc($cc);
		$smtp -> data();
		$smtp->datasend("From: $from \n");
		$smtp->datasend("To: $to \n");
		$smtp->datasend("Cc: $cc \n");
		$smtp -> datasend("Subject: $subject");
		$smtp -> datasend("\n");
		$smtp->datasend($body);
		$smtp -> dataend();
		$smtp -> quit();
	}
	else{
		print "ERROR: Could not connect to SMTP server\n";
	}	
###############################################################################
}

sub writecharacterizationsummary {

getpmtdata($usb,$pmt);
my ($i,$channel);
my $pmtcharacterizationsummaryfile = $homedir . "Scripts_mb\\pmt_characterization_summary.txt";
my $newpmtcharacterizationsummaryfile = $homedir . "Scripts_mb\\pmt_characterization_summary_1.txt";

open OUT, ">$newpmtcharacterizationsummaryfile";
if(-e $pmtcharacterizationsummaryfile) {
    open IN, $pmtcharacterizationsummaryfile or die $!;
    while(<IN>) {
        chomp;
		print OUT "$_\n"
    }
	close IN or die $!;
} 

print OUT "PA$pmtserialnumber \t $pmt \t $HVsetting \t $ADCpe \t $p0 \t $p1 \t $noiseratePE[0] \t $noiseratePE[1] \t $noiseratePE[2] \t $noiseratePE[3] \t $date \n";
close OUT or die $!;

system qq|cmd /C move "$newpmtcharacterizationsummaryfile" "$pmtcharacterizationsummaryfile"|;

# load  info into the mysql database to have info on the pmt characterization

eval {
  $dsn = "dbi:mysql:database=$database;host=$hostname";

  $dbh = DBI->connect($dsn, $user, $password);

  $drh = DBI->install_driver("mysql");  
		
  if(${usemaroc2gainconstantsmb} eq "no"){
			($gain[1], $gain[2], $gain[3], $gain[4], $gain[5], $gain[6], $gain[7], $gain[8], $gain[9], 
			$gain[10], $gain[11], $gain[12], $gain[13], $gain[14], $gain[15], $gain[16], $gain[17], $gain[18], $gain[19], $gain[20], 
			$gain[21], $gain[22], $gain[23], $gain[24], $gain[25], $gain[26], $gain[27], $gain[28], $gain[29], $gain[30], $gain[31], 
			$gain[32], $gain[33], $gain[34], $gain[35], $gain[36], $gain[37], $gain[38], $gain[39], $gain[40], $gain[41], $gain[42], 
			$gain[43], $gain[44], $gain[45], $gain[46], $gain[47], $gain[48], $gain[49], $gain[50], $gain[51], $gain[52], $gain[53], 
			$gain[54], $gain[55], $gain[56], $gain[57], $gain[58], $gain[59], $gain[60], $gain[61], $gain[62], $gain[63], $gain[64]) 
			= (16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,
			16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,
			16,16,16,16,16,16,16,16,16,16,16,16,16,16,16,16);
		}
	else{
		$i=0;
		open IN1, ${gaindirectory} or die $!;
		while(<IN1>) {
	        chomp;
			$i++;
			($channel,$gain[$i])= split /\s+/;
			}
		close IN1;
	}
		$dbh->do("INSERT INTO pmt_characterization_summary VALUES ('PA$pmtserialnumber', $boardnumber, $HVsetting, $p0, $p1, $ADCpe,
		$noiseratePE[0], $noiseratePE[1], $noiseratePE[2], $noiseratePE[3], '$date',
		$gain[1], $gain[2], $gain[3], $gain[4], $gain[5], $gain[6], $gain[7], $gain[8], $gain[9], 
		$gain[10], $gain[11], $gain[12], $gain[13], $gain[14], $gain[15], $gain[16], $gain[17], $gain[18], $gain[19], $gain[20], 
		$gain[21], $gain[22], $gain[23], $gain[24], $gain[25], $gain[26], $gain[27], $gain[28], $gain[29], $gain[30], $gain[31], 
		$gain[32], $gain[33], $gain[34], $gain[35], $gain[36], $gain[37], $gain[38], $gain[39], $gain[40], $gain[41], $gain[42], 
		$gain[43], $gain[44], $gain[45], $gain[46], $gain[47], $gain[48], $gain[49], $gain[50], $gain[51], $gain[52], $gain[53], 
		$gain[54], $gain[55], $gain[56], $gain[57], $gain[58], $gain[59], $gain[60], $gain[61], $gain[62], $gain[63], $gain[64])");	

  $dbh->disconnect();
};

 print "Failed to import into MySQL info for this run: $@\n" if $@;

 #if($@ eq ""){ print "Info for this run imported into the mysql database \n";};
}



sub printsummary {

getpmtdata($usb,$pmt);
my $runnum = shift;
my $usb = shift;
my $pmt = shift;

our $dactotal;
our $dacmax;
our $dacmaxindex;

my $firstdacnoiserun = $homedir . "DATA\\Run_${runnum}\\USB_${usb}\\PMT_${pmt}\\summary.csv";
my @dacdata;

if(-e $firstdacnoiserun) {
    open IN, $firstdacnoiserun or die $!;
    while(<IN>) {
        chomp;
		s/\s*#.*//;
		next if /^\s*$/;
		my @dacline = split /\s*,\s*/;
		my $dacd = $dacline[7]; # get rate column
		push(@dacdata, $dacd);
    }
	close IN or die $!;
	
	my $dacn = @dacdata;
	if(!$dacn) {
		sendmail("Coud not check PMT $pmtserialnumber for specifications.  Please review data in file $firstdacnoiserun.\n");
		exit 0;
	}
	$dactotal=0;
	$dacmax = 0;
	$dacmaxindex = 0;
	#print "Analyzing noise rates...\n";
	for(my $dacm = 1; $dacm<=$dacn; $dacm++) { 
		$dactotal+=$dacdata[$dacm-1];
		if($dacdata[$dacm-1] > $dacmax) {
			$dacmax = $dacdata[$dacm-1];
			$dacmaxindex = $dacm;
		}
		if($dacdata[$dacm-1] >= 400) {
			print "Channel $dacm exceeds specifications at 1/3 p.e. with a noise rate of $dacdata[$dacm-1].\n";
			print "An expert will be notified.\n";
			sendmail("PMT $pmtserialnumber, Channel $dacm exceeds specifications at 1/3 p.e. with a noise rate of $dacdata[$dacm-1].  Please review data in file $firstdacnoiserun.\n");
		}
	}
	# print "Channel $dacmaxindex is the noisiest pixel at 1/3 p.e. with a noise rate of $dacdata[$dacmaxindex-1].\n";
	# print "Total noise rate for this tube at 1/3 p.e. is $dactotal Hz.\n";
	if($dactotal>=3000) { 
		print "This PMT fails total noise specifications at 1/3 p.e.  An expert will be notified.\n";
		sendmail("PMT $pmtserialnumber fails total noise specifications at 1/3 p.e. with total noise rate of $dactotal Hz.  Please review data in file $firstdacnoiserun.\n");
	}
	if($phratio>4.0 or $gainratio>4.0) {
		print "This PMT fails anode uniformity specifications at 800 V.  An expert will be notified.\n";
		sendmail("PMT $pmtserialnumber fails anode uniformity specifications at 800 V.  Pulse height ratio: ${phratio}, Gain ratio: ${gainratio}.\n");	
	}
}
else {
	print "Could not find file $firstdacnoiserun to retrieve summary of noise data at 1/3 p.e.\n";
	sendmail("Coud not check PMT $pmtserialnumber for specifications.  Please review data in file $firstdacnoiserun.\n");
} 

my $tempnoiserate = $noiseratePE[0] * 64.;
my $tempnoisestrate = $dacdata[$dacmaxindex-1] * 10.;
print "\n\n---------------------------------------------------------------------\n\n";
print "Summary of the current characterization \n\n";
print "--- PMT number ${pmtserialnumber} = \n\n";
print "--- Ratio of Max to Min Gain at 800V - ${gainratio} \n\n";
print "--- Ratio of Max to Min Pulse Heights at 800V - ${phratio} \n\n";
print "--- Final HV value = $HVsetting V \n\n";
print "--- Initial spread = ${inispread} % \n\n";
print "--- Final spread = ${spread_pulse_height} % \n\n";
print "--- Noisiest Channel = $dacmaxindex \n\n";
print "--- Noise rate of Noisiest Channel = $tempnoisestrate Hz\n\n";
print "--- Noise rate per pixel = $noiseratePE[0] Hz\n\n";
print "--- Total noise = $tempnoiserate Hz. \n\n";
print "---------------------------------------------------------------------\n";	
} #end of printsummary


#####################################################################################################

sub getratio800 {

my $sumfile = "summary.csv";
my $pmtmb = shift;
my $col = shift;

my @data;

# read summary file
if($pmtmb eq ""){
open IN, "$full_path\\$sumfile" or die $!;
}
else {
open IN, "$full_path\\PMT_${pmtmb}\\$sumfile" or die $!;
}

while(<IN>) {
    chomp;
    s/\s*#.*//;
    next if /^\s*$/;
    my @line = split /\s*,\s*/;
    my $d = $line[$col];
    push(@data, $d);
}
close IN;

# calculate max/min ratio
my $max=0;
my $min=1e10;
my $n = @data;
exit 0 if !$n;
$maxminratio=0;
for(@data) {
	if($_<=$min) {
		$min = $_;
		}
	if($_>=$max) {
		$max = $_;
		}
	
	}
if($min) { $maxminratio = $max/$min; }
else { $maxminratio = 0; }
my $colname = "";
if($col ==1 ) { $colname = "pulse height"; }
elsif($col ==2 ) { $colname = "pulse height sigma"; }
elsif($col ==3 ) { $colname = "hits per channel"; }
elsif($col ==4 ) { $colname = "hits per packet"; }
elsif($col ==5 ) { $colname = "number of photo-electrons"; }
elsif($col ==6 ) { $colname = "gain"; }
elsif($col ==7 ) { $colname = "rate per channel"; }
print "\n The ratio of the maximum to minimum for the ${colname} is $maxminratio \n\n";
}


#####################################################################################################


sub producentuple {

my $folder = "$full_path\\USB_${usb}";

my $ext = "${homedir}\\ROOTntuple\\ProcessOnWin";
print "$full_path\n";
print "$ext\n";
print "$folder\n";

my ($binfile,$tempfile);

system qq|cmd /C decode.pl $folder\\baselines.bin > $folder\\baselines.tmp|;


my @files = <${folder}\\signal_*.bin>;
print "Found " . ($#files+1) . " files.\n";    
foreach $tempfile (@files){
    $binfile = $tempfile;
    $tempfile =~ s|bin$|tmp|g;
	my $baseline_file = "$folder\\baselines.tmp";
	print "\nDecoding...\n";
	system qq|cmd /C ${ext}\\decode.pl $binfile > $tempfile|;	
	print "$tempfile\n";
	$binfile =~ s/\\/\\\\/g;
	$tempfile =~ s/\\/\\\\/g;
	$folder =~ s/\\/\\\\/g;
	$baseline_file =~ s/\\/\\\\/g;
	print "$tempfile\n";
    

    $binfile =~ s|bin$|root|g;
    print "Counting...\n";
    system qq|cmd /C ${ext}\\Count.pl $folder $tempfile 1|;
    print "Building NTuple...\n";
    system qq|cmd /C root.exe -l -b -q ${ext}\\ChipProgram.C(""""$tempfile"""","""""$baseline_file"""",""""$binfile"""")|;
	$tempfile =~ s/\\\\/\\/g;
	$folder =~ s/\\\\/\\/g;
	$baseline_file =~ s/\\\\/\\/g;
    system qq|cmd /C del $tempfile|;
}

system qq|cmd /C del $folder\\baselines.tmp|;   
system qq|cmd /C ${ext}\\Xpackets.pl ${folder} Timestamps.txt|;

}

#####################################################################################################
sub signal {

my $nbins = 100;        # number of bins in histograms
my $gain_c = 1.45e4;    # constant used in gain calculation

my $dir = shift;
my $file = shift;
my $baselines = shift;

my $pmt_board = shift;
#my $laserrate = shift;
my $homedir = shift;

my $stefan = "Stefan Simion's Empty Histogram (tm). Formerly known as channel %d.";

my($x, $f, $i, $j);
my(@sig, @sig_dev, @base, @base_dev, @hist, @hist2, @hits_pc, @bins, @title, @photo, @gain_pmt, @mean_norm, @rate_norm);
for $i (1..64) {
    $sig[$i] = 0;
    $sig_dev[$i] = 0;
    $base[$i] = 0;
    $base_dev[$i] = 0;
    $hits_pc[$i] = 0;
    $photo[$i] = 0;
    $gain_pmt[$i] = 0;
    $hist[0]{$i} = 0;
	$mean_norm[$i] = 0;
	$rate_norm[$i] = 0;
}

# get "typical module" data
print "Getting data for typical module...\n";

if(defined($homedir)) {
    open IN, "${homedir}\\Scripts_mb\\macro\\normalized.txt" or goto nohomedir;
    while(<IN>) {
        chomp;
        next if /^\s*$/;
		next if /^#/;
        my($i, $m, $d) = split ",";
        $mean_norm[$i] = $m;
        $rate_norm[$i] = $d;
    }
    close IN or die $!;
}

nohomedir:

# get baselines
print "Getting baselines...\n";

if(defined $baselines) {
    open IN, $baselines or die $!;
    while(<IN>) {
        chomp;
        next if /^\s*$/;
        my($i, $m, $d) = split ",";
        $base[$i] = $m;
        $base_dev[$i] = $d;
    }
    close IN or die $!;
}

# parse data file

print "Getting signal file...$file\n";
# parse data file
open IN, "cmd /C unpack.pl \"$file\" \"$pmt_board\" |" or die $!;
my ($first, $last, $count, $countovf);
$count = 0;
$last = 0;
$first = 0;
$countovf = 0;

while(<IN>) {
    chomp;
    my ($c, $d, $t) = split ",";
    if($count == 0) { 
        $first = $t; 
        $count++;
    }
	if($t < $last){
	  $countovf++;
	}
    $last = $t;
    if($c == 0) {
        $hist[$c]{$d}++;            # $hist[0] = hits per packet  
    } else {
        $d -= $base[$c];
        $hist[$c]{$d}++;
        $hits_pc[$c]++;
    }
}
my $rate;
if($sync eq 'on') { $rate = 1/$runlength if $runlength; }
else { #only works if sync pulse is OFF!
#print "number of ovf $countovf\n";
$countovf = $countovf * 2**32;
#print "number of ovl after multiply it: $countovf \n";
$rate = ($last + $countovf - $first) * 16e-9;
#print "rate=${rate}, last packet = ${last}, first packet =${first}\n";
$rate = 1 / $rate if $rate;
}
#if(${rate} < 0) {${rate} = -${rate};}
close IN or die $!;

#$dir = "$dir\\$pmt_board";
    
# calculate
print "Calculating...\n";
my($min, $max, $max_c) = (10000, -10000, -10000);
for $i (1..64) {
    my($avg, $dev) = (0, 0);
    my $n = $hits_pc[$i];
    next if $n == 0;
    while(($x, $f) = each(%{$hist[$i]})) {
        $avg += $f * $x;
        $min = $x if $x < $min;
        $max = $x if $x > $max;
    }
    $avg = $avg / $n;
    while(($x, $f) = each(%{$hist[$i]})) { $dev += $f * (($x - $avg) ** 2); }
    $dev = sqrt($dev / $n);
    $sig[$i] = $avg;
    $sig_dev[$i] = $dev;
    if($sig_dev[$i]) { $photo[$i] = ($sig[$i] / $sig_dev[$i]) ** 2; }
    if($photo[$i]) { $gain_pmt[$i] = $gain_c * $sig[$i] / $photo[$i]; }
}

# histogram hits
print "Histogramming...\n";
my $wbins = ($max - $min) / $nbins;
$wbins = 1 if $wbins == 0;
$min -= 0.0001 * $wbins;
$max += 0.0001 * $wbins;
#$min = -20; $max = 180; $nbins = 200;    ###################################
$wbins = ($max - $min) / $nbins;
for (0..$nbins-1) { push @{$hist2[0]}, $min + ($_ + 0.5) * $wbins; }
for $i (1..64) {
    for (0..$nbins-1) { $bins[$_] = 0; }
    while(($x, $f) = each(%{$hist[$i]})) { $bins[int(($x - $min) / $wbins)] += $f; }
    if($hits_pc[$i] == 0) { 
        $bins[0] += 0.00001; 
        $title[$i] = sprintf($stefan, $i);
    } else {
        $title[$i] = sprintf("Channel %02d    Mean = %.3f    Std Dev = %.3f    Total = %d",
            $i, $sig[$i], $sig_dev[$i], $hits_pc[$i]);
    }
    for (0..$nbins-1) { 
        push @{$hist2[$i]}, $bins[$_]; 
        $max_c = $bins[$_] if $bins[$_] > $max_c;
    }
}

# raw data file
open OUT, ">$dir\\raw.csv" or die $!;
for $i (1..64) {
    print OUT "# Channel $i\n";
    if($hits_pc[$i] == 0) { print OUT "$min,0\n$max,0\n"; }
    else { while(($x, $f) = each(%{$hist[$i]})) { print OUT "$x,$f\n"; } }
    print OUT "\n\n";
}
close OUT or die $!;

# histogram file
open OUT, ">$dir\\hist.csv" or die $!;
print OUT "#x,", (join ",", (1..64)), "\n";
for $j (0..$nbins-1) {
    print OUT $hist2[0][$j];
    for $i (1..64) { print OUT ",$hist2[$i][$j]"; }
    print OUT "\n";
}
close OUT or die $!;

# summary file
open OUT, ">$dir\\summary.csv" or die $!;
print OUT "#channel,mean,dev,hits_per_channel,hits_per_packet,photo_e,gain_pmt,rate_per_channel,gain_constant,normalized_rate\n";
for $i (1..64) {
    my $tmp1 = $hits_pc[$i]*$rate;
	my $tmp2 = 0;
	if($rate_norm[$i]) { $tmp2 = $tmp1/$rate_norm[$i]; }
    print OUT "$i,$sig[$i],$sig_dev[$i],$hits_pc[$i],$hist[0]{$i},$photo[$i],$gain_pmt[$i],$tmp1,$gain[$i],$tmp2\n";
}
close OUT or die $!;

# 2d summary
open OUT, ">$dir\\summary_2d.csv" or die $!;
print OUT "#x,y,mean,dev,hits_per_channel,hits_per_packet,photo_e,gain_pmt,norm_mean\n";
for $i (0..8) {
    for $j (0..8) {
        my $xx = $i + 0.5;
        my $yy = $j + 0.5;
        my $c = 8 * $i + $j + 1;
        if($c <= 64) {
			my $tmp3 = 0;
			if($mean_norm[$c]) { $tmp3 = $sig[$c]/$mean_norm[$c]; }		
            print OUT "$xx,$yy,$sig[$c],$sig_dev[$c],$hits_pc[$c],$hist[0]{$c},$photo[$c],$gain_pmt[$c],$tmp3\n";
        } else {
            print OUT "$xx,$yy,0,0,0,0,0,0,0\n";
        }
    }
    print OUT "\n";
}
close OUT or die $!;

# 2d histograms
open OUT, ">$dir\\hist_2d.csv" or die $!;
print OUT "#x,y,z\n";
for $i (0..64) {
    for $j (0..$nbins) {
        my $xx = $i + 0.5;
        my $yy = $min + ($j + 0.5) * $wbins;
        if(($i < 64) && ($j < $nbins)) {
            print OUT "$xx,$yy,$hist2[$i + 1][$j]\n";
        } else {
            print OUT "$xx,$yy,0\n";
        }
    }
    print OUT "\n";
}
close OUT or die $!;

# change min and max so we can use them as ranges
$min += 0.5 * $wbins;
$max -= 0.5 * $wbins;

# generate plots
$dir =~ s/\\/\\\\/g;
print "Plotting...\n";
open PLOT, ">plot.gp" or die $!;
print PLOT qq|
    set grid
    set datafile separator ","
    set term png size 1024,768
|;

# summary
print PLOT qq|
    set title "ADC pulse heights"
    set xlabel "Channel #"
    set ylabel "ADC counts"
    set autoscale
    set xrange [0.5:64.5]
    set yrange [$min:*]
    set output "${dir}\\\\mean.png"
    plot "${dir}\\\\summary.csv" using 1:2 title "Mean" with histeps

    set title "ADC sigma"
    set xlabel "Channel #"
    set ylabel "ADC counts"
    set autoscale
    set xrange [0.5:64.5]
    set yrange [0:*]
    set output "${dir}\\\\sigma.png"
    plot "${dir}\\\\summary.csv" using 1:3 title "Sigma" with histeps

    set title "Hits by channel"
    set xlabel "Channel #"
    set ylabel "Count"
    set autoscale
    set xrange [0.5:64.5]
    set yrange [0:*]
    set output "${dir}\\\\per_channel.png"
    plot "${dir}\\\\summary.csv" using 1:4 title "Hits" with histeps
    
    set title "Hits in ADC packet"
    set xlabel "Number"
    set ylabel "Count"
    set autoscale
    set xrange [0.5:64.5]
    set yrange [0:*]
    set output \"${dir}\\\\per_packet.png\"
    plot "${dir}\\\\summary.csv" using 1:5 title "Hits" with histeps

    set title "Photoelectrons"
    set xlabel "Channel #"
    set ylabel "Number"
    set autoscale
    set xrange [0.5:64.5]
    set yrange [0:*]
    set output "${dir}\\\\photo_e.png"
    plot "${dir}\\\\summary.csv" using 1:6 title "Photoelectrons" with histeps

    set title "Gain"
    set xlabel "Channel #"
    set ylabel "Number"
    set autoscale
    set xrange [0.5:64.5]
    set yrange [0:*]
    set output "${dir}\\\\gain.png"
    plot "${dir}\\\\summary.csv" using 1:7 title "Gain" with histeps
        
    set title "Data acquisition rate per channel"
    set xlabel "Channel #"
    set ylabel "Rate [Hz]"
    set autoscale
    set xrange [0.5:64.5]
    set yrange [0:*]
    set output "${dir}\\\\rate_per_channel.png"
    plot "${dir}\\\\summary.csv" using 1:8 title "Rate (Hz)" with histeps

    set title "Maroc Gain Constants"
    set xlabel "Channel #"
    set ylabel "Number"
    set autoscale
    set xrange [0.5:64.5]
    set yrange [0:42]
    set output "${dir}\\\\gainconst.png"
    plot "${dir}\\\\summary.csv" using 1:9 title "Maroc Gain Constants" with histeps

    set title "Data acquisition rate per channel (normalized)"
    set xlabel "Channel #"
    set ylabel "Normalized Rate"
    set autoscale
    set xrange [0.5:64.5]
    set yrange [0:*]
    set output "${dir}\\\\rate_per_channel_normalized.png"
    plot "${dir}\\\\summary.csv" using 1:10 title "Normalized Rate" with histeps

|;


# histograms - individual channels
for $i (1..64) {
    my $minus1 = $i - 1;
    my $plus1 = $i + 1;
    my $tmp = ($i < 10) ? "0$i" : $i;
    print PLOT qq|
        set title "$title[$i]"
        set autoscale
        set yrange [0:$max_c]
        set xlabel "ADC counts"
        set ylabel "Count"
        set output "${dir}\\\\adc_$tmp.png"
        plot "${dir}\\\\hist.csv" using 1:$plus1 title "Histogram" with histeps, \\
             "${dir}\\\\raw.csv" index $minus1 title "Raw data" with impulses
    |;
}

# histograms - all in one
print PLOT qq|
    set title "All channels"
    set xlabel "ADC counts"
    set ylabel "Count"
    set output "${dir}\\\\all_in_one.png"
    set autoscale
    set yrange [0:$max_c]
    set key spacing 0.5
    plot \\|;
for $i (1..64) {
    my $plus1 = $i + 1;
    print PLOT qq|
        "${dir}\\\\hist.csv" using 1:$plus1 title "Channel $i" with histeps|;
    if($i < 64) { print PLOT ", \\"; }
}
print PLOT "\n";

# 2d pulse heights
print PLOT qq|
    set title "ADC pulse heights"
    unset grid
    set xlabel "X"
    set ylabel "Y"
    set autoscale
    set xrange [0.5:8.5]
    set yrange [0.5:8.5]
    set pm3d at b corners2color c1
    set view map
    set style data pm3d
    set output "${dir}\\\\mean_2d.png"
    splot "${dir}\\\\summary_2d.csv" using 1:2:3 title ""
    unset pm3d
    set grid
|;

# 2d pulse heights normalized
print PLOT qq|
    set title "Normalized ADC pulse heights"
    unset grid
    set xlabel "X"
    set ylabel "Y"
    set autoscale
    set xrange [0.5:8.5]
    set yrange [0.5:8.5]
    set pm3d at b corners2color c1
    set view map
    set style data pm3d
    set output "${dir}\\\\mean_normalized_2d.png"
    splot "${dir}\\\\summary_2d.csv" using 1:2:9 title ""
    unset pm3d
    set grid
|;

# 2d all in one
print PLOT qq|
    set title "Histograms"
    unset grid
    set xlabel "Channel #"
    set ylabel "X"
    set autoscale
    set xrange [0.5:64.5]
    set yrange [$min:$max]
    set pm3d at b corners2color c1
    set view map
    set style data pm3d
    set output "${dir}\\\\hist_2d.png"
    splot "${dir}\\\\hist_2d.csv" using 1:2:3 title ""
    unset pm3d
    set grid
|;

# histograms - 8x8 individual channels
print PLOT qq|
    set output "${dir}\\\\all_in_8x8.png"
    unset grid
    unset title
    unset xlabel
    unset ylabel
    unset key
    set autoscale
    set yrange [0:$max_c]
    set noxtics
    set noytics
    set format x ""
    set format y ""
    set multiplot layout 8,8
|;
for $i (0..63) {
    my $plus2 = $i + 2;
    print PLOT qq|
        plot "${dir}\\\\hist.csv" using 1:$plus2 with histeps, \\
             "${dir}\\\\raw.csv" index $i with impulses
    |;
}
print PLOT "unset multiplot\n";

close PLOT or die $!;
system("wgnuplot.exe plot.gp") == 0 or die "Error running gnuplot: $?";

}

#######################################################################################################

sub countrun{

# print "Full Path: $full_path\n";

my $file_count=0;
	
opendir(DIR, "$full_path\\USB_$usb");	
    while(my $file = readdir(DIR)) {
		if($file =~ /signal_(\d+).bin/) {
			$file_count++;
		}
    }

closedir(DIR);
    
# print "Files: $file_count\n";
    
open OUT, ">$full_path\\USB_$usb\\howmanyruns.log" or die $!;
    
print OUT "$file_count";
    
close OUT or die $!;

}
######################################################################################################
sub loadpmtdata_auto {

$usb = shift;
$pmt = shift;
my $pmtserialnumber =  shift;

push(@pmt_board_array, $pmt); ##camillo added this on 02/06/17

if($pmt eq -1) {

$pmtdata[$usb][$pmt][0] = "none";
$pmtdata[$usb][$pmt][1] = $pmt;
$pmtdata[$usb][$pmt][2] = 0;
$pmtdata[$usb][$pmt][3] = 1000;
$pmtdata[$usb][$pmt][4] = "no";
$pmtdata[$usb][$pmt][5] = "testhit";
$pmtdata[$usb][$pmt][6] = 5;
$pmtdata[$usb][$pmt][7] = 0b00000110;
$pmtdata[$usb][$pmt][8] = "test with default configuration";

goto goend;
}

if( defined($pmtserialnumber) ){

#code for retrieving info form mysql database

my $foundcount = 0;

my $choicefirst = "";
my $choicesecond;
my $DACt1; 
my $gate1;
my $maroc2;
my $board1;

eval {
	$dsn = "dbi:mysql:database=$database;host=$hostname"; 
	$dbh = DBI->connect($dsn, $user, $password); 

print "Failed to connect to database: $@\n" if $@;
if($@ eq "") {print "Connected to MySQL database\n\n"; }

	$drh = DBI->install_driver("mysql");

	$sth = $dbh->prepare("SELECT USB_serial, PMT_Serial, board_number, HV, DAC_threshold,
	 use_maroc2gain, gate, hdelay, trigger_mode, gain1, gain2, gain3, gain4, 
	gain5, gain6, gain7, gain8, gain9, gain10, 
	gain11, gain12, gain13, gain14, gain15, gain16 , 
	gain17, gain18, gain19, gain20, gain21, gain22 , 
	gain23, gain24, gain25, gain26, gain27, gain28 , 
	gain29, gain30, gain31, gain32, gain33, gain34 , 
	gain35, gain36, gain37, gain38, gain39, gain40 , 
	gain41, gain42, gain43, gain44, gain45, gain46, 
	gain47, gain48, gain49, gain50, gain51, gain52, 
	gain53, gain54, gain55, gain56, gain57, gain58, 
	gain59, gain60, gain61, gain62, gain63, gain64 FROM onlinenew");

	$sth->execute();
	while (my $ref = $sth->fetchrow_arrayref()) {		
			if($ref->[1] eq $pmtserialnumber){ #check for matching pmt_serial number and board_address
				$pmtdata[$usb][$pmt][0] = $ref->[1];
				$pmtdata[$usb][$pmt][1] = $ref->[2];
				$pmtdata[$usb][$pmt][2] = $ref->[3];
				$pmtdata[$usb][$pmt][3] = 500.+$ref->[4];
				$pmtdata[$usb][$pmt][4] = $ref->[5];
				$pmtdata[$usb][$pmt][5] = $ref->[6];
				$pmtdata[$usb][$pmt][6] = $ref->[7];
				$pmtdata[$usb][$pmt][7] = $ref->[8]; 
			
				for(my $index=0;$index<64;$index++){
					$gaindata[$usb][$pmt][$index] =  $ref->[9+$index];
				}
			#if($ref->[0] == $usb and $ref->[1] eq $pmtserialnumber and $ref->[2] == $pmt){				#check for matching pmt_serial number and board_address
		        print "Found info for: PMT_serial = $ref->[1], board_number = $pmt \n\n";
				print "Please check that your setup has the following setting:\n";
				print "USB address = $usb \n";
				print "Pmt serial number=$pmtdata[$usb][$pmt][0]\n";
				print "Board number=$pmtdata[$usb][$pmt][1]\n";
				print "HV setting=$pmtdata[$usb][$pmt][2]\n";
				print "DAC threshold = $pmtdata[$usb][$pmt][3]\n";
				print "Use of maroc2 gain constants = $pmtdata[$usb][$pmt][4]\n";
				print "Gate = $pmtdata[$usb][$pmt][5]\n";
				print "hdelay = $pmtdata[$usb][$pmt][6]\n";
				print "trigger_mode = $pmtdata[$usb][$pmt][7]\n\n";
				$DACt1 = $pmtdata[$usb][$pmt][3];
				$gate1 = $pmtdata[$usb][$pmt][5];
				$maroc2 = $pmtdata[$usb][$pmt][4];
				$foundcount = 1;
				$mysqlload = 1;
				#print "Press enter if you want to take data with that settings. If instead you would like to change any settings type change.\n\n";
				#chomp($choicefirst = <STDIN>);
			}
			if($ref->[1] eq $pmtserialnumber){
				$board1 = $pmtdata[$usb][$pmt][1];
			}				
	}
	$sth->finish(); 
	$dbh->disconnect();
};

print "Reading from MySQL failed: $@\n" if $@;
#if($@ eq "") {print "Setting read out from MySQL server @ Nevis\n\n";}

  
  if($foundcount == 0){
	print " \t\t !!!!!! Warning !!!!!!!\n";
	print "Info for your PMT <-> board not present in the mysql database or do not match.\n\n";
	print "For PMT = $pmtserialnumber the board address in the MySQL database is not $pmt\n";
	$mysqlload = 0;
	restartchoice:
	print "If you want to quit just type quit and press enter, if you want to continue and put the run info manually type manual\n";
	chomp($choice = <STDIN>);                                    
		if($choice ne "quit" && $choice ne "manual") {goto restartchoice};
		if($choice eq "quit") {exit()};
		if($choice eq "manual") {
			setpmtdata($usb,$pmt);
			if($pmtdata[$usb][$pmt][4] eq "yes") {
				print "I will use if founded the MARCOC2 gain constants stored in this machine\n"; 
			}
		goto goend	
		}
  }
  else{
  if($choicefirst eq "change"){
    $choice = "manual";
	setpmtdata($usb,$pmt);
	if($pmtdata[$usb][$pmt][4] eq "yes") {
		print "Please type which gain constants you would like to use (mysql or local)\n";
		chomp($choicesecond = <STDIN>);
		if($choicesecond eq "local"){ $mysqlload = 0;};	
	}
  }
}
  
print "\n\n"; 

}
else {
print "Loading setting as default for pmt board $pmt\n";

$pmtdata[$usb][$pmt][0] = "none";
$pmtdata[$usb][$pmt][1] = $pmt;
$pmtdata[$usb][$pmt][2] = 0;
$pmtdata[$usb][$pmt][3] = 1000;
$pmtdata[$usb][$pmt][4] = "no";
$pmtdata[$usb][$pmt][5] = "off";
$pmtdata[$usb][$pmt][6] = 5;
$pmtdata[$usb][$pmt][7] = 0b00000110;
$pmtdata[$usb][$pmt][8] = "test with sync configuration";

}

goend:
#$comments = "none";
if(!$pmtserialnumber and $pmt ne 1){
print "Please enter comments for this run (max 50 characters), press enter when done\n";
chomp($comments = <STDIN>);
$pmtdata[$usb][$pmt][8] = $comments;
}
$pmtdata[$usb][$pmt][8] = $comments;
print "\n";
print "\n";

}

###########################################################################################################

sub baselinemb {

#print "Initializing pmt boards ... be patient ...\n";
#print "\n";

my $fileb;

init_readout "127.0.0.1";           # or "localhost"

my $usb = shift;    # -1 = all
#my $pmt1 = shift;    # 63 = all
my $pmt1;

our $filebase = "baselines.bin";         # name of file to generate

my $fullpath = "$full_path\\USB_${usb}";
dir $fullpath;  

new $usb, "$dir\\$filebase";        # open new file

#insert this into a loop from 1..4
usb $usb, 1;							#auto token on
tarry 0.5;

foreach $pmt1 (@pmt_board_array) {
            
#print "Starting  USB = $usb,  PMT = $pmt,  Run = $run\n";
com $usb, $pmt1, 110, 1;                # turn led on/off
com $usb, $pmt1, 109, 1;                # vdd_fsb on
com $usb, $pmt1, 73, 0b00000;           # set up pmt module
#print "Configuring...\n";
com $usb, $pmt1, 74, 0b0100000;         # default gain
com $usb, $pmt1, 70, 0;                 # load default
dac $usb, $pmt1, 1000;                  # dac threshold
com $usb, $pmt1, 71, 0;                 # rst_g to maroc
com $usb, $pmt1, 72, 0;                 # write config
com $usb, $pmt1, 72, 0;                 # write config
com $usb, $pmt1, 73, 0b00110;           # set up pmt module
com $usb, $pmt1, 75, 0b00010000;        #set up trigger mode
com $usb, $pmt1, 254, 0;                # enable triggers

tarry 1;

#print "Taking baseline data for pmtboard $pmt1 ...\n";
for(1..10) {						#avoid first packets
    com $usb, $pmt1, 81, 0;         # test trigger
}

tarry 1;

#new $usb, "$dir\\$filebase";        # open new file

for(1..500) {
    com $usb, $pmt1, 81, 0;         # test trigger
}

print "Finishing up with pmt board $pmt1...\n";
com $usb, $pmt1, 255, 0;
com $usb, $pmt1, 73, 0b00000;       # set up pmt module
tarry 1;                          # wait for late packets


###finish loop
}

shut $usb;                          # close file

my $dir2 = $homedir . "DATA\\Run_${run}\\USB_${usb}"; 
my $full_path2 = $homedir . "DATA\\Run_${run}\\USB_${usb}";
                 
tarry 2;


print "Baseline data taken completed.\n\n";

foreach $pmt1 (@pmt_board_array) {
#add another loop for pmtboard from 1..4
our $dir1 = $homedir . "DATA\\Run_${run}\\USB_${usb}\\PMT_${pmt1}";  # change this to create your own directory structure
dir $dir1;     

my $dir2 = $homedir . "DATA\\Run_${run}\\USB_${usb}"; 
my $full_path2 = $homedir . "DATA\\Run_${run}\\USB_${usb}";
                 
system qq|cmd /C baselines.pl "$dir2" "$filebase" "$pmt1"|;
print "Baseline decoded for pmt $pmt1...\n\n";
initializeusb($usb,$pmt1);
}

}



#######################################################################################################
1;
