use lib $ENV{DCONLINE_PATH} . "/DCOV/readout/macro";

use strict;
use warnings;
use DCFOV;
use Sys::Syslog qw( :DEFAULT setlogsock);
use File::stat;

#reading the IP configuration files

$remote_path = "/local/home/dconline/myOnline";
our $nfs_path = "/nfs/dconline/myOnline";

my $EB_counter = 0;
my $DF_counter = 0;
our $stop_time;
our $ip_config_file = "DCSpaceIP.config";      load_ip_config($ip_config_file);
our $port_config_file = "DCSpacePort.config";  load_port_config($port_config_file);
our $db_config_file = "DCDatabase.config";     load_db_config($db_config_file);

syslog('LOG_NOTICE',"OV Re-process started");

system qq| date|;

my ($Run_number,$DFok);
our ($needtorun,$needtorunDF);
my ($dsn,$dbh,$drh,$sth);
my ($dsn2,$dbh2,$drh2,$sth2);
my ($dsn3,$dbh3,$drh3,$sth3);
my ($EB_NAME,$EB_PATH,$EB_SIZE,$EB_DATE,$EB_file,$EB_datetime);	    
my ($DF_NAME,$DF_PATH,$DF_SIZE,$DF_DATE,$DF_file,$DF_datetime);	    
my $wearegood;
#get the list of Run Number from 21000 and for each run in OV_runsummary check:
#1. date of the EB zip file and compare with the date (OPERATION_DATE) store in TransferOv table where DAQ_type = ovdaq
#2. date of the DF files and compare with the date (OPERATION_DATE) store in FileTrackTable_TransferOV where DAQ_type = raw

#the ORIGIN_PATH should be get from FileTrackTabe_TransferOV


eval {
    $dsn = "dbi:mysql:database=$database;host=$DBhostname";
    $dbh = DBI->connect($dsn, $user, $password);
    $drh = DBI->install_driver("mysql");
    $sth = $dbh->prepare("SELECT Run_number FROM OV_runsummary where Run_number > 56960 and Run_number < (select max(Run_number) from OV_runsummary)-2 and process_finish = 1");
# and Run_number > (select max(Run_number) from FileTrackTable_DeleteOV where DAQ_TYPE = 'ovdaq') order by Run_number"); 
    $sth->execute();

    while (my $ref = $sth->fetchrow_arrayref()) {
	$Run_number = int $ref->[0];	   
	print "Run= $Run_number \n";
	$needtorun = 0;
	$needtorunDF = 0;
	$wearegood = 0;
#EB check date and size####
	#get date and path for the Run from FileTrackTable_TransferOV for the EB file (DAQ_type = ovdaq)
	$dsn2 = "dbi:mysql:database=$database;host=$DBhostname";
	$dbh2 = DBI->connect($dsn2, $user, $password);
	$drh2 = DBI->install_driver("mysql");
	$sth2 = $dbh2->prepare("SELECT NAME, ORIGIN_PATH, SIZE, UNIX_TIMESTAMP(OPERATION_DATE) FROM FileTrackTable_TransferOV WHERE Run_number='$Run_number' and DAQ_TYPE = 'ovdaq'");
	$sth2->execute();
	if($sth2->rows == 0) {
	    print "Run=$Run_number -- ovdaq entry not found on TransferOV \n";
	    print "####### \n";
#at this point we got the zip file on disk and we need to re-run the DF
            $needtorun = 1;
	    rerunDF($Run_number);
	}
	else {
	    while (my $ref2 = $sth2->fetchrow_arrayref()) {
		$EB_NAME = $ref2->[0];
		$EB_PATH = $ref2->[1];
		$EB_SIZE = $ref2->[2];
		$EB_DATE = $ref2->[3];
		
		$EB_file = "/dcfovmon${EB_PATH}${EB_NAME}";
		
		if($EB_SIZE > 0) { # just to check that and entry is there and it is non zero
		    print "Run=$Run_number is updated on Irods \n";
		}
		else {
		    print "Run = $Run_number -- EB File not found /dcfovmon${EB_PATH}${EB_NAME} \n";
		    #need to make a call to irods or rerun on DAQ data
                    #check if deleted by datamigro in that case everything was fine
		    print "File deleted by DM so no problem \n";
		    $wearegood = 1;
		}
		
	    }
	    $sth2->finish();
	    $dbh2->disconnect();

#	    if($wearegood == 1){ goto nothingelsetocheck; }
#at this point ovdaq data is the most recent unless $needtorun = 1

#NOW the DF check date and size
	    #get date and path for the Run from FileTrackTable_TransferOV for the DF file (DAQ_type = raw)
	    $dsn3 = "dbi:mysql:database=$database;host=$DBhostname";
	    $dbh3 = DBI->connect($dsn3, $user, $password);
	    $drh3 = DBI->install_driver("mysql");
	    $sth3 = $dbh3->prepare("SELECT NAME, ORIGIN_PATH, SIZE, UNIX_TIMESTAMP(OPERATION_DATE) FROM FileTrackTable_TransferOV WHERE Run_number='$Run_number' and DAQ_TYPE = 'raw'");
	    $sth3->execute();
	    
	    if($sth3->rows == 0) {
		print "Run=$Run_number -- DF Data on DataMigro table not found \n";
		print "####### \n";
		$needtorun = 1;
		rerunDF($Run_number);
		$sth3->finish();
		$dbh3->disconnect();
	    }
	    else {
		while (my $ref3 = $sth3->fetchrow_arrayref()) {
		    $DF_NAME = $ref3->[0];
		    $DF_PATH = $ref3->[1];
		    $DF_SIZE = $ref3->[2];
		    $DF_DATE = $ref3->[3];
		    
		    $DF_file = "/dcfovmon${DF_PATH}${DF_NAME}";

		    if($DF_SIZE > 1) { #then it is fine
			    print "Run=$Run_number is updated on Irods \n";
		    }
		    elsif($DF_SIZE < 2) { #problem with the DF data dimension too small
			print "Run=$Run_number need to be reprocessed by DM \n";
			$needtorunDF = 1;
			$needtorun = 1;
		    }
		    else { #DF data not found problem as well
			print "Run = $Run_number -- DF File /dcfovmon${DF_PATH}${DF_NAME} not found \n";
			$needtorunDF = 1;
			$needtorun = 1;
		    }
		    
		}
	    }
	    $sth3->finish();
	    $dbh3->disconnect();
	    if($needtorunDF == 1) {
		print "Going to rerun DF for Run=$Run_number \n";
		rerunDF($Run_number);
	    }
	    
	}

	$sth2->finish();
	$dbh2->disconnect();

	if($needtorun == 1) {
	    print "Going to rerun DM for Run=$Run_number\n";
	    rerunDM($Run_number);
	}

nothingelsetocheck:

    } # close the call to the OV_runsummary

    $sth->finish();
    $dbh->disconnect();

};


################################################################################################

sub rerunDM {

    my($DMdsn,$DMdbh,$DMdrh);

    $Run_number = shift;
    
    my $DM_Run_number = int ${Run_number}; # this take out the leading zeros, DM does not like them
# update the datamigro flag to allow DataMigro to reprocess the run
    eval {
	$DMdsn = "dbi:mysql:database=$database;host=$DBhostname";
	$DMdbh = DBI->connect($DMdsn, $user, $password);
	$DMdrh = DBI->install_driver("mysql");
	$DMdbh->do("UPDATE RunTrackTable_TransferOV set STATUS_FLAG='ERROR', ERROR_FLAG='UNDEFINED' where run_number= $DM_Run_number");
    };
    $DMdbh->disconnect();

    system qq|ssh dconline\@${OVEVENTBUILDER_HOST} "${online_path}\/DCOV\/readout\/startupscript\/start_DM_reprocess.sh ${DM_Run_number}"|;
    
}

###################################################################################################

sub rerunDF {
    print "in rerunDF going to rerun DF for $Run_number, press enter to proceed \n";
    $Run_number = shift;

    $Run_number = sprintf("%0.7d",$Run_number);
    my $needtozip = 0;

    my ($EB_path,$DFdsn,$DFdbh,$DFdrh,$DFsth);
    my ($dsn4,$dbh4,$drh4,$sth4);
    my ($EBthreshold,$EBtriggertype,$EBruntype,$DAQdisk,$DAQFolder);
#get the file location

tryagainDF:
    eval {
	$DFdsn = "dbi:mysql:database=$database;host=$DBhostname";

	$DFdbh = DBI->connect($DFdsn, $user, $password);

	$DFdrh = DBI->install_driver("mysql");

	$DFsth = $DFdbh->prepare("SELECT Path FROM OV_ebuilder WHERE Run_number='$Run_number'");
	$DFsth->execute();
	while (my $DFref = $DFsth->fetchrow_arrayref()) {
	    $EB_path = $DFref->[0];
	}
	$DFsth->finish();

	$DFdbh->disconnect();
    };

#check that the zip file exist
#check if the DF file exist, maybe DF failed and that's it
    if(-e "/dcfovmon/data2/OVCommissioning/DATA/DCRun${Run_number}_RAW_v17_OV_muon.root") {
	print "DF File Found\n";
	$DFok = 1;
	goto fileDFexist;
    }
    
    if(-e "/dcfovmon/${EB_path}DCRunF${Run_number}OVDAQ.tar.gz" or -e "/dcfovmon/${EB_path}Run_${Run_number}") {

	if(-e "/dcfovmon/${EB_path}DCRunF${Run_number}OVDAQ.tar.gz") {
	    print "Unzipping file /dcfovmon/${EB_path}DCRunF${Run_number}OVDAQ.tar.gz \n";
	    system qq| cd /dcfovmon/${EB_path}; tar -xvf DCRunF${Run_number}OVDAQ.tar.gz|;
	}
	
	if(-e "/dcfovmon/${EB_path}Run_${Run_number}") {
	    $needtozip = 1;
	}
#rerun the DF
	eval {
	    print "Going to rerun DF for Run=$Run_number,press key to continue \n";
	    sleep 10;
	    system qq| cd ${remote_path}/DCOV/readout/macro; time perl OfflineDOGSifier.pl $Run_number 1|;

	    $DFok = checkDF($Run_number); #recheck if the DF completed successfully                    
	    if($DFok == -1) { #try once more the DF                                                    
		sleep(5);
		system qq| cd ${remote_path}/DCOV/readout/macro; time perl OfflineDOGSifier.pl $Run_number 1|;
		$DFok = checkDF($Run_number); #recheck if the DF completed successfully                
	    }
	};

fileDFexist:
#if DF complete then remove the EB file

	if($DFok == 1) {
	    #print "DF OK now - remove unzipped files \n";
	    if($needtozip == 0) {
		system qq| cd ${EB_path}; time rm -r Run_${Run_number}|;
		print "File deleted: Run_${Run_number} \n";
	    }
            #need to zip
	    if($needtozip == 1) {
	#   at this point if no more file and the run is ended already make a system call and zip the EB_path/Run_number     
		system qq| cd /dcfovmon/data1/OVDAQ/DATA/; time tar -cvzf DCRunF${Run_number}OVDAQ.tar.gz Run_${Run_number}/processed/*.bin|;
		system qq| cd /dcfovmon/data1/OVDAQ/DATA; time rm -r Run_${Run_number}|;
		print "File zipped and deleted for Run_${Run_number} \n";
	    }

	}
	else {
#DF was not successfull # do not re-delete the data send an email
	    print "DF failed for Run=$Run_number \n";
	}
    }
    else {
	print("Run: ${Run_number}, Got a problem with the EB zip file - not found\n");
	print("File ${EB_path}DCRunF${Run_number}OVDAQ.tar.gz not found \n");
	if($needtozip == 0) {
#now try to run the EB on dcfvoserv
	    eval {
		$dsn4 = "dbi:mysql:database=$database;host=$DBhostname";
		$dbh4 = DBI->connect($dsn4, $user, $password);
		$drh4 = DBI->install_driver("mysql");
		$sth4 = $dbh4->prepare("SELECT process_finish, Run_number, EBretval, SW_Threshold, SW_TriggerMode, Run_Type, daq_disk, stop_time FROM OV_runsummary where Run_number = $Run_number");
		$sth4->execute();
		while (my $ref4 = $sth4->fetchrow_arrayref()) {
		    $Run_number = sprintf("%0.7d",$ref4->[1]);
		    $EBthreshold = $ref4->[3];
		    $EBtriggertype = $ref4->[4];
		    $EBruntype = $ref4->[5];
		    
		    if($EBruntype eq "P") {  #if run is PHYS                                                                                       
			$DAQFolder = "OVDAQ/DATA";
		    }
		    elsif($EBruntype eq "D") { #if run is DEBUG                                                                                    
			$DAQFolder = "OVDAQ/DATA";
		    }
		    elsif($EBruntype eq "C") { #if run is CALIB                                                                                    
			$DAQFolder = "OVDAQ/DATA";
		    }
		    
		    $DAQdisk = $ref4->[6];
		    $stop_time = $ref4->[7];
		}
		$sth4->finish();
                #update the stop_time if not present
		if(!defined($stop_time)) { 
		    $dbh4->do("UPDATE doublechooz_ov_far.OV_runsummary set stop_time = '25:25:25' where Run_number = ${Run_number}"); 
		}
		$dbh4->do("DELETE from doublechooz_ov_far.OV_ebuilder where Run_number = ${Run_number} and Entry =0");
		$dbh4->disconnect();
	    };
	    
#need to check that all binary files has been moved to the decoded folder
	    if($DAQdisk == 1) {
		my $binpath = "/dcfovdaq/data1/OVDAQ/DATA/Run_${Run_number}";
		print "Run=$Run_number, Binary Path:${binpath}/binary \n";
		open IN, "cd ${binpath}/binary ; ls -ltr --time-style=long-iso *|";
#first remove .wr for file that have that exstension
		while(<IN>) {
		    if(/ \S+ \d+-\d+-\d+ \d+:\d+ (\d+)_(\d+).wr/) {
			#print "Debug: binary usb = $2, timestamp/filename = $1\n";
			system qq| mv ${binpath}/binary/$1_$2.wr ${binpath}/binary/$1_$2 |;
		    }
		}
		open IN, "cd ${binpath}/binary ; ls -ltr --time-style=long-iso *|";
		while(<IN>) {
		    if(/ \S+ \d+-\d+-\d+ \d+:\d+ (\d+)_(\d+)/) {
			#print "Debug: binary usb = $2, timestamp/filename = $1\n";
			system qq| mv ${binpath}/binary/$1_$2 ${binpath}/decoded/$1_$2.done |;
			#print "Moving file from binary to decoded press enter to continue \n";
		    }
		}
	    }
	    elsif($DAQdisk == 2) {
		my $binpath = "/dcfovdaq/data2/OVDAQ/DATA/Run_${Run_number}";
		print "Run=$Run_number, Binary Path:${binpath}/binary \n";
		open IN, "cd ${binpath}/binary ; ls -ltr --time-style=long-iso *|";
		while(<IN>) {
		    if(/ \S+ \d+-\d+-\d+ \d+:\d+ (\d+)_(\d+)/) {
			#print "Debug: binary usb = $2, timestamp/filename = $1\n";
			system qq| mv ${binpath}/binary/$1_$2 ${binpath}/decoded/$1_$2.done |;
			#print "Moving file from binary to decoded press enter to continue \n";
		    }
		}
		
	    }
	    print "Now try to run EB on dcfovserv with the following parameters: $Run_number, $EBthreshold, $EBtriggertype, $EBruntype \n";
	    sleep 10;
	    system qq|time ssh dconline\@dcfovserv.in2p3.fr "${online_path}\/DCOV\/readout\/startupscript\/start_OV_tools_nohup.sh ${remote_path} EventBuilder 0 1 ${Run_number} ${EBthreshold} ${EBtriggertype} ${EBruntype} dcfovserv.in2p3.fr 1"|;	    
	    #move the data from /dcfovmon/OVDAQ/REP to /dcfovmon/OVDAQ/DATA
	    system qq| mv /dcfovmon/data1/OVDAQ/REP/Run_${Run_number}/T2ADC0073P100P200/Run_${Run_number} /dcfovmon/data1/OVDAQ/DATA/. |;
	    system qq| rm -rf /dcfovmon/data1/OVDAQ/REP/Run_${Run_number} |;
	    #run the DF
	    $needtozip = 1;
	    goto tryagainDF;
	}
	else { # do not try more than once to run the EB on RAW data
	    $needtorun = 0;
	    sendmail("DM_reprocess","ERROR","EB zip file not found for Run=$Run_number - Fatal Error");
	}
    }

}

###################################################################################

sub checkDF {

    my $DF_Run_number = shift;

#check if DOGSfier finish ok                                                                                                   
    my $EB_path;
    my $DOGSifier_finished = 0;
    my ($DFdsn,$DFdbh,$DFdrh,$DFsth);

    eval {
        $DFdsn = "dbi:mysql:database=$database;host=$DBhostname";
        $DFdbh = DBI->connect($DFdsn, $user, $password);
        $DFdrh = DBI->install_driver("mysql");
        $DFsth = $DFdbh->prepare("SELECT Path FROM OV_ebuilder WHERE Run_number='$DF_Run_number'");
        $DFsth->execute();
        while (my $ref = $DFsth->fetchrow_arrayref()) {
            $EB_path = $ref->[0];
        }
        $DFsth->finish();
        $DFdbh->disconnect();
    };

    my $DF_file_count = 0;

    if(-e "/dcfovmon${EB_path}Run_${DF_Run_number}") {
        open IN, "ls /dcfovmon${EB_path}Run_${DF_Run_number}/ |" or die $!;
        while(<IN>) {
            next if $_ =~ /processed/;
            ++$DF_file_count;
        }
	close IN or die $!;
    }
    else {
        print "Try to re-DF Run: ${DF_Run_number}, Path for BIN data not found \n";
        return -1;
    }

    if($DF_file_count > 0) {
	return -1;
    }
    else {
        print "DOGSifier have finished processing run $DF_Run_number\n";
    }
    return 1;
}
