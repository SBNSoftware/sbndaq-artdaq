package DCFOV;

our (@ISA, @EXPORT, @EXPORT_OK, %EXPORT_TAGS, $VERSION);
use Exporter;
$VERSION = 1.00;
@ISA = qw(Exporter);

@EXPORT = qw(getrunconfig check_diskspace load_ip_config load_port_config load_db_config
gaibu_msg gaibu_msg_serv check_eventbuilderhost_diskspace initializeboard initializetriggerbox initializefaketriggerbox duplicate_run_number
starttakedata stoptakedata checknumusbs check_rate checkforlateeventbuilder checkDOGSifier check_file_size
loadrunsummarymysql checkHVstatus checkLVstatus test_system_time check_system_time where_to_run is_readout_alive
updaterunsummarymysql rc_runend_comment loadconfig OVCalibSummary updateOVCalibSummary checkOVCalibend
checkROPLog sendmail check_last_disk_used loadpmtdata_auto baselinemb initializeusb initializefilestructure takedatamb generatecsv2 plotdatamb2 setpmtdata getpmtdata
$pmt $usb $pmtserialnumber $HVsetting $lasersinglepe
$DACt $thresh $hdelay $runlength $pipedelay $force_trig $filenametemp $newrunlength $meanpulseheight $gate $gateonoff
$date $run $full_path $full_path2 $file $baselinedir $baselines @averagehitchannel
$gaindirectory $savedgaindirectory $filename $dacdirectory 
$newdacdirectory $dacfile $average $repeat $ADCpe @pmtdata @gaindata $inispread
$meanph $meanpe $meangain $meanentries $laserrate @dacvalue $takebaseline $mysqlload
$pmtmb $usemaroc2gainconstantsmb $boardnumber $gateuse $trigger_mode $newrunlength
$logfile $newlogfile $laserintensity $homedir $usemaroc2gainconstants $filelength
$usemultiplefile $filebase $hostnamepc @noiseratePE $cornercorrection $noisesummaryrun $username
$spread_pulse_height $maxminratio $gainratio $phratio $DebugDAC $elapsed_time
$DBhostname $dbh $dsn $user $password $tablename $database $client_sock $client_sock_OVRC
$gaibu_ip $gaibu_port $OVDAQ $OVRC_IP $OVRC_Port $online_path $online_path_local $module $OVCC_IP $OVCC_Port
$OVEVENTBUILDER_HOST $OVEVENTBUILDER_IP $OVONLINEMONITOR_HOST $OVONLINEMONITOR_IP
$OVSERV_HOST $OVSERV_IP
$runcontrol_hostname $usedogsifier $HWthreshold $dcmacropath $eb_flag
$eb_disk $disk $eb_disk_usage $disk_usage $eventbuilderhost_usage $trigger $detector_configuration
$DAQ_hostname $DataFolder $DataPath @usb $time_run_number $time
$onlinetable $run_comment $run_number $run_type $run_length $run_OVtype $run_shifter
$MNOTICE $MWARNING $MERROR $MEXCEPTION $starttime $stoptime $HV $threshold $Time_Data_Path
$totalpmt @pmttousb @pmttoboard $totalbox @boxtousb @boxtoboard $spreadCalib $totusbs $usb_box
$nfs_mount $user_online $remote_path $number_expected_usb %usbhowmanybox %usbhowmanyboards $totusbs $mindiskspace
@pmt_board_array);

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
use Sys::Syslog qw( :DEFAULT setlogsock);	
#use Filesys::DiskSpace;
use English;
use File::Compare;

require DCfunctions;

our ($client_sock,$client_sock_OVRC,$DebugDAC,$pmt,$usb,$pmtserialnumber,$HVsetting,$lasersinglepe,$mysqlload,
$DACt,$thresh,$hdelay,$runlength,$newrunlength,$pipedelay,$force_trig,$meanpulseheight,$gate,$gateonoff,$elapsed_time,
$date,$run,$full_path,$full_path2,$file,$baselinedir,$baselines,$ADCpe,$comments,
$gaindirectory,$savedgaindirectory,$filename,$dacdirectory,$newdacdirectory,$detector_configuration,
$dacfile,$average,$repeat,$logfile,$newlogfile,$laserintensity,@pmtdata,@gaindata,
$meanph,$meanpe,$meangain,$meanentries,$laserrate,@dacvalue,$homedir,$boardnumber,
$takebaseline,$usemaroc2gainconstants,$pmtmb,$usemaroc2gainconstantsmb,
$gateuse,$trigger_mode,$newrun_length,$filelength,$dir,$usemultiplefile,$filebase,@gain,
$hostnamepc,$hostnamepclc,$equalization,$passwd,$choice,@noiseratePE,$cornercorrection,$noisesummaryrun,$run_number,
$newpmtdatafound,$username,$inispread,@averagehitchannel,$spread_pulse_height,$OVDAQ,
$OVEVENTBUILDER_HOST, $OVEVENTBUILDER_IP, $OVONLINEMONITOR_HOST, $OVONLINEMONITOR_IP,
$OVSERV_HOST, $OVSERV_IP, $time,
$runcontrol_hostname, $run_OVtype, $usedogsifier, $time_run_number, $remote_path,
$gainratio, $maxminratio, $phratio, $starttime, $stoptime, $HV, $module, $number_expected_usb,
$threshold, $HWthreshold, $trigger, $spreadCalib, $Time_Data_Path, $user_online,$DAQ_hostname,$eb_flag,
$DataFolder,$DataPath,$disk,$eb_disk,$eventbuilderhost_usage,$eb_disk_usage,$disk_usage,$nfs_mount,
@pmt_board_array);


our ($gain1, $gain2, $gain3, $gain4, $gain5, $gain6, $gain7, $gain8, $gain9, 
			$gain10, $gain11, $gain12, $gain13, $gain14, $gain15, $gain16, $gain17, $gain18, $gain19, 
			$gain20, $gain21, $gain22, $gain23, $gain24, $gain25, $gain26, $gain27, $gain28, $gain29, 
			$gain30, $gain31, $gain32, $gain33, $gain34, $gain35, $gain36, $gain37, $gain38, $gain39, 
			$gain40, $gain41, $gain42, $gain43, $gain44, $gain45, $gain46, $gain47, $gain48, $gain49, 
			$gain50, $gain51, $gain52, $gain53, $gain54, $gain55, $gain56, $gain57, $gain58, $gain59, 
			$gain60, $gain61, $gain62, $gain63, $gain64);

our($p0,$p1);

our($usb_box,%usbhowmanybox,%usbhowmanyboards, $totusbs);

our($mode,@usblocal,@usb);

our $OVRC_IP; # = '10.44.1.104'; # snap
our $OVRC_Port; # = '50002';     # OV DAQ RC Port
our $gaibu_ip; # = '10.44.7.110'; # Gaibu server address 10.44.1.106;
our $gaibu_port; # = '50500'; # Gaibu server port 60100
our $OVCC_IP;
our $OVCC_Port;

our ($DBhostname,$database,$user,$password,$HVdatabase,$HVuser,$HVpassword);
our ($dsn,$dbh,$drh,$sth);

our $onlinetable = "online"; # this is the default table for the timing test# "online800";

our $MINSTRUCTION = 1;
our $MNOTICE = 2;
our $MWARNING = 3;
our $MERROR = 4;
our $MEXCEPTION = 5;
our $mindiskspace;
our ($run_type,$run_comment,$run_length,$run_shifter);
our ($pmt_local, $usb_local, $totalpmt, @pmttousb, @pmttoboard, @structure);
our ($totalbox, @boxtousb, @boxtoboard);
our ($oldsec,$oldmin,$oldhour,$oldday,$oldmonth,$oldyr19,@oldrest);

$homedir = getcwd;
$homedir =~ s|Scripts_mb/macro||;
$homedir =~ s|\/|\\|g;

$hostnamepc = hostname;
$hostnamepclc = hostname;
$hostnamepclc =~ tr/A-Z/a-z/;

$DBhostname = "localhost";
$user = "dcndaq" ;
$password = "137316neutron";
$database = "doublechooz_ov_far";

$totalpmt = 0;
$filelength = 5;
#$laserrate = 100;
$meanpulseheight  = 350;
$mindiskspace = 4.;
$mysqlload = 0;
$equalization = "false";
$choice = "";
$cornercorrection = "off";
$starttime  = "";
$stoptime   = "";
$HV         = "";
$threshold  = "";
$comments   = "";
$run_number = -999;

setlogsock('unix');
openlog('','','user');


$gaibu_port = "50500"; 

$gaibu_ip = "129.236.252.104"; #just an initialization value

our $online_path = $ENV{'DCONLINE_PATH'};   # this path can change is NFS is defined
$remote_path = $ENV{'DCONLINE_PATH'};   # this path can change is NFS is defined

our $online_mount_path;

our $dcmacropath = "e/h.0/localdev/readout/macro";

$OUTPUT_AUTOFLUSH = 1; #writing messages immediately to STDOUT

##################################################################################

sub rc_runend_comment {
#subroutine to generate the comment to be inserted at the end of the RC
#comment file

my $lab = shift;

if(!$lab) {
   $lab = "far"; #assuming it has default for now
}

my $run_filename_forendcomment;
my ($run_sec,$run_min,$run_hour,$run_day,$run_mon,$run_yr,@run_rest) =   
localtime(time);
$run_hour= sprintf("%0.2d",$run_hour);
$run_min = sprintf("%0.2d",$run_min);
$run_sec = sprintf("%0.2d",$run_sec);
$run_yr  = sprintf("%0.4d",${run_yr}+1900);
$run_mon = sprintf("%0.2d",${run_mon}+1);
$run_day = sprintf("%0.2d",${run_day});
#RUN_TIME_END_NUDAQ     :YYYY-MM-DD HH:MM:SS
my $run_db_string = sprintf("%0.4d-%0.2d-%0.2d %0.2d:%0.2d:%0.2d",${run_yr},${run_mon}+1,${run_day},${run_hour},${run_min},${run_sec});

if($lab eq "far") {
	$run_filename_forendcomment = sprintf("DCRCBackup4DBI_F%0.7s.tmp",${run_number}); 
}
if($lab eq "near") {
	$run_filename_forendcomment = sprintf("DCRCBackup4DBI_N%0.7s.tmp",${run_number}); 
}
# changed MHT 4/5/2010
#system qq|ssh dconline\@${runcontrol_hostname} "echo '     RUN_TIME_END_OVDAQ     :${run_db_string}' >> ${remote_path}\/dbbackup\/runinfo\/$run_filename_forendcomment"|;
system qq|ssh dconline\@${OVEVENTBUILDER_HOST} "echo '     RUN_TIME_END_OVDAQ     :${run_db_string}' >> ${remote_path}\/dbbackup\/runinfo\/$run_filename_forendcomment"|;

}

###################################################################################
sub check_eventbuilderhost_diskspace {

    #my $EB_HOST = shift;
    my ($hostpath,@extra) = split(/\./,shift);

    my $DataFolder_1 = "/$hostpath/data1";
    my $DataFolder_2 = "/$hostpath/data2";
    $eb_flag = 1;
    if(!(-e $DataFolder_1)) {
	gaibu_msg($MWARNING,"Folder $DataFolder_1 not found...EBuilder will not run properly...");
	$eb_flag = 0; return;
    }
    if(!(-e $DataFolder_2)) {
	gaibu_msg($MWARNING,"Folder $DataFolder_2 not found...EBuilder will not run properly...");
	$eb_flag = 0; return;
    }

# get data for data disk 1 on OVEVENTBUILDERHOST
my ($fs_type, $fs_desc, $used, $avail, $fused, $favail) = df $DataFolder_1; 
# calculate free space in %
my $df_free = (($avail) / ($avail+$used)) * 100.0;

# get data for data disk 2 on OVEVENTBUILDERHOST
my ($fs_type2, $fs_desc2, $used2, $avail2, $fused2, $favail2) = df $DataFolder_2; 
# calculate free space in %
my $df_free2 = (($avail2) / ($avail2+$used2)) * 100.0;

# if the space on disk is less than 3% (2.5% needed to store 8 hours of data)
if($eb_disk == 1) {
    if($df_free >=3) { 
	#nothing. = good
    }
    else {
	if($df_free2 >= 3) {
	    $eb_disk = 2;
	    gaibu_msg($MNOTICE,"OV EBuilder host data folder space free < 3% on ${DataFolder_1}"); 
	    gaibu_msg($MNOTICE,"OV EBuilder -- now using ${DataFolder_2}"); 
	}
	else {
	    gaibu_msg($MERROR,"OV EBuilder data folder space free < 3% on both disks"); 
	    sendmail("OVRC","CRITICAL","OV EBuilder PC data folder space free < 3% on both disks"); 
	}
    }
}
else {
    if($df_free2 >=3) {
	#nothing. = good
    }
    else {
	if($df_free >= 3) {
	    $eb_disk = 1;
	    gaibu_msg($MNOTICE,"OV EBuilder host data folder space free < 3% on ${DataFolder_2}"); 
	    gaibu_msg($MNOTICE,"OV EBuilder -- now using ${DataFolder_1}"); 
	}
	else {
	    gaibu_msg($MERROR,"OV EBuilder host data folder space free < 3% on both disks");
	    sendmail("OVRC","CRITICAL","OV EBuilder PC data folder space free < 3% on both disks"); 		
	}
    }
}   

$eb_disk_usage = 0;

if($eb_disk==1) {
 $eb_disk_usage = $df_free;
 $eventbuilderhost_usage = 100 - $eb_disk_usage;
}   
elsif ($eb_disk==2) {
 $eb_disk_usage = $df_free2;
 $eventbuilderhost_usage = 100 - $eb_disk_usage;
}
#print "eb_disk: $eb_disk \t eventbuilderhostusage: $eventbuilderhost_usage\n";
#($DAQ_hostname,@extra) = split(/\./,gethostbyaddr(inet_aton($OVRC_IP), AF_INET));
#print "DAQ_hostname: $DAQ_hostname\n";

# Function below is no longer necessary as all OV machines mount all the others.
#    open IN, "ssh dconline\@${OVEVENTBUILDER_HOST} \"df -lh /data1\" |" or die $!;
#    while(<IN>) {
#	if(/\s+(\d+)\%/) { $eventbuilderhost_usage = $1; last; }
#    }

}

###################################################################################
sub check_diskspace{

    $disk = check_last_disk_used();

    my $DataFolder_1 = "/data1/$DataPath";
    my $DataFolder_2 = "/data2/$DataPath";

    if(!(-e $DataFolder_1)) {

	mkpath "$DataFolder_1", 0775;
	gaibu_msg($MNOTICE,"Folder $DataFolder_1 not found. Creating...");
	
    }
    if(!(-e $DataFolder_2)) {

	mkpath "$DataFolder_2", 0775;
	gaibu_msg($MNOTICE,"Folder $DataFolder_2 not found. Creating...");
	
    }

# get data for data disk 1 on snap
my ($fs_type, $fs_desc, $used, $avail, $fused, $favail) = df $DataFolder_1; 
# calculate free space in %
my $df_free = (($avail) / ($avail+$used)) * 100.0;

# get data for data disk 1 on snap
my ($fs_type2, $fs_desc2, $used2, $avail2, $fused2, $favail2) = df $DataFolder_2; 
# calculate free space in %
my $df_free2 = (($avail2) / ($avail2+$used2)) * 100.0;

# if the space on disk is less than 3% (2.5% needed to store 8 hours of data)
if($disk == 1) {
    if($df_free >= $mindiskspace) {
	#nothing. = good
    }
    else {
	if($df_free2 >= $mindiskspace) {
	    $disk = 2;
	    gaibu_msg($MNOTICE,"OV DAQ PC data folder space free < 3% on ${DataFolder_1}"); 
	    gaibu_msg($MNOTICE,"OV DAQ -- now using ${DataFolder_2}"); 
	}
	else {
	    gaibu_msg($MERROR,"OV DAQ PC data folder space free < 3% on both disks"); 
	    sendmail("OVRC","CRITICAL","OV DAQ PC data folder space free < 3% on both disks"); 
	}
    }
}
else {
    if($df_free2 >=$mindiskspace) {
	#nothing. = good
    }
    else {
	if($df_free >= $mindiskspace) {
	    $disk = 1;
	    gaibu_msg($MNOTICE,"OV DAQ PC data folder space free < 3% on ${DataFolder_2}"); 
	    gaibu_msg($MNOTICE,"OV DAQ -- now using ${DataFolder_1}"); 
	}
	else {
	    gaibu_msg($MERROR,"OV DAQ PC data folder space free < 3% on both disks");
	    sendmail("OVRC","CRITICAL","OV DAQ PC data folder space free < 3% on both disks"); 		
	}
    }
}   

$disk_usage = 0;

if($disk==1) {
 $disk_usage = 100 - $df_free;
}   
elsif ($disk==2) {
 $disk_usage = 100 - $df_free2;
}

$DataFolder = "/data${disk}/${DataPath}";

}

###########################################################################
#getting the run configuration reading from file


sub getrunconfig{

    my $config_name = shift;
    $config_name =~ s/\s+//g;

    if($remote_path eq "") {
	print "Error: could not read online path";
    }
    my $found = 0;

    #my $mount_infile = $online_mount_path . "/config/DCRunProfile.config";

    my $infile = $remote_path . "/config/DCRunProfile.config";

    open IN, $infile or die $!;
    while(<IN>) {
	#search for the config_name !cd $config_name
	next if($_ =~ /^\s*#(.*)/);

	if( $_ =~ /^!cd $config_name\s*/ ) {
#	    print "Found run type !\n";
	    $found = 1;
	}

	if($found == 1){
	    if($_ =~ /^\s*RUN_DESCRIPTION\s*:(.*)/) {
		$run_comment = $1;
		print "\n Run description: $1\n";
	    }
            elsif($_ =~ /^\s*RUN_LENGTH\s*:(\d+)/) {
		my $run_length = $1;
		print "Run Length = $run_length sec \n";
	    }
	    elsif($_ =~ /^\s*DCOVDAQ_CONFIG_TABLE\s*:(\S+)/) {
		print "Run Parameter: $1\n\n";
		#here I will decide what mysql table for configuration I would use
		#$run_type = $1;
		$onlinetable = $1;
	    }
	    elsif($_ =~ /^\s*DCOVDAQ_OFFLINE_THR\s*:(\S+)/) {
		print "OFFLINE Threshold: $1\n\n";
		#here I will decide what mysql table for configuration I would use
		$threshold = $1;
	    }
	    elsif($_ =~ /^\s*DCOVDAQ_OFFLINE_TRIGGER_TYPE\s*:(\S+)/) {
		print "OFFLINE Trigger Type: $1\n\n";
		#here I will decide what mysql table for configuration I would use
		$trigger = $1;
		$found = 0;
	    }
	}
    }
    close IN or die $!;
}


###########################################################################
sub loadconfig {

my $mode_local = shift;
if(!$mode_local) { $mode = "mysql"; }
else { $mode = $mode_local; }
@usblocal = ();
#code for retrieving info form mysql database

if($mode eq "mysql") {

$totalpmt = 0;
$totalbox = 0;
$usb_box = 0;

eval {
	$dsn = "dbi:mysql:database=$database;host=$DBhostname"; 
	$dbh = DBI->connect($dsn, $user, $password); 
        print "Failed to connect to database: $@\n" if $@;
	$drh = DBI->install_driver("mysql");
	$sth = $dbh->prepare("SELECT USB_serial, PMT_Serial, board_number, HV, DAC_threshold,
	 use_maroc2gain, gate, pipedelay, force_trig, trigger_mode, pmtboard_u, gain1, gain2, gain3, gain4, 
	gain5, gain6, gain7, gain8, gain9, gain10, 
	gain11, gain12, gain13, gain14, gain15, gain16 , 
	gain17, gain18, gain19, gain20, gain21, gain22 , 
	gain23, gain24, gain25, gain26, gain27, gain28 , 
	gain29, gain30, gain31, gain32, gain33, gain34 , 
	gain35, gain36, gain37, gain38, gain39, gain40 , 
	gain41, gain42, gain43, gain44, gain45, gain46, 
	gain47, gain48, gain49, gain50, gain51, gain52, 
	gain53, gain54, gain55, gain56, gain57, gain58, 
	gain59, gain60, gain61, gain62, gain63, gain64 FROM $onlinetable");
	$sth->execute();
	while (my $ref = $sth->fetchrow_arrayref()) {
			$pmtdata[$ref->[0]][$ref->[2]][0] = $ref->[1]; #pmt serial number
			$pmtdata[$ref->[0]][$ref->[2]][1] = $ref->[2]; #pmt board number (not unique)
			$pmtdata[$ref->[0]][$ref->[2]][2] = $ref->[3]; #HV setting
			$pmtdata[$ref->[0]][$ref->[2]][3] = $ref->[4]; #DAC threshold
			$pmtdata[$ref->[0]][$ref->[2]][4] = $ref->[5]; #Use Maroc2 gain constants
			$pmtdata[$ref->[0]][$ref->[2]][5] = $ref->[6]; #op73
			$pmtdata[$ref->[0]][$ref->[2]][6] = $ref->[7]; #op85
			$pmtdata[$ref->[0]][$ref->[2]][7] = $ref->[8]; #op87
			$pmtdata[$ref->[0]][$ref->[2]][8] = $ref->[9]; #op75
			$pmtdata[$ref->[0]][$ref->[2]][9] = "mysql mode";  #comments
			$pmtdata[$ref->[0]][$ref->[2]][10] = $ref->[10]; #module number    
			if($pmtdata[$ref->[0]][$ref->[2]][2] eq "-999") {
			    for(my $index=0;$index<64;$index++){
				$gaindata[$ref->[0]][$ref->[2]][$index] =  16;
			    }
			    $totalbox++;
			    $boxtousb[$totalbox] = $ref->[0];     #usb number			
			    $boxtoboard[$totalbox] = $ref->[2];   #board number
			    $structure[$ref->[0]] = 0;
			    $usbhowmanybox{$ref->[0]}++;
			    $usb_box = $ref->[0];                 #unique usb to have boxes connected
			}
			else {
			    for(my $index=0;$index<64;$index++){
				$gaindata[$ref->[0]][$ref->[2]][$index] =  $ref->[11+$index];
			    }
			    $totalpmt++;
			    $pmttousb[$totalpmt] = $ref->[0];     #usb number
			    $pmttoboard[$totalpmt] = $ref->[2];   #board number
			    $structure[$ref->[0]] = 0;
			    $usbhowmanyboards{$ref->[0]}++;
			}
	  }
	  $sth->finish(); 
 
	  $dbh->disconnect();

   }; #close eval

$totusbs = keys(%usbhowmanyboards);

   print "Reading from MySQL failed: $@\n" if $@;
   if(!$@) { 
#	print "\nLoading settings from MySQL table: $onlinetable\n\n";
        $mysqlload = 1.; 
   }

}

elsif($mode eq 'debug') { # debug mode then I need two argument
    my $usb_board = shift;
    my $pmt_board = shift;
    my $triggerbox = shift;

    if(!$usb_board || !$pmt_board) { 
	print "error cannot initialize need USB board and PMT board ! \n";
    }
    else {
	if(!$triggerbox) {
#	    print " Board $pmt_board on USB $usb_board parameters loaded \n";
	    $pmtdata[$usb_board][$pmt_board][0]='none'; # pmt serial number
	    $pmtdata[$usb_board][$pmt_board][1]= $pmt; #board number
	    $pmtdata[$usb_board][$pmt_board][2]=0; # HV
	    if($pmt_board eq 1) {     $pmtdata[$usb_board][$pmt_board][3]=900; } #DAC
	    elsif($pmt_board eq 11) { $pmtdata[$usb_board][$pmt_board][3]=900; } #DAC
	    elsif($pmt_board eq 21) { $pmtdata[$usb_board][$pmt_board][3]=900; } #DAC
	    elsif($pmt_board eq 31) { $pmtdata[$usb_board][$pmt_board][3]=900; } #DAC
	    elsif($pmt_board eq 41) { $pmtdata[$usb_board][$pmt_board][3]=900; } #DAC
	    else{ $pmtdata[$usb_board][$pmt_board][3]=$DebugDAC; } #DAC
	    $pmtdata[$usb_board][$pmt_board][4]="no"; # use m2 gains
	    $pmtdata[$usb_board][$pmt_board][5]="off"; # gate (on, off,..)
            if($pmt_board <=40) {
	        $pmtdata[$usb_board][$pmt_board][6]=20; # pipedelay
            }
            else {
	        $pmtdata[$usb_board][$pmt_board][6]=21; # pipedelay
            }
	    $pmtdata[$usb_board][$pmt_board][7]=0; # op87
	    $pmtdata[$usb_board][$pmt_board][8]=0; # op75
	    $pmtdata[$usb_board][$pmt_board][9]="debug mode"; #comments    
	    $pmtdata[$usb_board][$pmt_board][10]=0; # module number
	    $totalpmt++;  
            $pmttousb[$totalpmt] = $usb_board;
            $pmttoboard[$totalpmt] = $pmt_board;
            ### should see what to do in initializeboard ###
	    $structure[$usb_board] = 0;
	}
	else {
	    $pmtdata[$usb_board][$pmt_board][0]='none'; # pmt serial number
	    $pmtdata[$usb_board][$pmt_board][1]= $pmt; #board number
	    $pmtdata[$usb_board][$pmt_board][2]=-999; # HV
	    $pmtdata[$usb_board][$pmt_board][4]="no"; # use m2 gains
	    $pmtdata[$usb_board][$pmt_board][5]="off"; # gate (on, off,..)
            if($pmt_board <=40) {
	        $pmtdata[$usb_board][$pmt_board][6]=20; # pipedelay
            }
            else {
	        $pmtdata[$usb_board][$pmt_board][6]=21; # pipedelay
            }
	    $pmtdata[$usb_board][$pmt_board][7]=0; # op87
	    $pmtdata[$usb_board][$pmt_board][8]=0; # op75
	    $pmtdata[$usb_board][$pmt_board][9]="debug mode"; #comments
	    $pmtdata[$usb_board][$pmt_board][10]=0; # module number    
	    $totalbox++;  
            $boxtousb[$totalbox] = $usb_board;
            $boxtoboard[$totalbox] = $pmt_board;
            ### should see what to do in initializeboard ###
	    $structure[$usb_board] = 0;
	}
    }
	    
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
$pipedelay = $pmtdata[$usbboard][$pmtnumber][6];
$force_trig = $pmtdata[$usbboard][$pmtnumber][7];
$trigger_mode = $pmtdata[$usbboard][$pmtnumber][8];
$comments = $pmtdata[$usbboard][$pmtnumber][9];
$module = $pmtdata[$usbboard][$pmtnumber][10];

=pod
if ($gate eq "on"){
    $gateonoff = 0b11010;
}
elsif( $gate eq "off" ){
    $gateonoff = 0b01010;
}
elsif( $gate eq "trigger" ){
    $gateonoff = 0b01011;  #highest bit -> en_gate,en_input,en_all,en_adc,en_hit  
}
elsif( $gate eq "triggera" ){
    $gateonoff = 0b01111;  
}
elsif( $gate eq "alladc" ){
    $gateonoff = 0b11110;  
}
elsif( $gate eq "alladcoff" ){
    $gateonoff = 0b01110;  
}
elsif( $gate eq "test" ){
    $gateonoff = 0b01010;
    $trigger_mode = 0b01100000;  
}
elsif( $gate eq "allhit") {
    $gateonoff = 0b01001;  
}
=cut
#print "pmt serial number=$pmtserialnumber\n";
#print "board number=$boardnumber--";
#print "HV setting=$HVsetting\n";
#print "DAC threshold in use = $DACt\n";
#print "use of maroc2 gain constants = $usemaroc2gainconstantsmb\n";
#print "gate = $gate\n";
#print "gateonoff = $gateonoff\n";
#print "pipedelay = $pipedelay\n";
#print "force_trig = $force_trig\n";
#print "comments = $comments\n";
#print "module = $module\n";

for(my $index=0;$index<64;$index++){
    if($mysqlload) { $gain[$index+1] = $gaindata[$usbboard][$pmtnumber][$index]; }
    else { $gain[$index+1] = 16; } 
    #print "$gain[$index+1]\n";
}

}

###########################################################################

sub initializeboard {

my $define_runnumber = shift;
my $trigger_num = shift;

my $pmtini;
my $pmtfin;
#my $pmtini = shift; #as unique number
#my $pmtfin = shift; #as unique number

if(!defined($pmtini = shift) || !defined($pmtfin = shift)) {
#both not defined
   $pmtini = 1;
   $pmtfin = $totalpmt;
}

if($mode eq 'debug'){
   my $totalboard = $pmtfin - $pmtini + 1;
   if($totalpmt != $totalboard) {
       print "totalpmt: $totalpmt\ttotalboard: $totalboard\n";
     print "Problem with initialization \n";
   }
}

my $pmt1;
my @usbbase;
my $usbread;

foreach $usbread (keys(%usbhowmanyboards)){
     set_inhibit_usb $usbread, -1;     # stop previous data taking (time check)
}

foreach $usbread (keys(%usbhowmanybox)){
     set_inhibit_usb $usbread, -1;     # stop previous data taking (time check)
}

tarry 1;

### now the new folder and the new data path will be passed to the DAQ

set_data_path($DataPath);
check_diskspace;
set_data_disk $disk;  #setting $DataFolder


#defining the run number
my $file_run;
my $max = 0;
my @files_run;
my ($sec,$min,$hour,$day,$month,$yr19,@rest) = localtime(time);
($oldsec,$oldmin,$oldhour,$oldday,$oldmonth,$oldyr19,@oldrest) = localtime(time);

$yr19 = $yr19 + 1900; # + 1900; # hack to get something for no
$month = sprintf("%0.2d",${month}+1);
$day = sprintf("%0.2d",$day);
$hour = sprintf("%0.2d",$hour);
$min = sprintf("%0.2d",$min);
$run = "${yr19}${month}${day}_${hour}_${min}";
$date = "${yr19}${month}${day}";

@files_run = <${DataFolder}/Run_*>;

foreach $file_run (@files_run) {
  if($file_run =~ /Run_(\d+)$/) {
    $max = $1 if $1 > $max;
  }
}  

$max = $max + 1;

if ($define_runnumber eq "auto") {
    $run_number = sprintf("%0.7d",$max);
}

set_run_number "$run_number";

$run = $run_number;

print "Baseline data taking .";


    for ($pmt1 = $pmtini; $pmt1<=$pmtfin; $pmt1++) {

	if(!defined($usb_local = $pmttousb[$pmt1]) || !defined($pmt_local = $pmttoboard[$pmt1])) {
	    print "usb_local or pmt_local not defined.\n";
	}
	getpmtdata (${usb_local},${pmt_local});
	usb 0, 1;                                          # auto token on
	com_usb $usb_local, $pmt_local, 110, 1;	           # turn off led off/on the PMT's board = 1/0
	com_usb $usb_local, $pmt_local, 109, 1;             # vdd_fsb on
	com_usb $usb_local, $pmt_local, 73,  0b00000;     # set up pmt module
	com_usb $usb_local, $pmt_local, 255, 0;             # clear buffers
	com_usb $usb_local, $pmt_local, 74,  0b0100000;     # default gain
	com_usb $usb_local, $pmt_local, 70,  0;             # load default
	dac_usb $usb_local, $pmt_local, 1000;               # threshold value
	com_usb $usb_local, $pmt_local, 71,  0;             # rst_g to maroc
	com_usb $usb_local, $pmt_local, 72,  0;             # write config was done twice
	com_usb $usb_local, $pmt_local, 73,  0b00110;     # set up pmt module
	com_usb $usb_local, $pmt_local, 87,  0;             # no force trigger
	com_usb $usb_local, $pmt_local, 75,  0b00010000;    # set up trigger mode for module            
	com_usb $usb_local, $pmt_local, 109, 0;	            # vdd_fsb off
	com_usb $usb_local, $pmt_local, 254, 0;             # enable trigger
	for (my $i=0; $i<10; $i++){
	    com_usb $usb_local, $pmt_local, 81, 0;          # avoid first packets
	}
	com_usb $usb_local, $pmt_local, 255, 0;             # disable trigger
        push(@usbbase, $usb_local);                         # prepare for set_inhibit
    }
    tarry 2.0;                                              

    foreach $usb_local (@usbbase) {
        if($structure[$usb_local]==0) { 
	   set_inhibit_usb $usb_local, -3;    #  -3; created file structure
	   tarry 0.5;
	   set_inhibit_usb $usb_local, -2;    #  -2; release inhibit for baseline
                                              #  -1; inhibit writing data
                                              #   0;  release inhibit for writing data
           $structure[$usb_local] = -2;
        }
    }

    for ($pmt1 = $pmtini; $pmt1<=$pmtfin; $pmt1++){
        $usb_local = $pmttousb[$pmt1];
        $pmt_local = $pmttoboard[$pmt1];
        usb_usb $usb_local, 0, 1;                           # turn auto token ON
	com_usb $usb_local, $pmt_local, 254, 0;             # enable trigger
	for(my $i=1; $i<=$trigger_num; $i++) {
	    com_usb $usb_local, $pmt_local, 81, 0;          # test trigger
	}
	com_usb $usb_local, $pmt_local, 255, 0;             # disable trigger
	tarry 0.01;                                         # give it some time
    }
    tarry 2.0;                                              # wait late packets

    foreach $usb_local (@usbbase) {
         if($structure[$usb_local] == -2) {
           set_inhibit_usb $usb_local, -1;    #  -2; release inhibit for baseline
                                              #  -1; inhibit writing data
                                              #   0;  release inhibit for writing data
           $structure[$usb_local] = -1;
           push(@usblocal, $usb_local);
         }
    }

    my ($newsec,$newmin,$newhour,$newday,$newmonth,$newyr19,@newrest) =   localtime(time);
    $elapsed_time = ($newday-$oldday)*24*3600+ ($newhour-$oldhour)*3600 + ($newmin-$oldmin)*60 + ($newsec - $oldsec);

    for(my $e=0; $e<=10*2; $e++){
           tarry 0.1;
           print ".";
    }

    print ": $elapsed_time sec\n";

    tarry 1;


###now let's initialize everything for the data taking

    print "Initializing .";

    my $dir2 = "${DataFolder}/Run_${run}"; 
    my $summary = "on";                                    #summary file default = off

    for (my $pmt1 = $pmtini; $pmt1<=$pmtfin; $pmt1++) {	
        $usb_local = $pmttousb[$pmt1];
        $pmt_local = $pmttoboard[$pmt1];
	getpmtdata (${usb_local},${pmt_local});
        usb_usb $usb_local, 0, 1;                           # auto token on
	com_usb $usb_local, $pmt_local, 110, 1;	            # turn off the three led on the PMT's board = 1
	com_usb $usb_local, $pmt_local, 109, 1;             # vdd_fsb on
	com_usb $usb_local, $pmt_local, 73, 0b00000;        # set up pmt module
	com_usb $usb_local, $pmt_local, 255, 0;             # clear buffers
	com_usb $usb_local, $pmt_local, 84, 255;            # buffer size limit  
	com_usb $usb_local, $pmt_local, 74, 0b0100000;      # default gain
	com_usb $usb_local, $pmt_local, 70, 0;              # load default
	com_usb $usb_local, $pmt_local, 67, 0b000010;       # statea
	com_usb $usb_local, $pmt_local, 68, 0b000000;       # stateb 
	com_usb $usb_local, $pmt_local, 69, 0b000000;       # statec
	com_usb $usb_local, $pmt_local, 71, 0;              # rst_g to maroc
	com_usb $usb_local, $pmt_local, 72, 0;              # write config
	com_usb $usb_local, $pmt_local, 80, 5;              # hold delay is variable. Has been fixed to 5 here	com_usb $usb_local, $pmt_local, 85, 0b10110; #$pipedelay;       # pipe delay for X&Y trigger box usage, set from MySQL
	dac_usb $usb_local, $pmt_local, $DACt;              # threshold value
	com_usb $usb_local, $pmt_local, 73, $gateonoff;     # gate
	com_usb $usb_local, $pmt_local, 85, $pipedelay;     # set up pipe delay
	com_usb $usb_local, $pmt_local, 75, $trigger_mode;  # trigger mode
	com_usb $usb_local, $pmt_local, 86, 1;              # edge strip mode
	com_usb $usb_local, $pmt_local, 87, $force_trig;    # force readout -> 01: 1msec, 10: 16msec, 11: 256msec

	if($usemaroc2gainconstantsmb eq "no") {
        	com_usb $usb_local, $pmt_local, 74, 0b0100000;               # default gain 
        }
	elsif($usemaroc2gainconstantsmb eq "yes" && $mysqlload == 0){ 
        	print "Error cannot load mysql gain constants from MySQL\n";
        	com_usb $usb_local, $pmt_local, 74, 0b0100000;               # default gain 
        }
	elsif($usemaroc2gainconstantsmb eq "yes" && $mysqlload == 1){
   		for(my $index=1;$index<65;$index++){
			com_usb $usb_local, $pmt_local, $index - 1, 2*$gain[$index]; # applying gain constants from MySQL
    		}
	}

        tarry 0.1;

        print ".";

	com_usb $usb_local, $pmt_local, 109, 0;             		      # vdd_fsb off

	com_usb $usb_local, $pmt_local, 254, 1;             		      # trigger on

#here create a first summary file or append to an existing one

       if($summary eq "on") {

	my $summaryfile = "${dir2}/summary.txt";
	my $summaryfile1 = "${dir2}/summary_new.txt";

	if(-e $summaryfile) {
	    open IN, $summaryfile or die $!;
	    open OUT, ">$summaryfile1" or die $!;
	    while(<IN>) {
		chomp;
		print OUT "$_\t";
    	    }
    	    print OUT "\n";
    	    print OUT "${pmt_local} \t ${pmtserialnumber} \t ${pmt_local} \t ${HVsetting} \t  ${DACt} \t ${gate} \t ${trigger_mode} \t ${pipedelay} \t ${force_trig} \t ${filelength} \t ${run_length} \t ${usemaroc2gainconstantsmb} \t ${comments} ";
    	    if($usemaroc2gainconstantsmb eq "yes") {
	       for(my $index=1;$index<65;$index++){
	           print OUT "$gain[$index] \t";       
	       }
            }
            close OUT or die $!;
            close IN or die $!;	
            system qq|mv "$summaryfile1" "$summaryfile"|;		
        } else {
            open OUT, ">$summaryfile" or die $!;
            print OUT "${pmt_local} \t ${pmtserialnumber} \t ${pmt_local} \t ${HVsetting} \t  ${DACt} \t ${gate} \t ${trigger_mode} \t ${pipedelay} \t ${filelength} \t ${run_length} \t ${usemaroc2gainconstantsmb} \t ${comments} \t";
            if($usemaroc2gainconstantsmb eq "yes") {
	        for(my $index=1;$index<65;$index++){
		    print OUT "$gain[$index] \t";       
		}
            }
            close OUT or die $!;
        }

      } # end for summary on or off

} # end loop over pmt

   tarry 3.;

   print "\n";

#   usb 0, 0;                           # auto token off


}
#############################################################################################################

sub initializetriggerbox {

    my $boxini;
    my $boxfin;
    #my $boxini = shift; #as unique number
    #my $boxfin = shift; #as unique number

    if(!defined($boxini = shift) || !defined($boxfin = shift)) { #both not defined
	$boxini = 1;
	$boxfin = $totalbox;
    }

    if($mode eq 'debug'){
	my $totalfans = $boxfin - $boxini + 1;
	if($totalbox != $totalfans) {
	    print "totalbox: $totalbox\ttotalfans: $totalfans\n";
	    print "Problem with initialization \n";
	}
    }

    my $box1;
    for ($box1 = $boxini; $box1<=$boxfin; $box1++) {

	if(!defined($usb_local = $boxtousb[$box1]) || !defined($pmt_local = $boxtoboard[$box1])) {
	    print "usb_local or pmt_local not defined.\n";
	}
	getpmtdata (${usb_local},${pmt_local});
        usb_usb $usb_local, 0, 1;                           # turn on auto token
	com_usb $usb_local, $pmt_local, 252, $trigger_mode; # control flags=
	                                                    # (flag == 1### --> test)
                                                            # (flag == 0000 fan in/out)
                                                            # (flag == 0001 X&Y trigger)
                                                            # (flag == 0010 edge strip mode)
	com_usb $usb_local, $pmt_local, 254, 0;             # enable trigger
	push(@usblocal, $usb_local);                        # prepare for set_inhibit
    }

    foreach $usb_local (@usblocal) {
        if($structure[$usb_local] ==0) { 
	   set_inhibit_usb $usb_local, -3;     #  -3; created file structure
           $structure[$usb_local] = -1;        # do not take baselines for fan-in modules
        }
    }
}


###########################################################################################


sub test_system_time {


$DataPath = "OVTime/DATA";
$disk = 2;  # AJF: This is also set in startOVDAQ.pl.  The setting here makes that one moot.
set_data_path($DataPath);
check_diskspace;
set_data_disk $disk;

#online table from database

$onlinetable = "online";
loadconfig("mysql");         #this will load the configuration from MySQL;

my $usbread;
my $usb_local;
my $box_local;
my $pmt_local;

#### defining the run number for this data taking

my $file_run;
my $max = 0;
my @files_run;
my ($sec,$min,$hour,$day,$month,$yr19,@rest) = localtime(time);
($oldsec,$oldmin,$oldhour,$oldday,$oldmonth,$oldyr19,@oldrest) = localtime(time);

$yr19 = $yr19 + 1900; # + 1900;                           # hack to get something for no
$month = sprintf("%0.2d",${month}+1);
$day = sprintf("%0.2d",$day);
$hour = sprintf("%0.2d",$hour);
$min = sprintf("%0.2d",$min);
$run = "${yr19}${month}${day}_${hour}_${min}";
$date = "${yr19}${month}${day}";

@files_run = <${DataFolder}/Run_*>;                       # DataFolder is set in check_diskspace

foreach $file_run (@files_run) {
  if($file_run =~ /Run_(\d+)$/) {
    $max = $1 if $1 > $max;
  }
}  

$max = $max + 1;

$time_run_number = sprintf("%0.7d",$max);

set_run_number "$time_run_number";                         # define the run_number and pass it to the DAQ

${Time_Data_Path} = "${DataFolder}/Run_${time_run_number}/binary";

print "Initializing time check for the OV .";

for (my $pmt1 = 1; $pmt1<=$totalpmt; $pmt1++) {
	
    if(!defined($usb_local = $pmttousb[$pmt1]) || !defined($pmt_local = $pmttoboard[$pmt1])) {
    	print "usb_local or pmt_local not defined.\n";
    }

    getpmtdata (${usb_local},${pmt_local});
    usb_usb $usb_local, 0, 1;                           # auto token on
    com_usb $usb_local, $pmt_local, 110, 1;	          # turn off the three led on the PMT's board = 1
    com_usb $usb_local, $pmt_local, 109, 1;             # vdd_fsb on
    com_usb $usb_local, $pmt_local, 73, 0b00000;        # set up pmt module
    com_usb $usb_local, $pmt_local, 255, 0;             # clear buffers
    com_usb $usb_local, $pmt_local, 84, 255;            # buffer size limit  
    com_usb $usb_local, $pmt_local, 74, 0b0100000;      # default gain
    com_usb $usb_local, $pmt_local, 70, 0;              # load default
    com_usb $usb_local, $pmt_local, 67, 0b000010;       # statea
    com_usb $usb_local, $pmt_local, 68, 0b000000;       # stateb 
    com_usb $usb_local, $pmt_local, 69, 0b000000;       # statec
    com_usb $usb_local, $pmt_local, 71, 0;              # rst_g to maroc
    com_usb $usb_local, $pmt_local, 72, 0;              # write config
    com_usb $usb_local, $pmt_local, 80, 5;              # hold delay is variable. Has been fixed to 5 here	
    dac_usb $usb_local, $pmt_local, 1000;               # threshold value
    com_usb $usb_local, $pmt_local, 73, 0b11110;        # gate alladc gate on -> 0b11110
                                                        #      alladc gate off-> 0b01110
    com_usb $usb_local, $pmt_local, 85, $pipedelay;         # set up pipe delay

    com_usb $usb_local, $pmt_local, 75, 0b01100000;     # trigger mode
    com_usb $usb_local, $pmt_local, 86, 0;              # edge strip mode is off
    com_usb $usb_local, $pmt_local, 87, 11;             # force readout -> 01: 1msec, 10: 16msec, 11: 256msec
    
    com_usb $usb_local, $pmt_local, 74, 0b0100000;      # default gain for the 64 channels 
    
    tarry 0.01;
    
    print ".";
    
    com_usb $usb_local, $pmt_local, 109, 0;             # vdd_fsb off

    com_usb $usb_local, $pmt_local, 254, 1;             # enable trigger
 
   tarry 0.01;

} #end loop over pmt

print "\n";

#initializing data taking for the trigger boxes

    usb_usb $usb_box, 0, 1;                             # turn on auto token

for (my $i = 1; $i<=$totalbox; $i++) {

    if(!defined($usb_local = $boxtousb[$i]) || !defined($box_local = $boxtoboard[$i])) {
	print "usb_local or pmt_local not defined.\n";
    }

    com_usb $usb_local, $box_local, 252, 0b1001;            # control flags=
	                                                    # (flag == 1### --> test)
                                                            # (flag == 0000 fan in/out)
                                                            # (flag == 0001 X&Y trigger)
                                                            # (flag == 0010 edge strip mode)
    com_usb $usb_local, $box_local, 254, 0;                 # enable trigger
    tarry 0.5;
}

foreach $usbread (keys(%usbhowmanyboards)){
     set_inhibit_usb $usbread, -3;     #  -3; create file structure
}

foreach $usbread (keys(%usbhowmanybox)){
     set_inhibit_usb $usbread, -3;     #  -3; create file structure
}


tarry 1.;

print "start data taking ....\n";

foreach $usbread (keys(%usbhowmanyboards)){
     set_inhibit_usb $usbread, 0;     #  start_taking_data
}

foreach $usbread (keys(%usbhowmanybox)){
     set_inhibit_usb $usbread, 0;     #  start_taking data
}


#data taking in progress -----<

}

###########################################################################################

sub check_system_time {

    my $filesize;
    my %readfilesize;
    my $usbread;

    my $i=0;
    my $diff;

    open IN, "ls -ltr --time-style=long-iso ${Time_Data_Path}|";
    while(<IN>) {
        if(/ (\S+) \d+-\d+-\d+ \d+:\d+ (\d+)_(\d+)/) { 
	    #print "$1, $2\n";
	    $filesize=$1;
	    #print "USB $3 - Size: $filesize\n"; 
            if($3 != $usb_box) {
               $readfilesize{$3} += $filesize;
               $i++;
            }
	  }
    }
    close IN;

    my $maxdim = 0;
    my $mindim = 1000000000;

    if( keys(%readfilesize) != keys(%usbhowmanyboards) ) { print "Problem in the number of USB file found (".keys(%readfilesize).") in path $Time_Data_Path expected ".keys(%usbhowmanyboards)." \n"; }

    foreach $usbread (keys(%readfilesize)){
        if($usbread!=$usb_box) {
            $readfilesize{$usbread}/=$usbhowmanyboards{$usbread}; #this normalize the file dimension by the pmt boards
	    if($readfilesize{$usbread} > $maxdim) { $maxdim = $readfilesize{$usbread}; }
	    if($readfilesize{$usbread} < $mindim) { $mindim = $readfilesize{$usbread}; }
        }
    }
    
    $diff = $maxdim - $mindim;

    if($maxdim == 0 or $mindim == 0) {
	print "Got a problem Number of packet is ZERO!\n"; 
	gaibu_msg($MERROR,"Got a problem Number of packet is ZERO!"); 
        
    }
 
    if($diff > 300) { 
	print "We got a problem with the timing info DeltaDim(Max-Min) between files=$diff \n"; 
	#gaibu_msg($MERROR,"We got a problem with the timing info DeltaDim(Max-Min) between files=$diff"); 
	syslog('LOG_ERR',"We got a problem with the timing info DeltaDim(Max-Min) between files=$diff"); 
        sendmail("OVRC","ERROR","We got a problem with the timing info DeltaDim(Max-Min) between files=$diff"); 
    }
}
###########################################################################################

sub starttakedata {

    my $pmtini;
    my $pmtfin;
#my $pmtini = shift; #as unique number
#my $pmtfin = shift; #as unique number
my $usb_local;

if(!defined($pmtini = shift) || !defined($pmtfin = shift)) { #both not defined
   $pmtini = 1;
   $pmtfin = $totalpmt;
}

if($mode eq 'debug'){
   my $totalboard = $pmtfin - $pmtini + 1;
   print "$totalpmt\t$totalboard\t\n";
   if($totalpmt != $totalboard) {
     print "Problem with initialization at starttakedata \n";
   }
}
else {
	$pmtini = 1;
        $pmtfin = $totalpmt;
}

    my $boxini;
    my $boxfin;
#my $boxini = shift; #as unique number
#my $boxfin = shift; #as unique number

if(!defined($boxini = shift) || !defined($boxfin = shift)) { #both not defined
    $boxini = 1;
    $boxfin = $totalbox;
}

if($mode eq 'debug'){
   my $totalfans = $boxfin - $boxini + 1;
   print "$totalfans\t$totalbox\t\n";
   if($totalbox != $totalfans) {
     print "Problem with initialization of fan-in modules at starttakedata \n";
   }
}
else {
	$boxini = 1;
        $boxfin = $totalbox;
}

for (my $pmt1 = $pmtini; $pmt1<=$pmtfin; $pmt1++){

	if(!defined($usb_local = $pmttousb[$pmt1]) || !defined($pmt_local = $pmttoboard[$pmt1])) {
	    print "usb_local or pmt_local not defined.\n";
	}
#        usb_usb $usb_local, 0, 1;                                    # auto token on
#        com_usb $usb_local, $pmt_local, 254, 1;                      # enable trigger
#        tarry 0.01;                                                  # give it some time
}

for (my $box1 = $boxini; $box1<=$boxfin; $box1++){

	if(!defined($usb_local = $boxtousb[$box1]) || !defined($pmt_local = $boxtoboard[$box1])) {
	    print "usb_local or pmt_local not defined.\n";
	}
#	usb_usb $usb_local, 0, 1;                                    # auto token on
#	com_usb $usb_local, $pmt_local, 254, 1;                      # enable trigger
#        tarry 0.01;                                                  # give it some time
}




foreach $usb_local (@usblocal) {
    if($structure[$usb_local] == -1) {   # only trigger box !!
        set_inhibit_usb $usb_local, 0;     #  -2; release inhibit for baseline
                                           #  -1; inhibit writing data
                                           #   0;  release inhibit for writing data
	$structure[$usb_local] = 1;
    }
}

($oldsec,$oldmin,$oldhour,$oldday,$oldmonth,$oldyr19,@oldrest) = localtime(time);

print ".....Taking data .....\n";

}

################################################################

sub stoptakedata {

    my $pmtini;
    my $pmtfin;
#my $pmtini = shift; #as unique number
#my $pmtfin = shift; #as unique number

if(!defined($pmtini = shift) || !defined($pmtfin = shift)) {#both not defined
   $pmtini = 1;
   $pmtfin = $totalpmt;
}

if($mode eq 'debug'){
   my $totalboard = $pmtfin - $pmtini + 1;
   if($totalpmt != $totalboard) {
     print "Problem with initialization at stoptakedata \n";
   }
}
else {
	$pmtini = 1;
        $pmtfin = $totalpmt;
}

    my $boxini;
    my $boxfin;
#my $boxini = shift; #as unique number
#my $boxfin = shift; #as unique number

if(!defined($boxini = shift) || !defined($boxfin = shift)) { #both not defined
    $boxini = 1;
    $boxfin = $totalbox;
}

if($mode eq 'debug'){
   my $totalfans = $boxfin - $boxini + 1;
   if($totalbox != $totalfans) {
     print "Problem with initialization at stoptakedata \n";
   }
}
else {
	$boxini = 1;
        $boxfin = $totalbox;
}

    my ($newsec,$newmin,$newhour,$newday,$newmonth,$newyr19,@newrest) =   localtime(time);
    $elapsed_time = ($newday-$oldday)*24*3600+ ($newhour-$oldhour)*3600 + ($newmin-$oldmin)*60 + ($newsec - $oldsec);

    print "($elapsed_time sec...)";

for (my $pmt1 = $pmtini; $pmt1<=$pmtfin; $pmt1++){

	if(!defined($usb_local = $pmttousb[$pmt1]) || !defined($pmt_local = $pmttoboard[$pmt1])) {
	    print "usb_local or pmt_local not defined.\n";
	}
#        com_usb $usb_local, $pmt_local, 255, 0;                      # enable trigger
#        tarry 0.01;                                                  # give it some time
}

print "shutting down ";

for(my $m=0; $m<=10; $m++){
   print ".";
   tarry 0.2;
}
print "done\n";

usb 0,0;
foreach $usb_local (@usblocal) {
    if($structure[$usb_local] == 1) {
           set_inhibit_usb $usb_local, -1;    #  -2; release inhibit for baseline
                                              #  -1; inhibit writing data
                                              #   0;  release inhibit for writing data
           $structure[$usb_local] = -1;

    }
}
tarry 1;

}

#######################################################################################

sub loadrunsummarymysql {

#this is going to be called at the beginning of each run

    my $tmp_emptyvalue = "";

eval {
  $dsn = "dbi:mysql:database=$database;host=$DBhostname";

  $dbh = DBI->connect($dsn, $user, $password);

  $drh = DBI->install_driver("mysql");  
		
  $dbh->do("INSERT INTO OV_runsummary (Run_number, DATE, start_time, HV, RUN_length, Run_Type, Shifter, HW_Threshold, SW_Threshold, SW_TriggerMode, config_table, use_DOGSifier, daq_disk, daq_disk_usage, RCcomment, bin_erased, process_finish) VALUES ('${run_number}', '$date', '$starttime', '$HV', '$run_length', '$run_OVtype', '$run_shifter', '$HWthreshold', '$threshold', '$trigger', '$onlinetable', '$usedogsifier', '$disk', '$disk_usage', '$run_comment', '0', '0')");

  $dbh->disconnect();
};

  if($@){
   print "Failed to import into MySQL info for this run: $@\n" if $@;
   gaibu_msg($MWARNING, "OV DAQ: Failed to import intial run info inside MySQL");
  }

}

##################################################################################################
sub updaterunsummarymysql {

#this can be called in the middle of the run for adding comments but should also be
#called at the end to write down the stop time

my $tmpvariable = shift;
my $tmpvalue = shift;

eval {
  $dsn = "dbi:mysql:database=$database;host=$DBhostname";

  $dbh = DBI->connect($dsn, $user, $password);

  $drh = DBI->install_driver("mysql");  

  if($tmpvariable eq "starttime") {
    $dbh->do("UPDATE OV_runsummary SET start_time='$tmpvalue' WHERE Run_number='${run_number}'"); 
  }
  elsif($tmpvariable eq "stoptime") {
    $dbh->do("UPDATE OV_runsummary SET stop_time='$tmpvalue' WHERE Run_number='${run_number}'"); 
  }
  elsif($tmpvariable eq "RCcomment") {
    $dbh->do("UPDATE OV_runsummary SET RCcomment='$tmpvalue' WHERE Run_number='${run_number}'"); 
  }
  elsif($tmpvariable eq "comment") {
    $dbh->do("UPDATE OV_runsummary SET Comment='$tmpvalue' WHERE Run_number='${run_number}'"); 
  }
  elsif($tmpvariable eq "HV") {
    $dbh->do("UPDATE OV_runsummary SET HV='$tmpvalue' WHERE Run_number='${run_number}'"); 
  }
  elsif($tmpvariable eq "SWthreshold") {
    $dbh->do("UPDATE OV_runsummary SET SW_Threshold='$tmpvalue' WHERE Run_number='${run_number}'"); 
  }
  elsif($tmpvariable eq "wheretorun") {
    $dbh->do("UPDATE OV_runsummary SET nfs_run='$tmpvalue' WHERE Run_number='${run_number}'"); 
  }
  $dbh->disconnect();
};

#error message
 if($@){
   print "Failed to import into MySQL info for this run: $@\n";
   gaibu_msg($MWARNING, "OV DAQ: Failed to update run info inside MySQL");
 }

}

##################################################################################################

sub OVCalibSummary {


my $start_time = shift;
#my $stop_time = shift;
$date = shift;
$run_number =shift;


eval {

  $dsn = "dbi:mysql:database=$database;host=$DBhostname";

  $dbh = DBI->connect($dsn, $user, $password);

  $drh = DBI->install_driver("mysql");  

  $dbh->do("INSERT INTO OV_CalibSummary SET Run_number='$run_number', date='${date}',start_time='${start_time}',spread=0"); 


  $sth = $dbh->prepare("SELECT spread FROM OV_CalibSummary WHERE Run_number='$run_number'"); 
  $sth->execute();
  while (my $ref = $sth->fetchrow_arrayref()) {
      $spreadCalib = $ref->[0];
  }
  $sth->finish();  


  $dbh->disconnect();
};

#error message
 if($@){
   print "Failed to import into MySQL Calib info: $@\n";
   gaibu_msg($MWARNING, "OV DAQ: Failed to update Calib info inside MySQL");
 }

}

######################################################################################

sub updateOVCalibSummary {

    my $stoptime = shift;

eval {

  $dsn = "dbi:mysql:database=$database;host=$DBhostname";

  $dbh = DBI->connect($dsn, $user, $password);

  $drh = DBI->install_driver("mysql");  

  $dbh->do("UPDATE OV_CalibSummary SET stop_time = '$stoptime' WHERE Run_number=$run_number"); 

  $dbh->disconnect();
};

#error message
 if($@){
   print "Failed to update into MySQL the Calib info: $@\n";
   gaibu_msg($MWARNING, "OV DAQ: Failed to update Calib info inside MySQL");
 }


}

#####################################################################################

sub sendmail {

my $thisServer = "dcfovdaq.in2p3.fr";	        #the hostname of the server this script is installed on
my $smtpServer = "smtp.nevis.columbia.edu";	#the smtp server we want to connect to
my $debug = 0;		
# changed MHT 4/5/2011 (but not actually)
my $from = "dcdaq\@nevis.columbia.edu";
my $to = "dc-ov-expert\@listserv.nevis.columbia.edu";
my $cc = "dcdaq\@nevis.columbia.edu";
my $process = shift;
my $errorlevel = shift;
my $message_description = shift;
my $subject = "$process - $errorlevel";

my ($sec,$min,$hour,$day,$month,$yr19,@rest) = localtime(time);
$yr19 = $yr19 + 1900; # + 1900; # hack to get something for no                                                                                
$month = sprintf("%0.2d",${month}+1);
$day = sprintf("%0.2d",$day);
$hour = sprintf("%0.2d",$hour);
$min = sprintf("%0.2d",$min);
$sec = sprintf("%0.2d",$sec);
$date = "${yr19}${month}${day}";
$time = "$hour:$min:$sec";

my $Run_Number = 9999999;

if($run_number < 0 || !defined($run_number) ) {
    $Run_Number = 9999999 
}
else {
    $Run_Number = sprintf("%0.7d",$run_number);
}

#send an email
	if(my $smtp = Net::SMTP->new($smtpServer, Hello => $thisServer, Debug => $debug, Timeout => 2)){
		$smtp -> auth("dcdaq","lithium137nine");	#trying to authenticate
		$smtp -> mail($from);   #send server sending email address
		$smtp -> to($to);  #send server recieving email address
		$smtp -> cc($cc);
		$smtp -> data();
		$smtp->datasend("From: $from \n");
		$smtp->datasend("To: $to \n");
		$smtp->datasend("Cc: $cc \n");
		$smtp -> datasend("Subject: $subject");
		$smtp -> datasend("\n");
		$smtp -> datasend("$date\n");
		$smtp -> datasend("$time\n");
		$smtp -> datasend("$Run_Number\n");
		$smtp -> datasend("$process\n");
		$smtp -> datasend("$message_description");
		$smtp -> dataend();
		$smtp -> quit();
	}
	else{
		print "ERROR: Could not connect to SMTP server $debug\n";
	}	
}

#####################################################################################################

sub gaibu_msg {

    my $log_priority;
    my $priority = shift;
    my $msg = shift;
#    print "Gaibu IP: $gaibu_ip \n";

    # Gaibu client definition
    # check if socket is closed
    if (!$client_sock) {
	#print "Trying to connect to Gaibu Server at ${gaibu_ip}:${gaibu_port}\n";
	$client_sock = new IO::Socket::INET ( 
	    PeerAddr => $gaibu_ip,
	    PeerPort => $gaibu_port,
	    Proto => 'tcp', ); 
	eval { $client_sock->send ("OVRunControl\n"); };
	if ($@) { syslog('LOG_WARNING', "OVRC: Gaibu server connection broken. Unsent message: $msg"); }
    }

    if($client_sock) {
        eval { 
		$client_sock->send("$priority $msg\n");
	};
	if ($@) { print "connection closed ! \n"; 
		  undef $client_sock;
		  syslog('LOG_WARNING', "OVRC: Gaibu server connection broken. Unsent message: $msg");  
		}
    }

    if($priority > 1) {
	if($priority == 2) { $log_priority = "LOG_NOTICE"; }
	elsif($priority == 3) { $log_priority = "LOG_WARNING"; }
	elsif($priority == 4) { $log_priority = "LOG_ERR"; }
	else { $log_priority = "LOG_CRIT"; }
	syslog("$log_priority", "$msg");
    }

}

#####################################################################################################

sub gaibu_msg_serv {

    my $serv = shift;
    my $log_priority;
    my $priority = shift;
    my $msg = shift;

    # Gaibu client definition
    # check if socket is closed
    if (!$client_sock) {
	print "Trying to connect to Gaibu Server at ${gaibu_ip}:${gaibu_port}\n";
	$client_sock = new IO::Socket::INET ( 
	    PeerAddr => $gaibu_ip,
	    PeerPort => $gaibu_port,
	    Proto => 'tcp', ); 
	eval { $client_sock->send ("OV_$serv\n"); };
	if ($@) { syslog('LOG_WARNING', "OV $serv: Gaibu server connection broken. Unsent message: $msg"); }
    }

    if($client_sock) {
        eval { 
		$client_sock->send("$priority $msg\n");
	};
	if ($@) { print "connection closed ! \n"; 
		  undef $client_sock;
		  syslog('LOG_WARNING', "OV $serv: Gaibu server connection broken. Unsent message: $msg");  
		}
    }

    if($priority > 1) {
	if($priority == 2) { $log_priority = "LOG_NOTICE"; }
	elsif($priority == 3) { $log_priority = "LOG_WARNING"; }
	elsif($priority == 4) { $log_priority = "LOG_ERR"; }
	else { $log_priority = "LOG_CRIT"; }
	syslog("$log_priority", "$msg");
    }
}

#####################################################################################################



# determine looking at nfs mount point if running with the local version or the NFS one

sub where_to_run {

    my $client_ip = shift;

    my $mount_infile;

#initialization

    $nfs_mount = 0;  #initialize a check for NFS folder config file change

    $online_path = "/local/home/dconline/myOnline"; # default remote machine runs local
    $remote_path = "/local/home/dconline/myOnline"; # default remote machine runs local

    $online_mount_path = $online_path; 
    
    $client_sock = 0;

#check who is connected to us

    if( $client_ip =~ /$OVRC_IP/ or $client_ip =~ /$OVEVENTBUILDER_IP/ ) {
	#at this point I run locally
	print "Client is connected from an OV machine, force running in local mode. \n";
        #everything is already set to run locally
    } 
    else {
	#try to run on NFS for the EB and OM
	if(defined($ENV{'DCONLINE_MOUNT'})) {
	    $online_mount_path = $ENV{'DCONLINE_MOUNT'};
	    $mount_infile = $online_mount_path . "/config/";
	    
	    if (-e "$mount_infile"){
		$nfs_mount = 1;                                   # the NFS mount point is defined
		$remote_path = $ENV{'DCONLINE_MOUNT'}; #"/dcfovserv/dconline"; #"/nfs/dconline";                  # remote machine NFS mount $HOME
		#at this point online_path is still local
		#but remote_path become remote
		print "Local installation of DCOnline in use for the DAQ but Remote Path ${remote_path} is in use for the EB \n";
	    }
	    else {
		print "NFS config path not found, local copy in use ! \n";
	    }	    
    	}
    	else { 
	    print "Environment variable DCONLINE_MOUNT not found. Using local copy.\n"; 
    	}
    }
}


#####################################################################################################

sub load_ip_config {

    my $ip_file = shift;

    my $debug = 0;
   
    my $infile = $remote_path . "/config/${ip_file}";

    open IN, $infile or die $!;
    while(<IN>) {
	next if($_ =~ /^\s*#(.*)/);

	if($_ =~ /^DCOVDAQ_HOST\s*:(\S+).*/) { #\s*:(\S+)/) {
	    $OVDAQ = $1;
	    if($debug eq 1){ print "ov daq host = $1\n" };
	}
	elsif($_ =~ /^DCOVDAQ_IP\s*:(\S+).*/) {
	    $OVRC_IP = $1;
	    if($debug eq 1){ print "ov daq ip = $1\n" };
	}
	elsif($_ =~ /^DCOVCC_IP\s*:(\S+).*/) {
	    $OVCC_IP = $1;
	    if($debug eq 1){ print "ov cc ip = $1\n" };
	}
	elsif($_ =~ /^DCOVEVENTBUILDER_HOST\s*:(\S+).*/) {
	    $OVEVENTBUILDER_HOST = $1;
	    if($debug eq 1){ print "event builder host =$1\n" };
	}
	elsif($_ =~ /^DCOVEVENTBUILDER_IP\s*:(\S+).*/) {
	    $OVEVENTBUILDER_IP = $1;
	    if($debug eq 1){ print "event builder ip = $1\n" };
	}
	elsif($_ =~ /^DCOVONLINEMONITOR_HOST\s*:(\S+).*/) {
	    $OVONLINEMONITOR_HOST = $1;
	    if($debug eq 1){ print "online monitor host = $1\n" };
	}
	elsif($_ =~ /^DCOVONLINEMONITOR_IP\s*:(\S+).*/) {
	    $OVONLINEMONITOR_IP = $1;
	    if($debug eq 1){ print "online monitor ip = $1\n" };
	}
	elsif($_ =~ /^DCSLOWCONTROL_OV_HOST\s*:(\S+).*/) {
	    $OVSERV_HOST = $1;
	    if($debug eq 1){ print "Slow monitor host = $1\n" };
	}
	elsif($_ =~ /^DCSLOWCONTROL_OV_IP\s*:(\S+).*/) {
	    $OVSERV_IP = $1;
	    if($debug eq 1){ print "Slow monitor ip = $1\n" };
	}
	elsif($_ =~ /^DCGAIBU_HOST\s*:(\S+).*/) {
	    if($debug eq 1){ print "gaibu host = $1\n" };
	}
	elsif($_ =~ /^DCGAIBU_IP\s*:(\S+).*/) {
	    $gaibu_ip = $1;
	    if($debug eq 1){ print "gaibu ip = $1\n" };
	}
	elsif($_ =~ /^DCRUNCONTROL_HOST\s*:(\S+).*/) {
	    $runcontrol_hostname = $1;
	    if($debug eq 1){ print "rc_hostname = $1\n" };
        }
    }
    close IN or die $!;

#little check that the file is up to date
#check on the IP address

#    my $address = inet_ntoa( (gethostbyname($hostnamepc))[5]); # [4] is 127.0.1.1, [5] is 10.44.1.104

    #print "address: ${address}\tOVRC_IP: ${OVRC_IP}\t${hostnamepc}\n";
    #if ($address ne $OVRC_IP) {
    #	print "There is a problem with the setting in the DCSpaceIP.config \n";
    #     gaibu_msg($MERROR, "OV RC - DCSpaceIP.config is not correct");
	#exit(1);
    #}

}

###############################################################################

sub load_port_config {

    my $port_file = shift;

    my $infile = $remote_path . "/config/${port_file}";


    open IN, $infile or die $!;
    while(<IN>) {
	next if($_ =~ /^\s*#(.*)/);

	if($_ =~ /^DCOVDAQ_PORT\s*(\d+).*/) {
	    $OVRC_Port = $1;
	    #print "$1\n";
	}
	elsif($_ =~ /^DCOVCC_PORT\s*(\d+).*/) {
	    $OVCC_Port = $1;
	    #print "OVCC_Port = $1\n";
	}
	elsif($_ =~ /^DCGAIBU_PORT\s*(\d+).*/) {
	    $gaibu_port = $1;
	    #print "$1\n";
	}
    }
    close IN or die $!;

}

###############################################################################

sub load_db_config {

    my $db_file = shift;
  
    my $infile = $remote_path . "/config/${db_file}";

    open IN, $infile or die $!;
    while(<IN>) {
	next if($_ =~ /^\s*#(.*)/);

	if($_ =~ /^DCDB_SERVER_HOST\s*:(\S+).*/) { #\s*:(\S+)/) {
	    $DBhostname = $1;
	    #print "$1\n";
	}
	elsif($_ =~ /^DCDB_OV_USER\s*:(\S+).*/) {
	    $user = $1;
	    #print "$1\n";
	}
	elsif($_ =~ /^DCDB_OV_PASSWORD\s*:(\S+).*/) {
	    $password = $1;
	    #print "$1\n";
	}
	elsif($_ =~ /^DCDB_OV_DBNAME\s*:(\S+).*/) {
	    $database = $1;
	    if($database =~/^doublechooz_ov_(\S+)/){
		$detector_configuration = $1;
	    }
	    elsif($database =~/^dc_ov_(\S+)/){ #Adding this to accomodate DCOnline convention --AJF, 4/5/2011
		$detector_configuration = $1;
	    }
	    if($detector_configuration eq 'fd'){ #Adding this to accomodate DCOnline convention. --AJF, 4/5/2011
		$detector_configuration = 'far';
	    }
	    elsif($detector_configuration eq 'nd'){
		$detector_configuration = 'near';
	    } #AJF: add exception cases here?
	    #print "We are now running the following detector config.: $detector_configuration\n";
	}

	elsif($_ =~ /^DCDB_HVOV_DBNAME\s*:(\S+).*/) {
	    $HVdatabase = $1;
	}
	elsif($_ =~ /^DCDB_HVOV_USER\s*:(\S+).*/) {
	    $HVuser = $1;
	}
	elsif($_ =~ /^DCDB_HVOV_PASSWORD\s*:(\S+).*/) {
	    $HVpassword = $1;
	}
    }
    close IN or die $!;

}

#################################################################################################
sub checknumusbs{

    my %unique;
    for (@pmttousb) { $unique{$_} = 1 if $_; }
    for (@boxtousb) { $unique{$_} = 1 if $_; }
    my $expected = keys %unique;

    my $usbcnt=0;
    open IN, "lsusb |" or die $!;
    while(<IN>) {
	if($_ =~ /:dead/) { $usbcnt++; }
    }
    close IN;

    if($usbcnt != $expected) { 
	print "OV RC did not find expected number of USB devices connected\n";
	print "Expected from mySQL: $expected\tFound with lsusb: $usbcnt\n";
	gaibu_msg($MERROR,"Number of USB found in the system inconsistent with MySQL config");
	#exit(0);
    }

}
#################################################################################################
sub checkHVstatus{

#Subroutine to check the HV status - to run just at the beginnig og the run 

my $config = "hvOV";
my $nCrates = 1;
my $nSlots = 9;
my $nChannels = 16;
my $restart_required = 0;
if(!defined($user_online)) {$user_online = "dconline"; }

#check if the HVserver and HVMonitor are running only if on dcfovserv
my ($hostname_full,$HV_server_running,$HV_monitor_running);

$hostname_full = sprintf("%s.in2p3.fr",$hostnamepc);

if(${OVSERV_HOST} eq ${hostname_full})  {

    $HV_server_running = `pidof ./DCHVControlServer`;

    $HV_monitor_running = `pidof ./DCHVMonitorServer`;
    #MHT, 072611: Do not send email, just write to syslog
    if($HV_server_running eq "" ) {
	#gaibu_msg($MWARNING,"OVHV Server process is not running");
	syslog('LOG_WARNING',"OVHV Server process is not running");
	#sendmail("OVRC", "WARNING","OVHV Server Process is not running");
#    print "Result: $HV_server_running \n";
	print "Something wrong with the HV Server process \n";
	$restart_required = 1;
	goto end;
    }

    if($HV_monitor_running eq "" ) {
	#gaibu_msg($MWARNING,"OVHV Monitor process is not running");
	syslog('LOG_WARNING',"OVHV Monitor process is not running");
	#sendmail("OVRC", "WARNING","OVHV Monitor Process is not running");
#    print "Result: $HV_monitor_running \n";
	print "Something wrong with the HV Monitor process \n";
	$restart_required = 1;
    goto end;
    }

}


#I need the db connection.
my $dsn = "dbi:mysql:database=$HVdatabase;host=$DBhostname";

# now connect and get a database handle  
my $dbh = DBI->connect($dsn, $HVuser, $HVpassword)
 or die "Can’t connect to the DB: $DBI::errstr\n"; 

# also I need a time for reference
# this is lame I know.

my $timemonitormin = 2;  # set the maximum interval for the HV monitor in min

my $tableName;
my $tableNameSet;
my $myquery;
my $sqlQuery;
my @row;
my @isConn;
my @isON;
my @VSet;
my @VSetSig;
my @ISet;
my @ISetSig;
my @VMon;

my $nChan=0;
my $nChanOn=0;
my $nChanOff=0;
my $nChanGood=0;
my $nChanBad=0;

for(my $i=0; $i < $nCrates; $i++){
    for(my $j=0; $j <= $nSlots; $j++){
	
	$tableName = sprintf("history_%s_Vmon_Crate%dSlot%d",$config, $i, $j);
	$tableNameSet = sprintf("history_%s_Set_Crate%dSlot%d",$config, $i, $j);
	
	#print "$tableName\n";
	
	#Get the settings.
	$myquery = sprintf("select * from %s",$tableNameSet);
	$sqlQuery  = $dbh->prepare($myquery);
	$sqlQuery->execute();
	@row = $sqlQuery->fetchrow_array;

	my $nval =0;
	my $nchn =0;
	my $index=0;
	foreach my $value (@row){
	    if($nval == 0){
		 #print "Settings from $value ";
		
	    }else{
		if($index==0){ $isConn[$nchn] = $value; }
		if($index==1){ $isON[$nchn] = $value; }
		if($index==2){ $VSet[$nchn] = $value; }
		if($index==3){ $VSetSig[$nchn] = $value; }
		if($index==4){ $ISet[$nchn] = $value; }
		if($index==5) { 
		    $ISetSig[$nchn] = $value;
		    $nchn++;
		    $index=0;
		}else{
		    $index++;
		}
	    }
	    $nval++;
	}

	#Get Some Values...
	$myquery = sprintf("select * from %s where time>=(select DATE_SUB(NOW(), INTERVAL %d MINUTE))",$tableName,$timemonitormin);
	$sqlQuery  = $dbh->prepare($myquery);
	$sqlQuery->execute();
	@row = $sqlQuery->fetchrow_array;    

	$nval=0;
	$nchn=0;

#        print "Query=$myquery\n";
#	print "number of row read $#row \n";

	if($#row < 16){
	    print "\n--->OVHV Monitor has been off for more than $timemonitormin mins \n\n";
	    gaibu_msg($MWARNING,"OVHV Monitor has been off for more than $timemonitormin mins");
	    updaterunsummarymysql("HV","und");
	    sendmail("OVHV","WARNING","OVHV Monitor OFF");
	    $restart_required = 1;
	    goto end;
	}
	else{
	    foreach my $value (@row){
		if($nval == 0){
		    #print "VMon from $value \n";
		}else{
		    $VMon[$nchn] = $value;
		    $nchn++;
		}
		$nval++;
	    }
   
	    #Now do some accounting.
	    my $diff;
	    for(my $i=0; $i<$nChannels; $i++){
		
		if($isON[$i] && $isConn[$i]){
		    $nChanOn++;
		    $diff = $VMon[$i] - $VSet[$i];
		    $diff = sqrt($diff*$diff);	    
		    if($diff < $VSetSig[$i]){
			$nChanGood++;
		    }else{
			$nChanBad++;
		    }
		}else{
		    $nChanOff++;
		}
		
		$nChan++;
	    }
	    
	}
	#Done This Loop.
	#print "\n";
    }
}

print "OVHV Tot. Ch:${nChan},ON:${nChanOn},OFF:${nChanOff}\n";
print "Good:$nChanGood,Bad:${nChanBad}\n";

#update of runsummary table check
if( ($nChanOn) == ($nChanGood) ){
	updaterunsummarymysql("HV","ON");
#	print "OVHV is ON and Working !\n";
}
if( ($nChanBad) > 0){
        updaterunsummarymysql("HV","BAD");
#	print "OVHV is ON but some channels are bad (nChanBad)\n";
	gabu_msg($MWARNING,"HV of OV is ON but there are $nChanBad bad channels");
        sendmail("OVHV","WARNING","HV is ON but there are $nChanBad bad channels");
}
if ( $nChanOff == $nChan )
{
    gaibu_msg($MERROR,"OV-HV is off");
    updaterunsummarymysql("HV","OFF");
    sendmail("OVHV","ERROR","HV is OFF");
}

end:
    if($restart_required == 1) {
	tarry 2;
#try to restart the HV Server and Monitor on $OVSERV_HOST
	#gaibu_msg($MWARNING,"Restart the OVHV Monitor and Server");
	syslog('LOG_WARNING',"Automatic Restart the OVHV Monitor and Server performed");
	#sendmail("OVRC", "WARNING","Automatic Restart the OVHV Monitor and Server performed");
#stop the HVMonitor
	system qq|ssh ${user_online}\@${OVSERV_HOST} "${online_path}\/DCOV\/readout\/startupscript\/stop_HV.sh HVmonitor"|;
#stop the HVServer
	system qq|ssh ${user_online}\@${OVSERV_HOST} "${online_path}\/DCOV\/readout\/startupscript\/stop_HV.sh HVserver"|;
#restart HVserver and HVmonitor
	system qq|ssh ${user_online}\@${OVSERV_HOST} "${online_path}\/DCOV\/readout\/startupscript\/start_HV.sh HVserver"|;
	tarry 1;
	system qq|ssh ${user_online}\@${OVSERV_HOST} "${online_path}\/DCOV\/readout\/startupscript\/start_HV.sh HVmonitor"|;
}

}
##################################################################################

sub check_rate {

    my $folderlocation = shift;
    my $tot_pmt = shift;
    my $tot_usb = shift;
    my $boxusb = shift;
    my $switch = shift;

    my $location_baseline;
    my @files;
    my $tempfile;
    my %hit_pmt = ();  #initializing
    my %cnt_pmt = ();  #hash containing last time variable
    my %gap_pmt = ();  # hash containing gap size
    my ($diff,$tdiff);
    my $keymax = 0;
    my $keymin = 1000000;
    my $key;
    my $usbnum;
    my $found_pmt_in_time = 0;
    my $module_number;


    if(!defined($switch)){
	@files = <${folderlocation}/*_*>;
    }
    else {
	${location_baseline} = "${folderlocation}/baseline_* ";
	@files = <${location_baseline}>;
	$tot_usb = $tot_usb - 1;  #box do not get baseline file
    }


    if($#files < $tot_usb - 1) { print "No files found \n"; }
	foreach $tempfile (@files) {
	    if($tempfile =~ /_(\d+)$/) { $usbnum = $1; }
	    next if($usbnum == $boxusb);
	    open IN, "/usr/bin/perl ${dcmacropath}/decode.pl \"${tempfile}\" |";
	    while(<IN>) {
		chomp ;
		next if /^\s*$/;
		my @line = split /\s*,\s*/;
		if($line[0] eq "p") {
		    shift @line;
		    my $tmp = $line[0];
		    my $mod = ($tmp >> 8) & 0x7f;
		    $mod += 100*$usbnum;
		    $hit_pmt{$mod}++;
		    if(!$switch) {
			shift @line;
			my $tim = ($line[0] >> 8) & 0xff; # This assumes 256ms internal clocked trigger
			my $tdiff;
			if($cnt_pmt{$mod}) {
			    $tdiff = $tim - $cnt_pmt{$mod}; 
			    if($tdiff > 1) {
				$gap_pmt{$mod}{$tdiff}++;
			    }
			}
			$cnt_pmt{$mod} = $tim;
		    }
		}
	    }
	} # matches foreach $tempfile (@files) {

#check which is the missing pmt
    for(my $i=1; $i<=$tot_pmt; $i++){
	$found_pmt_in_time = 0;
	foreach $key (sort (keys(%hit_pmt))){
	    $module_number = $pmttoboard[$i] + 100*$pmttousb[$i];
	    if($key == $module_number) {
		$found_pmt_in_time = 1;
	    }
	}
	if($found_pmt_in_time == 0){
	    if(!defined($switch)) {
		print "Error: PMT Board $pmttoboard[$i] is missing from the timing check \n";
	    }
	    else {
		print "Error: PMT Board $pmttoboard[$i] is missing from the baseline file \n";
		gaibu_msg($MERROR,"Error: PMT Board $pmttoboard[$i] is missing from the baseline file \n");
		sendmail("OVRC","CRITICAL","PMT Board $pmttoboard[$i] is missing from the baseline file");
	    }
	}
    }
      
    my $tmptotpmts = keys(%hit_pmt);
    if($tmptotpmts != $tot_pmt) {
	print "Error: Number of pmts found: $tmptotpmts\tNumber of pmts expected in MySQL: $tot_pmt\n";
	gaibu_msg($MERROR,"Error: Number of pmts found: $tmptotpmts, Number of pmts expected in MySQL: $tot_pmt");
	sendmail("OVRC","ERROR","Number of pmts found $tmptotpmts expected $tot_pmt");
    }
#now find the min and max value for the hit
      foreach $key (sort (keys(%hit_pmt))){
         if($hit_pmt{$key}>$keymax){ 
		$keymax = $hit_pmt{$key};
	 }
         if($hit_pmt{$key}<$keymin) {
		$keymin = $hit_pmt{$key};
         }
      }

      $diff = $keymax - $keymin;

      if($keymax == 0 or $keymin == 0) {
	print "Got a problem Number of packet is ZERO!\n"; 
      }

      if( ($diff) > 5) {
	  if(!$switch) {
	      print "Error the difference between the trigger packet is $diff\n";
	      gaibu_msg($MNOTICE,"Error the difference between the trigger packet is $diff");
	  }
          else { 
	      print "Error the difference between baseline packets is $diff\n"; 
	      gaibu_msg($MNOTICE,"Error the difference between baseline packets is $diff"); 
	  }
      }
      else {
#         print "Check on number of packets = $keymax<->$keymin\n";
      }

    if(!$switch) {
      foreach $key (keys(%hit_pmt)){
	  my $tot_missed = 0;
	  foreach my $gapkey (keys %{$gap_pmt{$key}} ) { 
	      $tot_missed += $gapkey * $gap_pmt{$key}{$gapkey};
	  }
	  if($tot_missed > 1 and $hit_pmt{$key}) {
	      print "PMT $key missing fraction of triggers: $tot_missed/$hit_pmt{$key}\n";
	      gaibu_msg($MNOTICE, "PMT $key missing fraction of triggers: $tot_missed/$hit_pmt{$key}");
	  }
      }

    }

    if($switch and $keymin < $switch - 5) {
	print "Error minimum number of baseline packets found is $keymin\n"; 
	gaibu_msg($MERROR,"Error minimum number of baseline packets found is $keymin"); 
    }
}

##################################################################################################
sub checkOVCalibend{

    $run_number=shift;

    my $mean = 0;
    my $rms = 0;

    print "Waiting for DCOVCalib to finish processing .";

  restart:

    eval {
	
	$dsn = "dbi:mysql:database=$database;host=$DBhostname";

	$dbh = DBI->connect($dsn, $user, $password);

	$drh = DBI->install_driver("mysql");  

	$sth = $dbh->prepare("SELECT mean,RMS FROM OV_CalibSummary WHERE Run_number=$run_number"); 

	$sth->execute();

	while (my $ref = $sth->fetchrow_arrayref()) {
	    $mean = $ref->[0];
	    $rms = $ref->[1];
	}
	$sth->finish();  
        

	$dbh->disconnect();
    };
    
    while($mean == 0 && $rms == 0){
	print ".";
	tarry 20;  #wait 20s and the recheck
	goto restart;
    }

}
##################################################################################################
sub checkDOGSifier {

    my $run_num = shift;
 
    my $DOGSifier_finished = 0;
    my $EB_path;
    # Assumes where_to_run, load_ip_config, load_port_config, and load_db_config have been called.

    if(!defined($OVEVENTBUILDER_HOST)) {
	print "Error! Variable OVEVENTBUILDER_HOST not defined!\n";
    }

    eval {
	$dsn = "dbi:mysql:database=$database;host=$DBhostname";

	$dbh = DBI->connect($dsn, $user, $password);

	$drh = DBI->install_driver("mysql");

	$sth = $dbh->prepare("SELECT Path FROM OV_ebuilder WHERE Run_number='$run_num'");
	$sth->execute();
	while (my $ref = $sth->fetchrow_arrayref()) {
	    $EB_path = $ref->[0];
	}
	$sth->finish();

	$dbh->disconnect();
    };

  tryagain:
    my $file_count = 0;
    open IN, "ssh dconline\@${OVEVENTBUILDER_HOST} \"ls $EB_path/Run_$run_num/\" |" or die $!;
    while(<IN>) {
        next if $_ =~ /processed/;
        ++$file_count;
    }
    close IN or die $!;

    if($file_count > 0) {
        print ".";
        sleep 10;
        goto tryagain;
    }
    else {
        print "DOGSifier have finished processing run $run_num\n";
    }
}
##################################################################################################
sub checkforlateeventbuilder{

    my $disk = shift;
    my $Folder = shift;
    my $run_num = shift;

    print "Waiting for Ebuilder to finish .";

restart:
    my $file_count = 0;

    # counting how many .bin file are in the DAQ folder if any then the data processing
    # has not been completed
    my $path = "/data$disk/$Folder/Run_$run_num/binary";
    #print "\npath: ${path}\n";
    foreach (<${path}/*>) {
	next if $_ =~ /baseline/;
	++$file_count;
    }
    if($file_count > 0) {
	print ".";
	tarry 10;
	goto restart;
    }
    tarry 5;
    #opendir(DIR, "/data$disk/$Folder/Run_$run_number/binary");	
    #while(my $file = readdir(DIR)) {
    #	if($file =~ /(\d+)_(\d+)/) {
    #	    $file_count++;
    #	}
    #}
    #closedir(DIR);


    #while($file_count !=0){
    #	print ".";
    #	tarry 10;
    #	goto restart;
    #}

}

#############################################################################################################

sub is_readout_alive {

#    loadconfig("mysql");   # at this point $totusbs->contains number of usb from mysql
    # $usb_box is the trigger box USB

#print "Number of USB found in MySQL connected to PMT board= $totusbs, the trigger box is on USB $usb_box and we have $totalbox trigger box\n";
    my $expected_proc_number;

    if(!defined($number_expected_usb)) {
	$number_expected_usb=0;
	open IN, "lsusb |" or die $!;
	while(<IN>) {
	    if($_ =~ /:dead/) { $number_expected_usb++; }
	}
	close IN;
    
	$expected_proc_number = $number_expected_usb;
    }
    else {
	$expected_proc_number = $number_expected_usb;
    }

# now one process is the monitor
# then 1 process for USB - to PMT boards x 4
# one process for Trigger Box x 1 
    my %stream = ();
    my %cpu = ();
    my %mem = ();
    my $pid;
    
    my $key;
    my $cpu_diff = 0;
    my $cpumax = 0;
    my $cpumin = 1000000;

    my $mem_diff = 0;
    my $memmax = 0;
    my $memmin = 1000000;

    open IN, "ps aux \| grep \"./readout \" \| grep -v emacs \| grep -v grep |" or die $!;
    while(<IN>) {
	chomp;
#    print "$_ \n";
	my @temp = split(/\s+/,$_);
	$pid = $temp[1];
	$stream{$pid}++;
	$cpu{$pid} = $temp[2];
	$mem{$pid} = $temp[4];
#    print "$pid = $temp[1], cpu = $temp[2], mem = $temp[4]\n"; 
    }
    close IN or die $!;

    my $count_zero_cpu = 0.;
    my $number_of_processes = 0.;
    

#now find the min and max value for the hit
    foreach $key (sort (keys(%stream))){  # for each pid
#    print "Pid=$key, cpu=$cpu{$key} \n";
	if( $cpu{$key} < 0.5 ) {  # usually 0 or 0.2 
	    $count_zero_cpu++;
	}
	else {
	    $number_of_processes++;
	    if($cpu{$key}>$cpumax){ 
		$cpumax = $cpu{$key};
	    }
	    if($cpu{$key}<$cpumin) {
		$cpumin = $cpu{$key};
	    }
	}
    }

    $cpu_diff = $cpumax - $cpumin;
    
#print "Diff = $cpu_diff \n";
    

# now for the memory

    my $count_zero_mem = 0.;

#now find the min and max value for the hit
    foreach $key (sort (keys(%stream))){  # for each pid
#    print "Pid=$key, mem=$mem{$key} \n";
	if( $mem{$key} < 16000 ) { #usually around 18000
	    $count_zero_mem++;
	}
	else {
	    if($mem{$key}>$memmax){ 
		$memmax = $mem{$key};
	    }
	    if($mem{$key}<$memmin) {
		$memmin = $mem{$key};
	    }
	}
    }

    $mem_diff = $memmax - $memmin;

    if($count_zero_cpu > 1) { 
	print "Number of Readout Processes with cpu < 0.1 =$count_zero_cpu\n";
	gaibu_msg($MERROR,"Got a problem with the OV ROP - found more than 1 ROP with zero cpu usage"); 
	return 1;
    }

    if($number_of_processes != $expected_proc_number) { 
	print "Number of Processes Running = $number_of_processes -- expected $expected_proc_number \n";
	gaibu_msg($MERROR,"Got a problem with the number of OV ROP - expected = $expected_proc_number, found = $number_of_processes"); 
	return 2;
    }

    if($cpu_diff > 3.5) { 
	print "Difference in the cpu for the readout processes out of limit,$cpu_diff \n";
	gaibu_msg($MERROR,"Got a problem with the OV ROP - in term of cpu usage, difference is $cpu_diff"); 
	return 1;
    }

    if($mem_diff > 500) {
	print "Difference in the memory allocated for the readout processes out of limit,$mem_diff \n";
	gaibu_msg($MERROR,"Got a problem with the OV ROP - in term of memory usage, difference is $mem_diff"); 
	return 1;
    }

    return 0;

}
##################################################################################################

sub check_file_size {

my $filesize;
my %readfilesize;
my $usbread;

my $i=0;
my $diff;
my $disk;
my $run;
my $lasttimestamp;
my $timestamp_i = 0;
my $timestamp_i_old = 0;
my ($sth,$drh,$Data_Path,$readfile);

#get info from mysql

eval {
  $dsn = "dbi:mysql:database=$database;host=$DBhostname";
  $dbh = DBI->connect($dsn, $user, $password);
  $drh = DBI->install_driver("mysql");  
  $sth = $dbh->prepare("SELECT Run_number,daq_disk,EBcomment,EBretval,Run_Type FROM OV_runsummary WHERE Run_number=(select max(Run_number) from OV_runsummary)"); 
  $sth->execute();
  while (my $ref = $sth->fetchrow_arrayref()) {
      $run = $ref->[0];
      $disk = $ref->[1];
      if(!defined($ref->[2])) { goto end;};
      $lasttimestamp = substr($ref->[2],0,8);
      if($ref->[3] != 0) {
	  goto end;  #EB not running nothing to monitor
      }
      $run_type = $ref->[4];
  }
  $sth->finish();  
  $dbh->disconnect();
};

$Data_Path = "/data$disk/OVDAQ/DATA/Run_$run/decoded/";

tarry 5;

$readfile = 0;

if (-e "/data$disk/OVDAQ/DATA/Run_$run/decoded" ) {
#    print "Found folder : $Data_Path \n";
    open IN, "cd $Data_Path ; ls -ltr --time-style=long-iso ${lasttimestamp}*_*|";
    while(<IN>) {
        if(/ (\S+) \d+-\d+-\d+ \d+:\d+ (\d+)_(\d+)/) { 
	    $readfile = 1;
	    $filesize=$1;
	#    print "${2}_${3} - ${1} \n";
	    $readfilesize{$3} += $filesize;
	    $timestamp_i = $2;
	    if($timestamp_i != $timestamp_i_old) {
		$i++;
		$timestamp_i_old = $timestamp_i;
	    }	    
	  }
    }
    close IN;
}
else {
    print "Unable to find the Path: $Data_Path \n";
    goto end;
}

if($readfile == 0) { goto end; } # this help with latency of NFS or disk

my $maxdim = 0;
my $mindim = 1000000000;
my $triggerbox_file_dimension = 0;

if( keys(%readfilesize) != keys(%usbhowmanyboards)+1 ) { print "Problem in the number of USB file found (".keys(%readfilesize).") in path $Data_Path expected ".keys(%usbhowmanyboards)." \n"; }

foreach $usbread (keys(%readfilesize)){
    if($usbread!=$usb_box) {
#	$readfilesize{$usbread}/=$usbhowmanyboards{$usbread}; #this normalize the file dimension by the pmt boards
	if($readfilesize{$usbread} > $maxdim) { $maxdim = $readfilesize{$usbread}; }
	if($readfilesize{$usbread} < $mindim) { $mindim = $readfilesize{$usbread}; }
    }
    else {
	$triggerbox_file_dimension = $readfilesize{$usbread}/$i;
    }
}

if($i) {
    $diff = ($maxdim - $mindim)/$i;
}
else {
    print "Zero file to divide to\n";
}

#print "triggerbox file dimension = $triggerbox_file_dimension \n";

if($maxdim == 0 or $mindim == 0 or $triggerbox_file_dimension == 0) {
    print "Got a problem file dimension is ZERO!\n"; 
    gaibu_msg($MERROR,"Got a problem Number of packet is ZERO!"); 
    return 0;
}

if($diff > 350000) { 
	print "We got a problem with the dimension of DAQ files, difference between file dimension is $diff \n"; 
	gaibu_msg($MERROR,"We got a problem with the dimension of DAQ files, difference between file dimension is $diff"); 
	return 0;
}

if($triggerbox_file_dimension < 3000) { 
	print "We got a problem with the dimension of TB file, dimension is $triggerbox_file_dimension \n"; 
	gaibu_msg($MERROR,"We got a problem with the dimension of TB file, dimension is $triggerbox_file_dimension"); 
	return 0;
}

end:
return 1;

}

##################################################################################################################

sub checkROPLog {

my $TotErrThresh = 20; # WARNING                                                                                                  
my $ErrPerUSBPollThresh = 5; # CRITICAL ERROR assumes polling every 5 sec                                                                     

my $Run = shift;
my @files = <$online_path/DCOV/log/usb_*_out.log>;
my (%LastErrCount,%ErrCount);

foreach my $file (@files) {
    if($file =~ /usb_(\d+)_out.log/) {
	if(!defined($LastErrCount{$1})) {
	    $LastErrCount{$1} = 0;
	}
	$ErrCount{$1} = 0;
	my ($FoundRun,$FoundDataFolder,$Found2) = (0,0,0);
	open IN, $file or die $!;
	while(<IN>) {
	    if($_ =~ /pmt_full: (\d+)/ and $Found2 == 1) { $ErrCount{$1}++; }
	    elsif($_ =~ /no_lock: (\d+)/ and $Found2 == 1) { $ErrCount{$1}++; }
	    elsif($_ =~ /no_clk: (\d+)/ and $Found2 == 1) { $ErrCount{$1}++; }
	    elsif($_ =~ /usb_full: (\d+)/ and $Found2 == 1) { $ErrCount{$1}++; }
	    if($_ =~ /Inhibit: -2/) {
		last if ($Found2 == 1);
		if($FoundDataFolder == 1) { $Found2 = 1; }
	    }
	    if($_ =~ /Data Folder.*OVDAQ\/DATA/ and $FoundRun == 1) { $FoundDataFolder = 1; }
	    else { $FoundDataFolder = 0; }
	    if($_ =~ /$Run/) { $FoundRun = 1; }
	    else { $FoundRun = 0; }
	}
        close IN or die $!
    }
}

#foreach(keys %ErrCount) { print "USB: $_ ErrorCnt: $ErrCount{$_}\n"; }

my $ReturnErr = 0;
my $TotErr = 0;
foreach(keys %ErrCount) {
    my $Diff = $ErrCount{$_} - $LastErrCount{$_};
    if($Diff > $ErrPerUSBPollThresh) {
	#print "Error! Found $Diff new errors from USB $_\n";
	$ReturnErr = 1;
    }
    $TotErr += $Diff;
    $LastErrCount{$_} = $ErrCount{$_};
}
if($TotErr > $TotErrThresh) {
    print "Warning! Found $TotErr new errors from OVDAQ\n";
    gaibu_msg($MERROR,"ERROR! Found $TotErr new errors from OVDAQ\n");
    sendmail("OVRC","CRITICAL","Found $TotErr new errors from OVDAQ");
}

return $ReturnErr;

}
#############################################################################

sub checkLVstatus {

my $config = "LowVolt";
my $nUnit = 2;
my $nChannels = 4;
my $restart_required = 0;
if(!defined($user_online)) {$user_online = "dconline"; }

#check if the HVserver and HVMonitor are running
my ($hostname_full,$LV_monitor_running);

$hostname_full = sprintf("%s.in2p3.fr",$hostnamepc);

if(${OVSERV_HOST} eq ${hostname_full})  {

    $LV_monitor_running = `pidof ./DCLVMonitorServer`;

    if($LV_monitor_running eq "" ) {
	#gaibu_msg($MWARNING,"OVLV Monitor process is not running");
	syslog('LOG_WARNING',"OVLV Monitor process is not running");
	#sendmail("OVRC", "WARNING","OVLV Monitor Process is not running");
#    print "Result: $HV_monitor_running \n";
	print "Something wrong with the LV Monitor process \n";
	$restart_required = 1;
	goto end;
    }
}

#I need the db connection.                  

my $LVdatabase="dc_env_fd";
my $LVuser="ENVuser";
my $LVpassword="thetaENV13";
my $DBhostname="dcfmysql.in2p3.fr";

my $dsn = "dbi:mysql:database=$LVdatabase;host=$DBhostname";

# now connect and get a database handle                                                             

my $dbh = DBI->connect($dsn, $LVuser, $LVpassword)
    or die "Can’t connect to the DB: $DBI::errstr\n";

my $timemonitormin = 2;  # set the maximum interval for the HV monitor in min 

my $tableName;
my $tableNameI;
my $tableNameSet;
my $myquery;
my $sqlQuery;
my @row;
my @isConn;
my @VSet;
my @VSetSig;
my @ISet;
my @ISetSig;
my @VMon;
my @IMon;

my $nChan=0;
my $nChanOn=0;
my $nChanOff=0;
my $nChanGood=0;
my $nChanBad=0;

for(my $unit=0; $unit < $nUnit; $unit++){
    $restart_required = 0;    
    $tableName = sprintf("history_%s_VMon_Unit%d",$config, $unit);
    $tableNameI = sprintf("history_%s_IMon_Unit%d",$config, $unit);
    $tableNameSet = sprintf("history_%s_Set_Unit%d",$config, $unit);
    
    #Get the settings.         
    
    $myquery = sprintf("select * from %s order by time desc",$tableNameSet);
    $sqlQuery  = $dbh->prepare($myquery);
    $sqlQuery->execute();
    @row = $sqlQuery->fetchrow_array;
    
    my $nval =0;
    my $nchn =0;
    my $index=0;
    foreach my $value (@row){
	if($nval == 0){
            #print "Settings from $value ";                                                                 
	}
	else{
	    if($index==0){ $isConn[$nchn] = $value; }
	    if($index==1){ $VSet[$nchn] = $value; }
	    if($index==2){ $VSetSig[$nchn] = $value; }
	    if($index==3){ $ISet[$nchn] = $value; }
	    if($index==4) {
		$ISetSig[$nchn] = $value;
		$nchn++;
		$index=0;
	    }else{
		$index++;
	    }
	}
	$nval++;
    }
    #Get Some Values...
    $myquery = sprintf("select * from %s where time>=(select DATE_SUB(NOW(), INTERVAL %d MINUTE))",$tableName,$timemonitormin);
#    $myquery = sprintf("select * from %s where time>=\'%s\' and time<=\'%s\'",$tableName, $earlyTime, $currTime);
    $sqlQuery  = $dbh->prepare($myquery);
    $sqlQuery->execute();
    @row = $sqlQuery->fetchrow_array;
    
    #reset.                                                                                                   
    $nval=0;
    $nchn=0;
    
    #print "Query=$myquery\n";                                                                                
    #print "VMon: number of row read $#row \n";
    
    if($#row < 1){
	print "\n--->OVLV Monitor has been off for more than $timemonitormin mins \n\\n\n";
	gaibu_msg($MWARNING,"OVLV Monitor has been off for more than $timemonitormin mins");
	sendmail("OVRC","WARNING","OVLV Monitor has been off for more than $timemonitormin mins");
	#updaterunsummarymysql("LV","monitor off");                                                           
	$restart_required = 1;
	goto end;
    }
    
    #Here is the Voltage filled.                                                                              
    foreach my $value (@row){
	if($nval == 0){
	    #print "VMon from $value \n";                                                                     
	}else{
	    $VMon[$nchn] = $value;
	    $nchn++;
	}
	$nval++;
    }
    
    #Now the Current.      
    $myquery = sprintf("select * from %s where time>=(select DATE_SUB(NOW(), INTERVAL %d MINUTE))",$tableNameI,$timemonitormin);
    #$myquery = sprintf("select * from %s where time>=\'%s\' and time<=\'%s\'",$tableNameI, $earlyTime, $currTime);
    #print "$myquery";                                                                                        
    $sqlQuery  = $dbh->prepare($myquery);
    $sqlQuery->execute();
    @row = $sqlQuery->fetchrow_array;
    
#    print "IMon: number of row read $#row \n";
    
    #reset.                                                                                                   
    $nval=0;
    $nchn=0;
    
    if($#row < 1){
	print "\n--->OVLV Monitor has been off for more than $timemonitormin mins \n\n\n";
	gaibu_msg($MWARNING,"LV Monitor has been off for more than $timemonitormin mins");
	sendmail("OVRC","WARNING","LV Monitor has been off for more than $timemonitormin mins");
	#updaterunsummarymysql("LV","monitor off");
	$restart_required = 1;                                                           
	goto end;
    }
    
    #Here is the Current filled.                                                                              
    foreach my $value (@row){
	if($nval == 0){
	#    print "IMon from $value \n";
	}else{
	    $IMon[$nchn] = $value;
	    $nchn++;
	}
	$nval++;
    }
    
    #Now do some accounting.                                                                                  
    my $diff;
    my $diffI;
    for(my $i=0; $i<$nChannels; $i++){
	#print " $isConn[$i] $VMon[$i] $VSet[$i] ";
	if($isConn[$i]){
	    $nChanOn++;
	    $diff = $VMon[$i] - $VSet[$i];
	    $diff = sqrt($diff*$diff);
	    if($diff < $VSetSig[$i]){
		$diffI = $IMon[$i] - $ISet[$i];
		$diffI = sqrt($diffI*$diffI);
		if($diffI < $ISetSig[$i]){
		    $nChanGood++;
		}else{
		    $nChanBad++;
		}
	    }else{
		$nChanBad++;
	    }
	}else{
	    $nChanOff++;
	}
	
	$nChan++;
    }
}

print "OV LV Tot. Ch:${nChan},ON:${nChanOn},OFF:${nChanOff}\n";
print "Good:${nChanGood},Bad:${nChanBad} \n";

if( ($nChanOn) > 5 ){                                                                                        
#    updaterunsummarymysql("LV","ON");                                                                        
    #print "\nOVLV is ON and Working !\n";                                                                      
    if( ($nChanGood) < 5){                                                                                   
#        updaterunsummarymysql("LV","ONw");                                                                   
     #   print "\n LV is ON but warning something is wrong\n";                                                
        gabu_msg($MWARNING,"LV of OV is on but in warning stage");                                           
    }                                                                                                        
}                                                                                                            
else                                                                                                         
{                                                                                                            
    gaibu_msg($MNOTICE,"LV of OV is off");                                                                   
#    updaterunsummarymysql("LV","OFF");                                                                       
}                                                                                                            

 end:
    #print "\n Bye Bye! \n";
    if($restart_required == 1) {
	#try to restart the LV Server
	#gaibu_msg($MWARNING,"OVLV process is restarting ...");
	syslog('LOG_WARNING',"OVLV process is restarting ...");
	#sendmail("OVRC", "WARNING","OVLV process is restarting ...");
        #stop the LVServer
	#system qq|ssh ${user_online}\@${OVSERV_HOST} "${online_path}\/DCOV\/readout\/startupscript\/stop_LV.sh LVmonitor"|;
	system qq|ssh ${user_online}\@${OVSERV_HOST} "\/local\/home\/dconline\/myOnline\/DCOV\/readout\/startupscript\/stop_LV.sh LVmonitor"|;
        #restart LVserver
	#system qq|ssh ${user_online}\@${OVSERV_HOST} "${online_path}\/DCOV\/readout\/startupscript\/check_LV.sh ${online_path}"|;
	system qq|ssh ${user_online}\@${OVSERV_HOST} "\/local\/home\/dconline\/myOnline\/DCOV\/readout\/startupscript\/check_LV.sh ${online_path}"|;
     }
}
#############################################################################

sub check_last_disk_used {

    my $daq_disk_past = 0;

    eval {
	$dsn = "dbi:mysql:database=$database;host=$DBhostname";
	$dbh = DBI->connect($dsn, $user, $password);
	$drh = DBI->install_driver("mysql");
    $sth = $dbh->prepare("SELECT daq_disk FROM OV_runsummary where Run_number =(select max(Run_number) from OV_runsummary)");
	$sth->execute();
	while (my $ref = $sth->fetchrow_arrayref()) {
	    $daq_disk_past = $ref->[0];
	}
	$sth->finish();
	$dbh->disconnect();

	#print "Last Run Number disk used by DAQ was $daq_disk_past \n";

};

    $daq_disk_past = 1 if $@;

	return $daq_disk_past;
}

############################################################################

sub duplicate_run_number {

    my $duplicaterunnumber = 0;

eval {
    $dsn = "dbi:mysql:database=$database;host=$DBhostname";
    $dbh = DBI->connect($dsn, $user, $password);
    $drh = DBI->install_driver("mysql");
    $sth = $dbh->prepare("SELECT Run_number FROM OV_runsummary where Run_number =${run_number}");
    $sth->execute();
    while (my $ref = $sth->fetchrow_arrayref()) {
	if($ref->[0]) { 
	    $duplicaterunnumber = 1; 
	}
    }
    $sth->finish();
    $dbh->disconnect();
    
};

    return $duplicaterunnumber;

}

#####################################################################
#LG2018
#
#####################################################################
######################################################################################################
sub setpmtdata {

my $usbboard = shift;
my $pmtnumber = shift;

#printf "Getting info for PMT board $pmtnumber \n";

pmtserialinput:
print "Enter PMT Serial Number (for example: 4662); ";
chomp($pmtserialnumber = <STDIN>);                #read line from stdin and remove newline
if($pmtserialnumber eq "") {goto pmtserialinput};

$boardnumber = $pmtnumber;

if($pmtserialnumber eq "none") {
    # $boardnumber = $pmt;
    $HVsetting = "none";
    $usemaroc2gainconstantsmb = "no";
}
else{
    # boardnumberinput;
    # print "Enter Board Number for $pmtserialnumber: ";
    # chomp($boardnumber = <STDIN>);
    # if($boardnumber eq "") {got boardnumberinput);

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
chomp($gate = <STDIN>);           #read line from stdin and remove newline
if($gate eq "") {goto gate};

$trigger_mode = 1;

#trigger
print "Which trigger (op75)? (0,1,2) ";
chomp($trigger_mode = <STDIN>);       #read line from stdin and remove rewline
if($trigger_mode eq "") {goto trigger};

hdelay:
print "Which hdelay? (push enter will set the default value of 5) ";
chomp($hdelay = <STDIN>);      #read line from stdin and remove newline
if($hdelay eq "") {$hdelay = 5};
print "$hdelay\n";

if($choice ne "manual") {
print "Any comments \n";
chomp($comments = <STDIN>);       #read line from stdin and remove newline
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

###################################################################################################
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
	    $dsn = "dbi:mysql:database=$database;host=$DBhostname"; 
	    $dbh = DBI->connect($dsn, $user, $password); 

	    print "Failed to connect to database: $@\n" if $@;
	    if($@ eq "") {print "Connected to MySQL database\n\n"; }

	    $drh = DBI->install_driver("mysql");

	    $sth = $dbh->prepare("SELECT USB_serial, PMT_Serial, board_number, HV, DAC_threshold,                                      
         use_maroc2gain, gate, pipedelay, trigger_mode, gain1, gain2, gain3, gain4,                                                   
        gain5, gain6, gain7, gain8, gain9, gain10,                                                                                 
        gain11, gain12, gain13, gain14, gain15, gain16 ,                                                                           
        gain17, gain18, gain19, gain20, gain21, gain22 ,                                                                           
        gain23, gain24, gain25, gain26, gain27, gain28 ,                                                                           
        gain29, gain30, gain31, gain32, gain33, gain34 ,                                                                           
        gain35, gain36, gain37, gain38, gain39, gain40 ,                                                                           
        gain41, gain42, gain43, gain44, gain45, gain46,                                                                            
        gain47, gain48, gain49, gain50, gain51, gain52,                                                                            
        gain53, gain54, gain55, gain56, gain57, gain58,                                                                            
        gain59, gain60, gain61, gain62, gain63, gain64 FROM online400");

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
		    #if($ref->[0] == $usb and $ref->[1] eq $pmtserialnumber and $ref->[2] == $pmt){#check for matching pmt_serial number and board_address
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
    my $pmt1 = shift;    # 63 = all
    #my $pmt1;

    our $filebase = "baselines.bin";         # name of file to generate

    my $fullpath = "$full_path/USB_${usb}";
    dir $fullpath;  

    new $usb , "$dir/$filebase";        # open new file

#insert this into a loop from 1..4
    usb_usb $usb, 0,1;#auto token on
    tarry 0.5;

    #my $pmt = 3;
    #push(@pmt_board_array, $pmt); 

    foreach $pmt1 (@pmt_board_array) {
        
#print "Starting  USB = $usb,  PMT = $pmt,  Run = $run\n";
	com_usb $usb, $pmt1, 110, 1;                # turn led on/off
       
	com_usb $usb, $pmt1, 109, 1;                # vdd_fsb on
	com_usb $usb, $pmt1, 73, 0b00000;           # set up pmt module
#print "Configuring...\n";
	com_usb $usb, $pmt1, 74, 0b0100000;         # default gain
	com_usb $usb, $pmt1, 70, 0;                 # load default
	dac $usb, $pmt1, 1000;                  # dac threshold
	com_usb $usb, $pmt1, 71, 0;                 # rst_g to maroc
	com_usb $usb, $pmt1, 72, 0;                 # write config
	com_usb $usb, $pmt1, 72, 0;                 # write config
	com_usb $usb, $pmt1, 73, 0b00110;           # set up pmt module
	com_usb $usb, $pmt1, 75, 0b00010000;        #set up trigger mode
	com_usb $usb, $pmt1, 254, 0;                # enable triggers

	tarry 1;

#print "Taking baseline data for pmtboard $pmt1 ...\n";
	for(1..10) {#avoid first packets
	    com_usb $usb, $pmt1, 81, 0;         # test trigger
	}

	tarry 1;

#new $usb, "$dir/$filebase";        # open new file

	for(1..500) {
	    com_usb $usb, $pmt1, 81, 0;         # test trigger
	}

	print "Finishing up with pmt board $pmt1...\n";
	com_usb $usb, $pmt1, 255, 0;
	com_usb $usb, $pmt1, 73, 0b00000;       # set up pmt module
	tarry 1;                          # wait for late packets


###finish loop
    }

    shut $usb;                          # close file

    my $dir2 =  "../DATA/Run_${run}/USB_${usb}"; 
    my $full_path2 = "../DATA/Run_${run}/USB_${usb}";
    print "$dir2\n";             
    tarry 2;


    print "Baseline data taken completed.\n\n";

    foreach $pmt1 (@pmt_board_array) {
#add another loop for pmtboard from 1..4
	our $dir1 = "../DATA/Run_${run}/USB_${usb}/PMT_${pmt1}";  # change this to create your own directory structure
	dir $dir1;     

	my $dir2 = "../DATA/Run_${run}/USB_${usb}"; 
	my $full_path2 = "../DATA/Run_${run}/USB_${usb}";
                 
	system qq| /usr/bin/perl  baselines.pl "$dir2" "$filebase" "$pmt1"|;
	print "Baseline decoded for pmt $pmt1...\n\n";
	initializeusb($usb,$pmt1);
    }
}
######################################################################################################################################
sub initializeusb {

    my $usb = shift;
    my $pmt = shift;

#baseline($usb,$pmt);  # taking baseline data

    getpmtdata $usb, $pmt;

#our $dir1 = "/home/camillo/readout/DATA/Run_${run}/USB_${usb}/PMT_${pmt}";  # change this to create your own directory structure
#dir $dir1;     

    my $dir2 = "../DATA/Run_${run}/USB_${usb}"; 
    my $full_path2 = "../DATA/Run_${run}/USB_${usb}";
                 

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
   # init_readout "127.0.0.1";           # or "localhost"
#send_out "Greetings\n";             # write message to get current dir

#print "Initializing  USB = $usb,  PMT = $pmt on run $run\n";
    com_usb $usb, $pmt, 110, 1;#turn off the three led on the PMT's board
    com_usb $usb, $pmt, 109, 1;             # vdd_fsb on
    com_usb $usb, $pmt, 73, 0b00000;        # set up pmt module
#com $usb, $pmt, 255, 0;             # clear buffers
#com $usb, $pmt, 84, 255;            # buffer size limit  
#tarry 2.5;                          # seconds
    com_usb $usb, $pmt, 74, 0b0100000;      # default gain
    com_usb $usb, $pmt, 70, 0;              # load default
    dac $usb, $pmt, $DACt;              # threshold value
    com_usb $usb, $pmt, 67, 0b000010;       # statea
    com_usb $usb, $pmt, 68, 0b000000;       # stateb  40f 25k 50k 100k 50f 100f
    com_usb $usb, $pmt, 69, 0b000000;       # statec
    com_usb $usb, $pmt, 71, 0;              # rst_g to maroc 
    com_usb $usb, $pmt, 72, 0;              # write config
    com_usb $usb, $pmt, 72, 0;              # write config
    com_usb $usb, $pmt, 73, $gateonoff;     # $gateonoff;     # gate
    com_usb $usb, $pmt, 75, $trigger_mode;  # trigger mode $trigger_mode see document for details
    com_usb $usb, $pmt, 80, 5;              # hdelay
    com_usb $usb, $pmt, 85, 0b0000;         # set up pipe delay
    com_usb $usb, $pmt, 86, 1;              # edge strip mode
    com_usb $usb, $pmt, 87, 0b10;           # force trigger -> 01: 1msec, 10: 16ms, 11:256ms

    if($usemaroc2gainconstantsmb eq "no") {
        com_usb $usb, $pmt, 74, 0b0100000;      # default gain 
    }
    elsif($usemaroc2gainconstantsmb eq "yes" && $mysqlload == 0 && $equalization eq "false"){ 

	$gaindirectory = "../Scripts_mb/marocgainsetting/gain_PA${pmtserialnumber}_${boardnumber}_${HVsetting}.txt";
	$savedgaindirectory = "../Scripts_mb/marocsavedgainsetting/gain_PA${pmtserialnumber}_${boardnumber}_${HVsetting}_${date}.txt";

	if(-e $gaindirectory) {
	    open IN, $gaindirectory or die $!;
#print "Local file with the Maroc2 gain constants found, now reading and applying settings...\n\n";
	    open OUT, ">$full_path/PMT_${pmt}/gain.txt" or die $!;
	    while(<IN>) {
		chomp; s/#.*//;    # delete comments
		    next if /^\s*$/;    # skip blank lines
		my($channel, $value) = split /\s+/;
		my $happy = 2 * $value;
		com_usb $usb, $pmt, $channel - 1, $happy;
		print OUT "$channel \t $value \n";
		$gain[$channel] = $value;
	    }
	    close IN or die $!;
	    close OUT or die $!;
	} else {
	    print "Local file with the Maroc2 gain constants not found, use default=16 for all channels\n";
	    print "$gaindirectory\n";
	    open OUT, ">$gaindirectory" or die $!;
	    open OUT1, ">$full_path2/PMT_${pmt}/gain.txt" or die $!;
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
	open OUT, ">$full_path2/PMT_${pmt}/gain.txt" or die $!;
	for(my $index=1;$index<65;$index++){
	    com_usb $usb, $pmt, $index - 1, 2*$gain[$index];
	    #print "MySQL gain constants found: $index, $gain[$index+1] \n";
	    print OUT "$index \t $gain[$index] \n";       
	}
	print "MySQL gain constants applied. \n";
	close OUT or die $!;
    }
    elsif($equalization eq "true"){ 

	$gaindirectory = "../Scripts_mb/marocgainsetting/gain_PA${pmtserialnumber}_${boardnumber}_${HVsetting}.txt";
	$savedgaindirectory = "../Scripts_mb/marocsavedgainsetting/gain_PA${pmtserialnumber}_${boardnumber}_${HVsetting}_${date}.txt";

	if(-e $gaindirectory) {
	    open IN, $gaindirectory or die $!;
#print "Local file with the Maroc2 gain constants found, now reading and applying settings...\n\n";
	    open OUT, ">$full_path2/PMT_${pmt}/gain.txt" or die $!;
	    while(<IN>) {
		chomp; s/#.*//;    # delete comments
		    next if /^\s*$/;    # skip blank lines
		my($channel, $value) = split /\s+/;
		my $happy = 2 * $value;
		com_usb $usb, $pmt, $channel - 1, $happy;
		print OUT "$channel \t $value \n";
		$gain[$channel] = $value;
	    }
	    close IN or die $!;
	    close OUT or die $!;
	} else {
	    print "Local file with the Maroc2 gain constants not found, use default=16 for all channels\n";
	    print "$gaindirectory\n";
	    open OUT, ">$gaindirectory" or die $!;
	    open OUT1, ">$full_path2/PMT_${pmt}/gain.txt" or die $!;
	    for(1..64) {
		print OUT "$_\t16\n"; 
		print OUT1 "$_\t16\n";
	    }
	    close OUT or die $!;
	    close OUT1 or die $!;
	}
	#system qq|cmd /C copy "$gaindirectory" "$full_path/PMT_${pmt}/gain.txt" >> logfile_${pmtserialnumber}_${date}.txt|;
    }
 
    com_usb $usb, $pmt, 109, 0;             # vdd_fsb off
    com_usb $usb, $pmt, 255, 0;                            # disable trigger

#print "Creating summary file for PMT $pmt...append info if file already exists\n";

    my $summaryfile = "$full_path2/summary.txt";
    my $summaryfile1 = "$full_path2/summary_new.txt";


    if($usemaroc2gainconstantsmb eq "yes"){
	open IN1,"$full_path2/PMT_${pmt}/gain.txt" or die $!;
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
	system qq|mv "$summaryfile1" "$summaryfile"|;
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
################################################################################################################################

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

my @files = <../DATA/Run_${date}_*>;

foreach $fileb (@files) {
  if($fileb =~ /Run_(\d+)_(\d+)/) {
      $max = $2 if $2 > $max;
  }
}  
$max = $max + 1;
            
$subrun = $max; 
our $run = "${date}_${subrun}";
our $full_path = "../DATA/Run_${run}";

$dir = "../DATA/Run_${run}/USB_${usb}";  # change this to create your own directory structure

our $filebase = "baselines.bin";         # name of file to generate
open IN, ">$filebase" or die $!;
close IN;

my $fullpath = "$full_path/USB_${usb}";
dir $fullpath;
#print "Opening file: $filebase\n";
new $usb, "$dir/$filebase";        # open new file


}
########################################################################################################################################

sub takedatamb {

my $usb = shift;
my $runlength = shift;

print "Preparing to take data ... be patient ...\n";
print "\n";
     
$filename = "$full_path/USB_${usb}/signal.bin";

if($filelength > $runlength){ $filelength = $runlength; }

$newrunlength = $runlength / $filelength;

if($usemultiplefile eq "no") { #not splitting the file in many file
	$newrunlength = 1;
	$filelength = $runlength; 
}
	
my $filenametemp;

for(my $i=0;$i<$newrunlength;$i++){
	print "Starting file $i...\n";
	$filenametemp = "$full_path/USB_${usb}"."/signal_$i.bin.part";
        #new $usb, $filenametemp;                    # open new file
        open IN,">$filenametemp" or die $!;
        close IN;
	if($i==0){
		com_usb $usb, 63, 254, 0; 
	}	
	tarry $filelength;
	tarry 0.75;                            		# wait for late packets
	shut $usb;                          		# close file
	tarry 0.75;
      
     	system qq|mv "${filenametemp}" "${full_path}/USB_${usb}/signal_${i}.bin"|;
}
#just at the end
com_usb $usb, 63, 73, 0b00000;		                # gate
tarry 0.5;
com_usb $usb, 0, 255, 0;                            # disable trigger
tarry 0.5;                		


# print "Data taken completed...\n";
# print "\n";
}
#####################################################################################

sub generatecsv2 {

$usb = shift;
$pmt = shift;

getpmtdata $usb, $pmt;

# print "Processing data for USB=$usb and PMT=$pmt\n";
# print "\n";
$full_path = "../DATA/Run_${run}";

my $full_path_temp = "$full_path/USB_${usb}/PMT_${pmt}";

dir $full_path_temp;                              # set usb to (create and) write to this folder

$baselines = "$full_path/USB_${usb}/PMT_${pmt}/baselines.dat";
$filename = "$full_path/USB_${usb}/signal";

open IN, ">$baselines" or die $!;
close IN;

open IN, ">$filename" or die $!;
close IN;

for(my $i=0;$i<$newrunlength;$i++){
    system qq| /usr/bin/perl decode.pl "${filename}_${i}.bin" >> "${filename}.dec"|;
}
my $temp = "$filename.dec";
my $temp1 = "$full_path/USB_${usb}/PMT_${pmt}/signal.dec";
open IN, ">$temp1" or die $!;
close IN;
system qq|mv "$temp" "$temp1"|;

DCfunctions::signal (${full_path},$temp1,$baselines,$pmt);
#system qq|cmd /C signal.pl "${full_path_temp}" "$temp1" "$baselines" "@gain" "$pmt" ${homedir} >> log_${pmtserialnumber}_${date}.txt|;

}
###############################################################################################################

sub plotdatamb2 {

$usb = shift;
$pmtmb = shift;
my $pmtserialnumber = shift;

getpmtdata $usb, $pmtmb;

print "Plotting data for USB=$usb and PMT=$pmtmb\n";

# make histograms
system qq|/usr/bin/perl histogram.pl "$full_path\\USB_${usb}\\PMT_${pmtmb}" "summary.csv" 1 "Pulse heights" >> log_${pmtserialnumber}_${date}.txt|;
system qq|/usr/bin/perl histogram.pl "$full_path\\USB_${usb}\\PMT_${pmtmb}" "summary.csv" 2 "Sigma" >> log_${pmtserialnumber}_${date}.txt|;
system qq|/usr/bin/perl histogram.pl "$full_path\\USB_${usb}\\PMT_${pmtmb}" "summary.csv" 3 "Hits by channel" >> log_${pmtserialnumber}_${date}.txt|;
system qq|/usr/bin/perl histogram.pl "$full_path\\USB_${usb}\\PMT_${pmtmb}" "summary.csv" 5 "Photoelectrons" >> log_${pmtserialnumber}_${date}.txt|;
system qq|/usr/bin/perl histogram.pl "$full_path\\USB_${usb}\\PMT_${pmtmb}" "summary.csv" 6 "Gains" >> log_${pmtserialnumber}_${date}.txt |;
system qq|/usr/bin/perl histogram.pl "$full_path\\USB_${usb}\\PMT_${pmtmb}" "summary.csv" 7 "Rate" >> log_${pmtserialnumber}_${date}.txt|;
system qq|/usr/bin/perl histogram.pl "$full_path\\USB_${usb}\\PMT_${pmtmb}" "summary.csv" 8 "Gain Constants" >> log_${pmtserialnumber}_${date}.txt|;
system qq|/usr/bin/perl histogram.pl "$full_path\\USB_${usb}\\PMT_${pmtmb}" "summary.csv" 9 "Normalized Hits by channel" >> log_${pmtserialnumber}_${date}.txt|;
}














1;
