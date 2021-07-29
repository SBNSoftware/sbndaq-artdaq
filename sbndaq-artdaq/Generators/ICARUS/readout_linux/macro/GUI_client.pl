use strict;
use warnings;
use lib $ENV{DCONLINE_PATH} . "/DCOV/readout/macro";
use DCFOV;
use usb_readout;
use English;
use IO::Socket;
use IO::Select;
use Time::HiRes;
$OUTPUT_AUTOFLUSH = 1;                               # writing messages immediately to STDOUT


#server socket definition
my ($client_ip_number,$port,$client_ip,$new_sock,$addr);
my ($Read_Handles_Object,$client,$client_id,$client_conn,$client_number,@ipnum);

#reading the IP configuration files
my $ip_config_file = "DCSpaceIP.config";      load_ip_config($ip_config_file);
my $port_config_file = "DCSpacePort.config";  load_port_config($port_config_file);
my $db_config_file = "DCDatabase.config";     load_db_config($db_config_file);

# Prepare run control server socket
my $sock = new IO::Socket::INET ( 
    LocalHost => $OVCC_IP,                            # This server's IP address
    LocalPort => $OVCC_Port,                          # This server's open port
    Proto => 'tcp', 
    Listen => 15,                                     # define number of connection should be = 1 in the final version
    Reuse => 1, 
    ); 
die "Could not create socket: $!n" unless $sock;
print "Opening OV CC socket on $OVCC_IP:$OVCC_Port \n";

gaibu_msg($MNOTICE, "OV Command GUI Started");

$Read_Handles_Object = new IO::Select();              # create handle set for reading 
$Read_Handles_Object->add($sock);                     # add the main socket to the set 

while(1) {                                            # forever loop

# get a set of readable handles (blocks until at least one handle is ready)
my ($client_conn) = IO::Select->select($Read_Handles_Object, undef, undef, 0);

$client_number = 0;

# take all readable handles in turn
foreach $client_id (@$client_conn) { 
    $client_number++;
    #print "\n Found readable socket ... \n";
    #if it is the main socket then we have an incoming connection and 
    #we should accept() it and then add the new socket to the $Read_Handles_Object 
    if ($client_id == $sock) { 
	#you get this when a new socket connection is formed..a new client starts and it assign a socket filehandle to it.
	my ($new_sock,$addr) = $client_id->accept(); 
	my($port,$ip) = sockaddr_in $addr;
	$ipnum[$client_number] = inet_ntoa $ip;
	$Read_Handles_Object->add($new_sock);
	print "\n New Client connected from IP: $ipnum[$client_number] \n";		
	gaibu_msg($MNOTICE, "Client from IP: $ipnum[$client_number] connected to OV Command GUI");
    }
    else { 
	my $buf = $client_id->getline(); # grab a line .. it wouldn't block due to the select
	
	if($buf) {  #if a command is waiting in the buffer
	    #print "\n Client $ipnum[$client_number] say: $buf \n";
	    #here in future we maybe would like to check also the IP address of DCRC before executing any command
	    print "OV Command Client says: $buf\n";
	    
	    if($buf =~ /startOVRC/) {
		print $client_id "OVCC Received: $buf\n"; 
		#system qq|ssh root\@${OVDAQ} "~dcdaq/dchooz/dconline/trunk/DCOV/readout/script/start_OV_RC.sh startOVDAQ.pl"|;
		system qq|ssh root\@${OVDAQ} "/etc/init.d/OVreadout start"|;
	    }
	    elsif($buf =~ /startOVCalib/) { #DCOVCalib.pl if does not find it, will start DCOVCalib.pl
		print $client_id "OVCC Received: $buf\n"; 
		system qq|ssh root\@${OVDAQ} "~dcdaq/dchooz/dconline/trunk/DCOV/readout/macro/start_OV_RC.sh DCOVCalib.pl"|;
	    }
	    elsif($buf =~ /killOVcalib/) {
		print $client_id "OVCC Received: $buf\n"; 
		###force the stop of the OV DAQ readout process
		system qq|ssh root\@${OVDAQ} "${online_path}/DCOV/readout/script/stop_readout.sh readout"|;
		###restart the OV RC process with implicit restart of the OV DAQ readout process
		system qq|ssh root\@${OVDAQ} "${online_path}/DCOV/readout/script/stop_OV_RC.sh DCOVCalib.pl"|;
                ### check for child processes running on crackle DCOVCalib
                ###
		system qq|ssh dcdaq\@${OVEVENTBUILDER_HOST} "${online_path}/DCOV/readout/startupscript/stop_launch.sh DCOVCalib"|;
	    }
	    elsif($buf =~ /startALLONLINE/) {  #check the script to be called here
		print $client_id "OVCC Received: $buf\n"; 
		system qq|ssh dcdaq\@${OVEVENTBUILDER_HOST} "${online_path}/DCOV/readout/startupscript/start_online.sh"|;
	    }
	    elsif($buf =~ /killALL/) {         #check the script to be called here
		print $client_id "OVCC Received: $buf\n"; 
		system qq|ssh dcdaq\@${OVEVENTBUILDER_HOST} "${online_path}\/DCOV\/readout\/startupscript/stop_shift.sh"|;	   	    		
	    } 
	    else {    #command not recognized
		print $client_id "OVCC Received a not recognized Command: $buf\n"; 
		gaibu_msg($MWARNING, "Error receiving command from GUI: Command $buf not recognized.");	
	    }
	} # at this point we got disconnected
	else{
####the other end of the socket closed...close our end and remove
#it from the list of sockets to listen to..
	    $Read_Handles_Object->remove($client_id); 
	    close($client_id); 
	    print "\n Warning client $ipnum[$client_number], close socket -- removing client from read set\n";
	    
	}#closing if on the buffer
    }#closing if on the socket identification
}#closing on foreach client that has connected

# after going through the sockets doing something else
    
tarry 1;
#restart looking for socket
} #at this point we have done (match while 1)
