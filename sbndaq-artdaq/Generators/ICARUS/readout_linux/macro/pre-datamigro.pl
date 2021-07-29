use lib $ENV{DCONLINE_PATH} . "/DCOV/readout/macro";

use strict;
use warnings;
use DCFOV;
use Sys::Syslog qw( :DEFAULT setlogsock);

#set location of file for datamigro
#binary data could be found on dcfovmon
#/data1/OVDAQ/DATA/Run_number/processed  ->this will go into /data1/OVDAQ/DATA/Run_number.tar.gz

#dogsified data
#/data2/OVDAQ/DATA/  <----this is also the folder to monitor

#reading the IP configuration files
my $EB_counter = 0;
my $DF_counter = 0;

my $ip_config_file = "DCSpaceIP.config";      load_ip_config($ip_config_file);
my $port_config_file = "DCSpacePort.config";  load_port_config($port_config_file);
my $db_config_file = "DCDatabase.config";     load_db_config($db_config_file);

my $Run_number = shift;
if(!defined($Run_number)) {
    print "Run number not defined\n";
    exit(0);
}

$Run_number = sprintf("%0.7d",$Run_number);

my $remote_path = shift;
my $error_cnt;

#gaibu_msg_serv("EB_Proc_Mon",$MNOTICE,"OV EB process monitor started");
syslog('LOG_NOTICE',"OV EB process monitor started");

if(!defined($remote_path)) {
    $remote_path = "/local/home/dconline";
    print "Remote path not defined not, assuming $remote_path \n";
}
my ($drh,$sth,$file_count);

    if(!defined($OVEVENTBUILDER_HOST)) {
        print "Error! Variable OVEVENTBUILDER_HOST not defined!\n";
    }
    if(!defined($OVDAQ)) {
        print "Error! Variable OVDAQ_HOST not defined!\n";
    }

#check for the data disk on mysql for the eventbuilder
#table dc_ov_fd.OV_ebuilder variable Path

my ($EBexitstatus,$already_processed);

eval {
    $dsn = "dbi:mysql:database=$database;host=$DBhostname";

    $dbh = DBI->connect($dsn, $user, $password);

    $drh = DBI->install_driver("mysql");

    $sth = $dbh->prepare("SELECT process_finish FROM OV_runsummary WHERE Run_number='$Run_number'");
    $sth->execute();
    while (my $ref = $sth->fetchrow_arrayref()) {
	 if(defined($ref->[0])) {
	     $already_processed = $ref->[0];
	 }
	 else {
	     $already_processed = 0;
	 }
    }
    $sth->finish();

    $dbh->disconnect();
};

if($already_processed == 1) {
    print "Run: $Run_number has been already processed \n";
}

restart:

eval {
    $dsn = "dbi:mysql:database=$database;host=$DBhostname";

    $dbh = DBI->connect($dsn, $user, $password);

    $drh = DBI->install_driver("mysql");

    $sth = $dbh->prepare("SELECT EBretval FROM OV_runsummary WHERE Run_number='$Run_number'");
    $sth->execute();
    while (my $ref = $sth->fetchrow_arrayref()) {
	$EBexitstatus = $ref->[0];
    }
    $sth->finish();

    $dbh->disconnect();
};

if($EBexitstatus == 0) { #EB running ok
    $EB_counter++;
    if($EB_counter > 62) { goto timeout; }
    sleep 10;
    goto restart;
} 
elsif($EBexitstatus == -1) {
#EB exit with Fatal
    # gaibu_msg_serv("EB_Proc_Mon",$MWARNING,"EB for Run=$Run_number reported Fatal Error - now quitting");
    sendmail("EB_Proc_Mon","ERROR","EB for Run=$Run_number reported Fatal Error - now quitting");
    exit(0);
}

#at this point the exit flag for the EB should be 1 = success

#    now we can check if the DOGSifier has finished
my $EB_path;
my $DOGSifier_finished = 0;

eval {
    $dsn = "dbi:mysql:database=$database;host=$DBhostname";

    $dbh = DBI->connect($dsn, $user, $password);

    $drh = DBI->install_driver("mysql");

    $sth = $dbh->prepare("SELECT Path FROM OV_ebuilder WHERE Run_number='$Run_number'");
    $sth->execute();
    while (my $ref = $sth->fetchrow_arrayref()) {
	$EB_path = $ref->[0];
    }
    $sth->finish();

    $dbh->disconnect();
};

  tryagain:
$file_count = 0;
if(-e "${EB_path}Run_${Run_number}") {
    open IN, "ls ${EB_path}Run_${Run_number}/ |" or die $!;
    while(<IN>) {
	next if $_ =~ /processed/;
	++$file_count;
    }
    close IN or die $!;
}
else {
    if($already_processed ==1 && -e "${EB_path}DCRunF${Run_number}OVDAQ.tar.gz") {
	print "Run: ${Run_number},already processed, found zip file \n"; 
    }
    else {
	print "Run: ${Run_number}, Path for BIN data not found \n";
	exit(0);
    }
}    


    if($file_count > 0) {
        print ".";
	$DF_counter++;
	if($DF_counter > 500) { goto timeout; }
        sleep 10;
        goto tryagain;
    }
# no files remain to be processed but now I will check if they are closed properly
    else {
#check if the ROOT files are closed properly
	my $rootexe = "$ENV{'ROOTSYS'}/bin/root.exe -l -q";
	my $dogsdatapath = "$ENV{'DOGS_PATH'}/DATA";
	my @files = <${dogsdatapath}/DCRun${Run_number}*>;

        $error_cnt = 0;
	foreach my $file (@files) {
	    open IN, "$rootexe $file |" or die $!;
	    while(<IN>) {
		if($_ =~ /Warning/ or $_ =~ /Error/ or $_ =~ /ERROR/) {
		    $error_cnt++;
		}
	    }
	    close IN or die $!;
	}

	if($error_cnt > 0) {
	    print "Found $error_cnt Bad files \n";
	    sendmail("EB_Proc_Mon","ERROR","Found $error_cnt Bad File in Run=$Run_number");
	    goto timeout;
	}
        #print "EB/DOGSifier have finished processing run $Run_number\n";
    }

#   at this point if no more file and the run is ended already make a system call and zip the EB_path/Run_number

if($already_processed == 0) {
    system qq| cd ${EB_path}; time tar -cvzf DCRunF${Run_number}OVDAQ.tar.gz Run_${Run_number}/processed/*.bin|;

    system qq| cd ${EB_path}; time rm -r Run_${Run_number}|;


#now update the OV_runsummary table for datamigro

    eval {
	$dsn = "dbi:mysql:database=$database;host=$DBhostname";
	
	$dbh = DBI->connect($dsn, $user, $password);
	
	$drh = DBI->install_driver("mysql");
	
	$dbh->do("UPDATE OV_runsummary set process_finish = 1 where Run_number = $Run_number;");
	
	$dbh->disconnect();
    };

	# gaibu_msg_serv("EB_Proc_Mon",$MNOTICE,"Run=$Run_number - processed successfully");
}

timeout:
    if($EB_counter > 62) { 
	# gaibu_msg_serv("EB_Proc_Mon",$MERROR,"Timeout: waiting too long for EB for Run=$Run_number - now quitting");
	sendmail("EB_Proc_Mon","ERROR","Timeout: waiting too long for EB for Run=$Run_number - now quitting");
    }

    if($DF_counter > 62 || $error_cnt > 0) { 

	# gaibu_msg_serv("EB_Proc_Mon",$MERROR,"Timeout: waiting too long for DOGSifier for Run=$Run_number - now quitting");
	sendmail("EB_Proc_Mon","ERROR","Timeout: waiting too long for DOGSifier for Run=$Run_number - now quitting");
    }
    exit(0);
