#!/usr/bin/perl

use strict;
use warnings;
use lib $ENV{DCONLINE_PATH} . "/DCOV/readout/macro";
use DCFOV;
use usb_readout;
use English;
use IO::Socket;
use IO::Select;
use Time::HiRes;

my $processName = "startOVDAQ";

my $user_online = "dconline";                           # online user

$OUTPUT_AUTOFLUSH = 1;                               # writing messages immediately to STDOUT

my $RUN_definition = "DCPHYS_RUN_D1";                # this parameter is going to be passed by DCRC
my $maxdiskusage = 97;                               # set the max disk occupancy 
my $max_client_number = 1;                           # set the max number of client that will connect

my $restart_time = 0;

my $usball = -1;                                     # -1 = all
#my @usb = (24,25,23,29,21);                          # DCNEAR
#my @usb = (15,16,17,18,19);                          # DCFAR

my $debug = '';                                      # yes for debug mode, oldmode for input manually the pmtserialnumber
my ($k,$current_usb);
my $tarrytimeRC = 0.2;                               # this sets the delay time to answer or get new info from DCRC
my $runnumbermode;
my $numbaselinetriggers = 50;

#offline variables
$HWthreshold = 0;                                    # threshold in ADC applied in the hardware
		                                     # this will be read from the calibration mysql table
$threshold = 120; #120                                      # threshold in ADC to be applied offline
$trigger = 1; #1                                       # Offline trigger requirements 0: None, 1:SingleLayer mode; 2: DoubleLayer mode
$usedogsifier = 1;                                   # use or not the DOGSFIER with event builder
$eb_flag = 1;                                        # Run the event builder

#for computing runlength
my ($oldsec,$oldmin,$oldhour,$oldday,$oldmonth,$oldyr19,@oldrest);
my ($currsec,$currmin,$currhour,$currday,$currmonth,$curryr19,@currrest);
my $elapsedtime;
my $timecount = 0;

#protocol for DCRC
my $HANDSHAKING = "GOT : ";
my $INCANTATION_MS = "STATE ";
my $INCANTATION_S = "STATE ";
my $RUNCONFIGURE_IC = "RunConfigure ";
my $TERMINATION = "\n";                               # \n\0;

#initial state
my $mystate="BEFOREBIRTH_IS";                         # "BEFORE_BIRTH";
my $flags = 0;

#server socket definition
my ($client_ip_number,$port,$client_ip,$new_sock,$addr,$ip);
my ($Read_Handles_Object,$client,$client_id,$client_conn,$client_number,@ipnum);

#Initialize process log: AJF, 3/21/11
my $logPath = $ENV{DCONLINE_PATH} . "/DCOV/log/";
my $logFile = $logPath . $processName . ".log";
open STDOUT, ">$logFile";
open STDERR, ">&STDOUT"; #print ERR and OUT to same place

#initialization parameters
my $DAQ_GAIBU_OFF = 1;                                # suppress GAIBU message for DAQ (no message = 1)
$run_length = 60*60*24*7;                             # run length in sec for autorunlenght
$DebugDAC = 900;                                      # 700 if PMT HV=off
$run_number = 0;                                      # if 0 run number is set automatically by the DAQ
$DataPath = "OVDAQ/DATA/";                            # Disk currently in use
$disk = 2;                                            #2 Disks available (1,2)
$eb_disk = 1;

#reading the IP configuration files
my $ip_config_file = "DCSpaceIP.config";      load_ip_config($ip_config_file);
my $port_config_file = "DCSpacePort.config";  load_port_config($port_config_file);
my $db_config_file = "DCDatabase.config";     load_db_config($db_config_file);


print "Opening OV RC socket on $OVRC_IP:$OVRC_Port \n";

# Prepare run control server socket
    my $sock = new IO::Socket::INET ( 
	LocalHost => $OVRC_IP,                            # This server's IP address
	LocalPort => $OVRC_Port,                          # This server's open port
	Proto => 'tcp', 
	Listen => 1,                                      # define number of connection should be = 1 in the final version
	Reuse => 1, 
	); 
    die "Could not create socket: $!n" unless $sock;

gaibu_msg($MNOTICE, "OV RC Started");

$Read_Handles_Object = new IO::Select();              # create handle set for reading 
$Read_Handles_Object->add($sock);                     # add the main socket to the set 

$client_number = 0;

while(1) {                                            # forever loop

# get a set of readable handles (blocks until at least one handle is ready)
client_disc:

    my ($client_conn) = IO::Select->select($Read_Handles_Object, undef, undef, 0);

# take all readable handles in turn
    foreach $client_id (@$client_conn) { 

	#print "\n Found readable socket ... \n";
	#if it is the main socket then we have an incoming connection and 
	#we should accept() it and then add the new socket to the $Read_Handles_Object 
	if ($client_id == $sock) { 
	    $client_number++;
	    #you get this when a new socket connection is formed..a new client starts and it assign a socket filehandle to it.
	    if($client_number <= $max_client_number) {
		($new_sock,$addr) = $client_id->accept(); 
	        ($port,$ip) = sockaddr_in $addr;
		$ipnum[$client_number] = inet_ntoa $ip;
		$Read_Handles_Object->add($new_sock);
		print "\n New Client connected from IP: $ipnum[$client_number] \n";
	    }
	    else {
		$Read_Handles_Object->remove($client_id); 
		my($new_sock_1,$addr_1) = $client_id->accept();
	        my($port_1,$ip_1) = sockaddr_in $addr_1;
		$ipnum[$client_number] = inet_ntoa $ip_1;
		print "New Connection request received from $ipnum[$client_number] that would be connection number $client_number\n";
		gaibu_msg($MERROR,"Additional TCP Client request connection to RC from $ipnum[$client_number]. ERROR");
		print $new_sock_1 "Connection refused. Only $max_client_number are allowed to connect to the OVRC\n";
		tarry $tarrytimeRC; # sleep(1);
		print "\n Additonal Client from $ipnum[$client_number] disconnected \n";
		close($new_sock_1);
		$client_number--;
		delete(@$client_conn[$client_id]);
		next;
	    }

#check who the client is ... if it is not dcfovdaq or dcfovmon then run locally otherwise check if NFS is mounted and decide
	    
	    where_to_run($ipnum[$client_number]);
	    
#start Process Monitor and Log monitor on monitor and daq machine
	    system qq|ssh ${user_online}\@${OVEVENTBUILDER_HOST} "${online_path}\/DCOV\/readout\/startupscript\/start_OV_log_monitor.sh ${online_path}"|;
	    system qq|ssh ${user_online}\@${OVEVENTBUILDER_HOST} "${online_path}\/DCOV\/readout\/startupscript\/start_OV_process_monitor.sh ${remote_path}"|;
	    system qq|ssh ${user_online}\@${OVDAQ} "${online_path}\/DCOV\/readout\/startupscript\/start_OV_log_monitor.sh ${online_path}"|;
	    system qq|ssh ${user_online}\@${OVDAQ} "${online_path}\/DCOV\/readout\/startupscript\/start_OV_process_monitor.sh ${remote_path}"|;
	    system qq|ssh ${user_online}\@${OVSERV_HOST} "${online_path}\/DCOV\/readout\/startupscript\/start_OV_log_monitor.sh ${online_path}"|;
	    system qq|ssh ${user_online}\@${OVSERV_HOST} "${online_path}\/DCOV\/readout\/startupscript\/start_OV_process_monitor.sh ${remote_path}"|;
	    system qq|ssh ${user_online}\@${OVSERV_HOST} "${online_path}\/DCOV\/readout\/startupscript\/start_OVHV_monitor.sh ${remote_path}"|;
	    system qq|ssh ${user_online}\@${OVSERV_HOST} "${online_path}\/DCOV\/readout\/startupscript\/start_OVLV_monitor.sh ${remote_path}"|;
		
#reading the IP configuration files
	    if($nfs_mount == 1) {
		print "NFS mount \n";
		load_ip_config($ip_config_file);
		load_port_config($port_config_file);
		load_db_config($db_config_file);
	    }

	    if($mystate eq "RUNAUTO_S") {
		($currsec,$currmin,$currhour,$currday,$currmonth,$curryr19,@currrest) =   localtime(time);
		$elapsedtime = ($currday-$oldday)*24*3600 + ($currhour-$oldhour)*3600 + ($currmin-$oldmin)*60 + ($currsec - $oldsec);
		#RC will be master.
		if($elapsedtime > $run_length) {
		    gaibu_msg($MNOTICE, "OV runlenght exceeded the expected runlength of $run_length");
		    stoptakedata;
		    print $new_sock "${INCANTATION_S}TRIGGEROFF_HS${TERMINATION}"; 
		    tarry $tarrytimeRC; # sleep(2);
		    $mystate = "STARTED_S";
		    gaibu_msg($MNOTICE,$mystate);
		    print $new_sock "${INCANTATION_S}${mystate}${TERMINATION}"; 
		    tarry $tarrytimeRC; # sleep(2);
		    $mystate = "STARTED_S";
		    rc_runend_comment($detector_configuration);  ##we need to get this from the database
		}
		else {
		    print $new_sock "${INCANTATION_S}RUN_S${TERMINATION}";
		    print $new_sock "$elapsedtime";   
		    my $buf = $client_id->getline();
		    print "RC say: $buf /n";
		}
	    }
	    elsif($mystate eq "BEFOREBIRTH_IS") { # started up for the first time
		gaibu_msg($MNOTICE,"Connected to RC");
		print $new_sock "${INCANTATION_S}BOOTING_MS${TERMINATION}"; 
		tarry $tarrytimeRC; # sleep(2);
		print "${INCANTATION_S}BOOTING_MS${TERMINATION}"; 
		################################################
		# BOOTING_MS 
		##################################################
		# START UP DAQ
		system qq|ssh root\@${OVDAQ} "${online_path}\/DCOV\/readout\/script\/start_readout.sh ${online_path} readout ${DataPath} ${DAQ_GAIBU_OFF}"|;
		$timecount = 0;
		#################################################
		$mystate = "BOOTED_S"; 
		print "${INCANTATION_S}${mystate}${TERMINATION}"; 
		gaibu_msg($MNOTICE,$mystate);
		print $new_sock "${INCANTATION_S}${mystate}${TERMINATION}"; 
		tarry $tarrytimeRC; # sleep(2);
	    }
	    else {
		print $new_sock "${INCANTATION_S}${mystate}${TERMINATION}"; 
		tarry $tarrytimeRC; # sleep(2);
	    }
	}

# otherwise it is a socket that has been already connected and we should read and process the buffer 
    else {
 
	my $buf = $client_id->getline(); # grab a line .. it wouldn't block due to the select

	if ($buf) {  #if a command is waiting in the buffer
            #print "\n Client $ipnum[$client_number] say: $buf \n";
	    #here in future we maybe would like to check also the IP address of DCRC before executing any command
	    print "DCRC: $buf\n";
	    
	    if($buf =~ /INITIALISE/) {
		if($mystate eq "BOOTED_S") {
		    print $client_id "${INCANTATION_S}INITIALISING_MS${TERMINATION}"; #sleep(1);
		    $mystate = "INITIALISED_S";
		    $stoptime = "";
		    print "${INCANTATION_S}${mystate}${TERMINATION}"; 
		    gaibu_msg($MNOTICE,$mystate);
		    print $client_id "${INCANTATION_S}${mystate}${TERMINATION}"; 
		    tarry $tarrytimeRC;
		} else {
		    gaibu_msg($MWARNING,"Error in ${mystate}: Receieved $buf");
		}
	    }
	    elsif($buf =~ /START/) {
		if($mystate eq "INITIALISED_S") {
		    print $client_id "${INCANTATION_S}STARTING_MS${TERMINATION}";
		    load_db_config($db_config_file);
		    test_system_time;       #this will start taking data and check for timing issue in the all OV
		    tarry $tarrytimeRC;
	            my $buf = $client_id->getline();
		    print "DCRC: $buf\n";
		    $run_number = sprintf("%0.7d",$buf);
		    print "Run number from RC = $buf\n";
		    tarry $tarrytimeRC;
   		    print $client_id "RUNNUMBER\n";
		    tarry $tarrytimeRC;
	            $buf = $client_id->getline();
		    print "DCRC: $buf\n";
		    $run_comment = $buf; 
		    print "Run comment from RC = $buf\n";
		    tarry $tarrytimeRC;
   		    print $client_id "RUNCOMMENT\n"; 
		    tarry $tarrytimeRC;
		    $buf = $client_id->getline();
		    print "DCRC: $buf\n";
		    $run_shifter = $buf;
		    print "Shifter name from RC = $buf\n";
		    tarry $tarrytimeRC;
   		    print $client_id "RUNSHIFTER\n"; 
		    tarry $tarrytimeRC;
	            $buf = $client_id->getline();
		    print "DCRC: $buf\n";
		    $run_length = $buf; 
		    print "Run length from RC = $buf\n";
		    tarry $tarrytimeRC;
   		    print $client_id "RUNLENGTH\n";
		    tarry $tarrytimeRC; 
	            $buf = $client_id->getline();
		    print "DCRC: $buf\n";
		    $RUN_definition = $buf;
		    print "Run profile from RC = $buf\n";
		    getrunconfig($RUN_definition);   #getting RunConfiguration from file
		    if($buf =~ /DCPHYS/){
			print ("Runtype = P \n");
			$run_OVtype = "P";
			$runnumbermode = "RC";
			$DataPath = "OVDAQ/DATA";
		    }
		    elsif($buf =~ /DCCALIB/){
			print ("Runtype = C \n");
			$run_OVtype = "C";
			$buf = $client_id->getline();
			print "DCCalib: $buf\n";
			$onlinetable = $buf;
			$runnumbermode = "auto";
			print "Online_table in use = $buf\n";
			$buf = $client_id->getline();
			print "DCCalib: $buf\n";
			$run_length = $buf;
			$DataPath = "OVDAQ/DATA";
		    }
		    else { 
			print ("Runtype = D \n");
			$run_OVtype = "D";
			$runnumbermode = "RC";
			$DataPath = "OVDAQ/DATA";
		    }

   		    print $client_id "RUNPROFILE\n"; 
#add test on Run_number
		    if(duplicate_run_number() == 1) {
			gaibu_msg($MERROR,"OVRC received a duplicate Run Number");
			$mystate = "INITIALISED_S";
                        print "${INCANTATION_S}${mystate}${TERMINATION}";
                        gaibu_msg($MNOTICE,$mystate);
                        print $client_id "${INCANTATION_S}${mystate}${TERMINATION}";
                        tarry $tarrytimeRC; # sleep(1);
		    }
		    else {
			loadconfig();
			checknumusbs();
			initializeboard(${runnumbermode},${numbaselinetriggers});                    #run number is set in the function, set_run_number
			initializetriggerbox();
			
			check_system_time;                                                           #check time alignment as file dimension
			
			check_rate($Time_Data_Path,$totalpmt,$totusbs,$usb_box);
			
			check_rate("${DataFolder}/Run_${run_number}/binary",$totalpmt,$totusbs,$numbaselinetriggers);                 
			#fill the run summary table
			my ($oldsec,$oldmin,$oldhour,$oldday,$oldmonth,$oldyr19,@oldrest) =   localtime(time);
			$oldhour = sprintf("%0.2d",$oldhour);
			$oldmin = sprintf("%0.2d",$oldmin);
			$oldsec = sprintf("%0.2d",$oldsec);
			$starttime = $oldhour.":".$oldmin.":".$oldsec;
			loadrunsummarymysql();
			checkHVstatus();
			#
			($DAQ_hostname,my @extra) = split(/\./,gethostbyaddr(inet_aton($OVRC_IP), AF_INET));
			check_eventbuilderhost_diskspace(${OVEVENTBUILDER_HOST});
			
			if($eventbuilderhost_usage lt $maxdiskusage) {    # OV EVENT BUILDER data disk is less than 97% full
			    print "${OVEVENTBUILDER_HOST}\n";
			    if($eb_flag == 1) {
				system qq|ssh ${user_online}\@${OVEVENTBUILDER_HOST} "${online_path}\/DCOV\/readout\/startupscript\/start_OV_tools.sh ${remote_path} EventBuilder ${usedogsifier} ${disk} ${run_number} ${threshold} ${trigger} ${run_OVtype} ${DAQ_hostname} ${eb_disk}"|;
				
			    } else { gaibu_msg($MERROR,"OV EBuilder not started"); }
			}
			else {
			    gaibu_msg($MERROR,"OV EBuilder disk over 96% full! OV EBuilder not started");
			}
			$mystate = "STARTED_S";
			print "${INCANTATION_S}${mystate}${TERMINATION}"; 
			gaibu_msg($MNOTICE,$mystate);
			print $client_id "${INCANTATION_S}${mystate}${TERMINATION}"; 
			tarry $tarrytimeRC; # sleep(1);
			# Start taking data
			starttakedata;
			####
			if($run_OVtype eq "C") {
			    print $client_id "${run_number}${TERMINATION}";
			    tarry $tarrytimeRC;
			}
		    }
		} else {
		    gaibu_msg($MWARNING,"Error in ${mystate}: Receieved $buf");
		}
	    }
	    elsif($buf =~ /TRIGGERON/) {
		if($mystate eq "STARTED_S") {
		    #####################################################
		    # TriggerON HS
		    #####################################################
		    # Start taking data
		    starttakedata;
                    #write down in run summary table the actual triggerON time
		    print $client_id "${INCANTATION_S}TRIGGERON_HS${TERMINATION}"; 
		    tarry $tarrytimeRC; # sleep(1);
		    $mystate = "RUN_S";
		    print "${INCANTATION_S}${mystate}${TERMINATION}"; 
		    gaibu_msg($MNOTICE,$mystate);
                    ## Current implementation of run control moves directly from TriggerOFF_HS to INITIALISED_S
                    updaterunsummarymysql("wheretorun",$nfs_mount);
		    ($oldsec,$oldmin,$oldhour,$oldday,$oldmonth,$oldyr19,@oldrest) =   localtime(time);
                    ## write down in run summary table the actual triggerON time
		    $oldhour = sprintf("%0.2d",$oldhour);
		    $oldmin = sprintf("%0.2d",$oldmin);
		    $oldsec = sprintf("%0.2d",$oldsec);
		    $starttime = $oldhour.":".$oldmin.":".$oldsec;
		    updaterunsummarymysql("starttime",$starttime);
		    print $client_id "${INCANTATION_S}${mystate}${TERMINATION}"; 
		    tarry $tarrytimeRC; # sleep(2);
		} else {
		    gaibu_msg($MWARNING,"Error in ${mystate}: Receieved $buf");
		}
	    }
	    elsif($buf =~ /TRIGGEROFF/) {
		if($mystate eq "RUN_S" || $mystate eq "RUNAUTO_S") {
		    #####################################################
		    # TriggerOFF HS
		    #####################################################
		    # Stop taking data
		    #stoptakedatafaketriggerbox;
		    stoptakedata;
		    #######################################################
		    print $client_id "${INCANTATION_S}TRIGGEROFF_HS${TERMINATION}";
		    tarry $tarrytimeRC; # sleep(2);
		    $mystate = "STARTED_S";
		    print "${INCANTATION_S}${mystate}${TERMINATION}"; 
		    gaibu_msg($MNOTICE,$mystate);
		    print $client_id "${INCANTATION_S}${mystate}${TERMINATION}"; 
		    tarry $tarrytimeRC; # sleep(2);
		} else {
		    gaibu_msg($MWARNING, "Error in ${mystate}: Receieved $buf");
		}
	    }
	    elsif($buf =~ /STOP/) {
		if($mystate eq "STARTED_S") {		    
		    ##################################################
		    # Stopping MS
		    stoptakedata;
		    ###################################################
                    #run pre-datamigro script
		    system qq|ssh ${user_online}\@${OVONLINEMONITOR_HOST} "${online_path}\/DCOV\/readout\/startupscript\/start_OV_pre_datamigro.sh ${run_number} ${remote_path}"|;
		    ###################################################
		    print $client_id "${INCANTATION_S}STOPPING_MS${TERMINATION}"; 
		    tarry $tarrytimeRC; # sleep(2);
		    $mystate = "INITIALISED_S";
		    print "${INCANTATION_S}${mystate}${TERMINATION}"; 
		    my ($tmpsec,$tmpmin,$tmphour,$tmpday,$tmpmonth,$tmpyr19,@tmprest) =   localtime(time);
		    $tmphour = sprintf("%0.2d",$tmphour);
		    $tmpmin = sprintf("%0.2d",$tmpmin);
		    $tmpsec = sprintf("%0.2d",$tmpsec);
		    $stoptime = $tmphour.":".$tmpmin.":".$tmpsec;
		    updaterunsummarymysql("stoptime",$stoptime);
		    gaibu_msg($MNOTICE,$mystate);
		    print $client_id "${INCANTATION_S}${mystate}${TERMINATION}"; 
		    tarry $tarrytimeRC; # sleep(2);
		} else {
		    gaibu_msg($MWARNING, "Error in ${mystate}: Receieved $buf");
		}
	    }
	    elsif($buf =~ /QUIT/) {
		if($mystate eq "INITIALISED_S") {
		    ##################################################
		    # Quitting MS
		    ###################################################
		    print $client_id "${INCANTATION_S}QUITTING_MS${TERMINATION}"; 
		    tarry $tarrytimeRC; # sleep(2);
		    $mystate = "BOOTED_S";
		    print "${INCANTATION_S}${mystate}${TERMINATION}"; 
		    gaibu_msg($MNOTICE,$mystate);
		    print $client_id "${INCANTATION_S}${mystate}${TERMINATION}"; 
		    tarry $tarrytimeRC; # sleep(2);
		} else {
		    gaibu_msg($MWARNING, "Error in ${mystate}: Receieved $buf");
		}
	    }
	    elsif($buf =~ /EXIT/) {
		if($mystate eq "BOOTED_S") {
		    ##################################################
		    # Dying MS
		    ###################################################
		    # Stop OV DAQ
		    system qq|${online_path}\/DCOV\/readout\/script\/stop_readout.sh readout|;
		    ###################################################
		    system qq|ssh ${user_online}\@${OVEVENTBUILDER_HOST} "${online_path}\/DCOV\/readout\/startupscript\/stop_OV_tools.sh ${remote_path} EventBuilder"|;
		    system qq|ssh ${user_online}\@${OVEVENTBUILDER_HOST} "${online_path}\/DCOV\/readout\/startupscript\/stop_OV_tools.sh ${remote_path} DOGSifierOV"|;
		    $mystate = "AFTERDEATH_IS";
		    print "${INCANTATION_S}${mystate}${TERMINATION}"; 
		    gaibu_msg($MNOTICE,$mystate);
		    gaibu_msg($MWARNING, "OV DAQ Killed by RC");
		    exit(1);
		} else {
		    gaibu_msg($MWARNING, "Error in ${mystate}: Receieved $buf");
		}		
	    } 
	    elsif($buf =~ /KILL/) {
	        ##################################################
	        # Killing MS
		stoptakedata;
	        ###################################################
	        # Kill OVMonitor, OVEBuilder, OVDAQ
		system qq|ssh ${user_online}\@${OVEVENTBUILDER_HOST} "${online_path}\/DCOV\/readout\/startupscript\/stop_OV_tools.sh ${remote_path} DOGSifierOV"|;
		system qq|ssh ${user_online}\@${OVEVENTBUILDER_HOST} "${online_path}\/DCOV\/readout\/startupscript\/stop_OV_tools.sh ${remote_path} EventBuilder"|;
		system qq|ssh root\@{OVDAQ} "${online_path}\/DCOV\/readout\/script\/stop_readout.sh readout"|;
	        ###################################################
		$mystate = "AFTERDEATH_IS";
		print "${INCANTATION_S}${mystate}${TERMINATION}"; 
		gaibu_msg($MNOTICE, $mystate);
  	        my ($tmpsec,$tmpmin,$tmphour,$tmpday,$tmpmonth,$tmpyr19,@tmprest) =   localtime(time);
		$tmphour = sprintf("%0.2d",$tmphour);
		$tmpmin = sprintf("%0.2d",$tmpmin);
		$tmpsec = sprintf("%0.2d",$tmpsec);
                $stoptime = $tmphour.":".$tmpmin.":".$tmpsec;
                updaterunsummarymysql("stoptime",$stoptime);
                updaterunsummarymysql("comment","Killed by RC");
		gaibu_msg($MWARNING, "OV DAQ Killed by RC");
		exit(1);
	    }
	    else {
		gaibu_msg($MWARNING, "Error in ${mystate}: Command $buf not recognized.");	
	    }

	}  
	else{
####the other end of the socket closed...close our end and remove
#it from the list of sockets to listen to..
	    print "\n Warning client $ipnum[$client_number], close socket -- removing client from read set\n";
	    $client_number--;
	    $Read_Handles_Object->remove($client_id); 
	    close($client_id);
	    if($mystate eq "RUN_S") { 
		$mystate = "RUNAUTO_S"; 
		print "${INCANTATION_S}${mystate}${TERMINATION}"; 
	        gaibu_msg($MNOTICE, $mystate); 
	    }

	}#closing if on the buffer

    }#closing if on the socket identification

}#closing on foreach client that has connected

# after going through the sockets doing something else

#here I can check that the readout processes are running
#start checking only after 2 minutes after booting the DAQ

    if($timecount > 10) { #initialize when the ROP start

	my $response = is_readout_alive($onlinetable);
#if I was restarting and I succeed then reset the variable restart_time
	if($restart_time > 0) {
	    if ($response == 0) { $restart_time = 0; } #everything good
	}
	
#if something fail
	
	if($response == 1) {  #detected an error
#first time try to restart
	    if($restart_time == 0) {
		print "Got a problem with the readout process, try to restart \n";
		$restart_time ++;
		sendmail("OVRC","WARNING","Detected a problem with the ROP cpu/memory usage");
#	    goto restarteverything;
	    }
	    if($restart_time > 0) {
#hang and wait forever
		print "Readout was restarted once and failed again - require calling expert \n";
#	    <STDIN>;
	    }
	}
	if($response == 2) {
		print "Got a problem with the number of readout process, they are not all there\n";
		stoptakedata;
  	        my ($tmpsec,$tmpmin,$tmphour,$tmpday,$tmpmonth,$tmpyr19,@tmprest) =   localtime(time);
		$tmphour = sprintf("%0.2d",$tmphour);
		$tmpmin = sprintf("%0.2d",$tmpmin);
		$tmpsec = sprintf("%0.2d",$tmpsec);
                $stoptime = $tmphour.":".$tmpmin.":".$tmpsec;
                updaterunsummarymysql("stoptime",$stoptime);
                updaterunsummarymysql("comment","Killed by OVRC - USB Processor problems");
		gaibu_msg($MWARNING, "OV DAQ Killed by OVRC - Not all USB processor are running");
		exit(1);
	    }
    }

#check on file dimension

    if($mystate eq "RUN_S" or $mystate eq "RUNAUTO_S") {
	($currsec,$currmin,$currhour,$currday,$currmonth,$curryr19,@currrest) =   localtime(time);
	$elapsedtime = ($currday-$oldday)*24*3600 + ($currhour-$oldhour)*3600 + ($currmin-$oldmin)*60 + ($currsec - $oldsec);
	if($elapsedtime > 15) {  #wait at least 30 sec from the start of the run
	    if(check_file_size()==0) {
		print "Checking file size \n";
		print "Got a problem with the readout process, file sizes are different \n";
	    #goto restarteverything;
	    }
	}
    }

#check log file for error messages from the DAQ

    if($timecount > 10) { #initialize when the ROP start
	my $log_error = checkROPLog($run_number);

	if( $log_error == 1 ) {

	    print "OVRC - Readout processes is getting into trouble - need expert intervention\n";
#here could be possible to have a script to power cycle the DAQ group over the WTI
	    #cyclepower();
	}
    }



if($mystate eq "RUNAUTO_S") { #if it's run_auto I check for the time length of the run
    ($currsec,$currmin,$currhour,$currday,$currmonth,$curryr19,@currrest) =   localtime(time);
    $elapsedtime = ($currday-$oldday)*24*3600 + ($currhour-$oldhour)*3600 + ($currmin-$oldmin)*60 + ($currsec - $oldsec);
    if($elapsedtime > $run_length) {
	stoptakedata;
	$mystate = "STARTED_S";
	print "${INCANTATION_S}${mystate}${TERMINATION}"; 
	gaibu_msg($MNOTICE, $mystate);
	rc_runend_comment($detector_configuration);
    }
}
    tarry 5;
    $timecount++;

#ops we are not connected anymore try to look if something happen to the config file
#check if the local config and port has changed on the NFS folder, if yes then close and reatart the OVRC TCP Server


#restart looking for socket
} #at this point we have done (match while 1)
