use lib $ENV{DCONLINE_PATH} . "/DCOV/readout/macro";

use strict;
use warnings;
use DCFOV;
use Proc::ProcessTable;

$remote_path = "/local/home/dconline/myOnline";       #run on the local installation - no Gaibu here

#reading the IP configuration files

my $ip_config_file = "DCSpaceIP.config";      load_ip_config($ip_config_file);
my $port_config_file = "DCSpacePort.config";  load_port_config($port_config_file);
my $db_config_file = "DCDatabase.config";     load_db_config($db_config_file);

my $Run_number = shift;
if(!defined($Run_number)) {
    print "Run number not defined\n";
    exit(0);
}

$Run_number = sprintf("%0.7d",$Run_number);

#gaibu_msg_serv("DF_Proc_Mon",$MNOTICE,"OV DO process monitor started");

my ($p,$t,$pid,$drh,$sth,$file_count);

sleep(20);

while(1) { #infinite loop
    $pid = 0;
    $t = new Proc::ProcessTable;
    foreach $p (@{$t->table}){
	if($p->cmndline =~/DOGSifierOV $Run_number/){
	       $pid = $p->pid;
	   }
    }
#    print "DOGSifier for Run=$Run_number has process number $pid \n";

    if($pid == 0) { exit(0);}
    else {
	eval {
	    $dsn = "dbi:mysql:database=$database;host=$DBhostname";
	    $dbh = DBI->connect($dsn, $user, $password);
	    $drh = DBI->install_driver("mysql");
	    $sth = $dbh->prepare("SELECT EBretval FROM OV_runsummary WHERE Run_number='$Run_number'");
	    $sth->execute();
	    while (my $ref = $sth->fetchrow_arrayref()) {
		if( $ref->[0] == -1 ) {
		    #EB Fatal exit on this run - DOGSifier need to be stopped for this run
		    #kill the DOGSifier process
#		    print "Found DF Process $pid for Run=$Run_number running without EB\n";
		    sendmail("OVDF Monitor","ERROR","Found DF Process for Run Number = $Run_number running when EB exit with Fatal, killing DF");
		    system qq| kill -15 ${pid} |;
		    #then exit
		    exit(0);
		}
	    }
	    $sth->finish();	    
	    $dbh->disconnect();
	};
    }
    sleep(60); #wait 5 minutes and then recheck
#    print "Waiting for Run=$Run_number - $pid\n";
}

