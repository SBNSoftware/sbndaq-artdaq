use lib $ENV{DCONLINE_PATH} . "/DCOV/readout/macro";
use strict;
use warnings;
use usb_readout;
use English;
use DCFOV;

use IO::Socket;
use IO::Select;
use Time::HiRes;

my $processName = "DCOVCalib";

$OUTPUT_AUTOFLUSH = 1; #writing messages immediately to STDOUT

#convergence condition

my $condition = 0.001;

#runlenght definition
my $runlength_calib     = 10; #4*60*60;


#Initialize process log: AJF
##my $logPath = $ENV{DCONLINE_PATH} . "/DCOV/log/";
##my $logFile = $logPath . $processName . ".log";
##open STDOUT, ">$logFile";
##open STDERR,, ">&STDOUT"; #print ERR and OUT to same place



#reading the IP configuration files
my $ip_config_file = "DCSpaceIP.config";      load_ip_config($ip_config_file);
my $port_config_file = "DCSpacePort.config";  load_port_config($port_config_file);
my $db_config_file = "DCDatabase.config";     load_db_config($db_config_file);

my $command;
my $termination = "\n";



reconnect:
#setup the socket for the TCP/IP
my $sock = new IO::Socket::INET ( 
PeerAddr => $OVRC_IP, 
PeerPort => $OVRC_Port, 
Proto => 'tcp', ); 
if(!$sock) {
    print "Could not create socket: $!\n";
    print "Waiting 2 sec and try to reconnect \n";
    tarry 2;
    if(!$sock){
	print " Try to restart DCOVRC \n";
        #system qq|ssh root@snap "~dcdaq/dchooz/dconline/trunk/DCOV/readout/script/start_OV_RC.sh startOVDAQ.pl" |;
	#system qq|ssh root\@snap "/etc/init.d/OVreadout restart" |; #AJF, 3/21/11
        tarry 2;
	goto reconnect;
    }
}

#die "Could not create socket: $!\n" unless $sock; 
print "Opening OV RC socket on $OVRC_IP:$OVRC_Port \n\n";

my $step = 0;
my $stopped = 0;
my $stopping = 0;

#argument to be passed to OVRC
my $run_number_calib     = 0.;             
$run_comment    = "OVCalibration";
$run_shifter    = "OVCalib_sys";
my $starttime;
my $stoptime;
my $date;
my $diffintime;
my $run_definition = "DCCALIB";
$onlinetable       = "online_CHOOZ_OV";
my $delay = 1.;
my $spreadold = -1;
my $onlinetablenew;
my $starttimesec;
my $stoptimesec;
my $char;
my ($tmpsec,$tmpmin,$tmphour,$tmpday,$tmpmonth,$tmpyr19,@tmprest);
my ($oldsec,$oldmin,$oldhour,$oldday,$oldmonth,$oldyr19,@oldrest);


my $correctinitialization = 0;
################

while (1) {
restart:
#here we will put all the command to be passed to the OVRC

    $char = $sock->getline(); # grab a line ..

    if($char) { #if a command is waiting in the buf
	print "OVRC:$char\n";

	if($char =~ m/BOOTED_S/){
	    print $sock "INITIALISE$termination";
            $correctinitialization = 1;
	    tarry $delay;
	}
    	
	elsif($stopped == 0 && $char =~ m/INITIALISED_S/){
	    $DataPath = "OVCalib/DATA/";
	    #$DataPath = "OVDAQ/DATA/";
	    $disk = 2;
	    print $sock "START\n";
	    tarry $delay;
	}
	
	elsif($char =~ m/STARTING_MS/){

	    #send run_number
	    print $sock "${run_number_calib}$termination";
	    tarry $delay;
	    $char = $sock->getline();        # grab a line ..
	    print "OVRC:$char\n";
	    tarry $delay;

	    #send run_comment
	    print $sock "$run_comment$termination";
	    tarry $delay;
	    $char = $sock->getline();        # grab a line ..
	    print "OVRC:$char\n";
	    tarry $delay;
	    
	    #send run_shifter
	    print $sock "$run_shifter$termination";
	    tarry $delay;
	    $char = $sock->getline();        # grab a line ..
	    print "OVRC:$char\n";
	    tarry $delay;
	    
	    #send run_length
	    print $sock "$runlength_calib$termination";
	    tarry $delay;
	    $char = $sock->getline(); # grab a line ..
	    print "OVRC:$char\n";
	    tarry $delay;

	    #send run_definition
	    print $sock "$run_definition$termination";
	    tarry $delay;

	    #send onlinetable
	    print $sock "$onlinetable$termination";
	    tarry $delay;

	    #send runlenght
	    print $sock "$runlength_calib$termination";
	    tarry $delay;
	    $char = $sock->getline(); # grab a line ..
	    print "OVRC received Run_Definition:$char\n";
	    tarry $delay;
	}
	elsif($char =~ m/STARTED_S/ && $stopping == 0){
            if($correctinitialization == 1){
		$char = $sock->getline(); # grab a line ..
		chomp($run_number = $char);
		print "OVRC:$char\n";
		print $sock "TRIGGERON$termination";
		tarry $delay;
	    }
	    else{
		goto Iwasdead;
	    }
	}
	elsif($char =~ m/RUN_S/){
	    ($oldsec,$oldmin,$oldhour,$oldday,$oldmonth,$oldyr19,@oldrest) =   localtime(time);
	    $oldhour = sprintf("%0.2d",$oldhour);
	    $oldmin = sprintf("%0.2d",$oldmin);
	    $oldsec = sprintf("%0.2d",$oldsec);
	    $starttime = $oldhour.":".$oldmin.":".$oldsec;
            $starttimesec = $oldhour*60*60+$oldmin*60+$oldsec;
	    tarry $runlength_calib;      #run until time expire
	    print $sock "TRIGGEROFF$termination";  #shutting down the run
	    tarry $delay;
	    $stopping = 1;
	}
	elsif($char =~ m/STARTED_S/ && $stopping == 1){
	    print $sock "STOP$termination";
	    $char = $sock->getline(); # grab a line ..
	    tarry $delay;
	    $stopping = 0;
	    $stopped = 1;
	}
	elsif($stopped == 1 && $char =~ m/INITIALISED_S/){
	    #start analysis here
	    $onlinetablenew = "${onlinetable}_${step}";

	    #timer start ->start_time on mysql
	    ($tmpsec,$tmpmin,$tmphour,$tmpday,$tmpmonth,$tmpyr19,@tmprest) =   localtime(time);
	    $tmpyr19 = $tmpyr19 + 1900; # + 1900; # hack to get something for no
	    $tmpmonth = sprintf("%0.2d",${tmpmonth}+1);
	    $tmpday = sprintf("%0.2d",$tmpday);
	    $date = "${tmpyr19}${tmpmonth}${tmpday}";	  
########
	    #$onlinetable name change
            $onlinetable = $onlinetablenew;               
#######	    
	    #update of OVCalib_Summary on MySQL
	    OVCalibSummary($starttime,$date,$run_number);     

###### check for the eventbuilder processing the run
            print "disk = $disk, datapath=$DataPath, run_number = $run_number\n";

	    checkforlateeventbuilder($disk,$DataPath,$run_number);


###### check for the dogsifier to complete the dogsification

	    checkDOGSifier($run_number);

###### start OV Calib
	    
	    system qq|ssh ${user_online}\@${OVEVENTBUILDER_HOST} "${online_path}\/DCOV\/readout\/startupscript\/start_OV_calib.sh ${run_number}"|;

	    checkOVCalibend($run_number); #check that the process on crackle is ended, check every 20s to see if the OVCalib Summary table has been updated.

	    ($tmpsec,$tmpmin,$tmphour,$tmpday,$tmpmonth,$tmpyr19,@tmprest) =   localtime(time);
	    $tmphour = sprintf("%0.2d",$tmphour);
	    $tmpmin = sprintf("%0.2d",$tmpmin);
	    $tmpsec = sprintf("%0.2d",$tmpsec);
	    $stoptime = $tmphour.":".$tmpmin.":".$tmpsec;                      ###stop time
            $stoptimesec = $tmphour*60*60+$tmpmin*60+$tmpsec;                  ###stop time in sec
           
            $diffintime = $stoptimesec - $starttimesec;

            print "Analysis and Data taking time: $diffintime (sec)\n";

	    updateCalibSummary($stoptime);
	    
	    if($step==0){
		$spreadold = $spreadCalib;
                #$spreadold = 11;
	    }
	    else {
		#$spreadold = 11;
		#$spreadCalib = 10;
		if( (($spreadold-$spreadCalib)/$spreadold) < $condition){
		    $stopped = 0;
		    goto converged;		       
		}
		else{
		    $spreadold = $spreadCalib;   
		}
	    }
	    $step++;
	    $stopped = 0;
	    print "DONE with step $step, restarting in 5 sec. \n";
	    tarry 5;
            <STDIN>;      ##############################################
	    print $sock "START\n";
	    tarry $delay; 
	    goto restart;
	}
	elsif($char =~ m/AFTERDEATH_IS/) { 
            exit(0);
	}
    
    } 
    else {
	print "Server Died\n";
	close($sock);
	goto reconnect;
    }
}



#after the code as minimized
converged:
print "Calibration has converged with final spread = ${spreadCalib}\n";
print $sock "KILL$termination";
tarry 2;
goto restart;



died:
print "Server died \n";
print "Trying to reconnect in 5 sec\n";
tarry 5;
goto reconnect;


Iwasdead:
	    print $sock "STOP$termination";
            $correctinitialization = 1;
	    tarry $delay;
goto restart;
            


forceexit:
close($sock);
exit;
