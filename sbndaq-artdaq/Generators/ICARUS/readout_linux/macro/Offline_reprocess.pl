use lib $ENV{DCONLINE_PATH} . "/DCOV/readout/macro";

use strict;
use warnings;
use DCFOV;
use Sys::Syslog qw( :DEFAULT setlogsock);

#reading the IP configuration files
$remote_path = "/local/home/dconline/myOnline";
our $nfs_path = "/nfs/dconline/myOnline";

my $EB_counter = 0;
my $DF_counter = 0;

our $ip_config_file = "DCSpaceIP.config";      load_ip_config($ip_config_file);
our $port_config_file = "DCSpacePort.config";  load_port_config($port_config_file);
our $db_config_file = "DCDatabase.config";     load_db_config($db_config_file);

syslog('LOG_NOTICE',"OV Re-process started");

system qq| date|;
our ($drh,$sth,$file_count,$Run_number);
my ($dsn2,$dbh2,$drh2,$sth2);
my ($DMdsn,$DMdbh,$DMdrh,$DMsth);

if(!defined($OVEVENTBUILDER_HOST)) {
    print "Error! Variable OVEVENTBUILDER_HOST not defined!\n";
}
if(!defined($OVDAQ)) {
    print "Error! Variable OVDAQ_HOST not defined!\n";
}

#check for status and data disk on mysql for the eventbuilder
#table dc_ov_fd.OV_ebuilder variable Path

my ($EBexitstatus,$EBexitstatus2,$already_processed);
my ($EBthreshold,$EBtriggertype,$EBruntype,$DAQdisk,$DAQFolder);
my $DFok = -999;
my $EBflag;
my $altertable;
my $error_cnt;
my $stop_time;
#get the list of Run Number for the Run that has EB exit with Fatal or process_finish not completed

eval {
    $dsn = "dbi:mysql:database=$database;host=$DBhostname";
    $dbh = DBI->connect($dsn, $user, $password);
    $drh = DBI->install_driver("mysql");
    $sth = $dbh->prepare("SELECT process_finish, Run_number, EBretval, SW_Threshold, SW_TriggerMode, Run_Type, daq_disk, stop_time FROM OV_runsummary where Run_number > 31000 and Run_number < (select max(Run_number) from OV_runsummary)-2 and process_finish =0 order by Run_number"); 
    $sth->execute();
    while (my $ref = $sth->fetchrow_arrayref()) {
	$Run_number = sprintf("%0.7d",$ref->[1]);	   
	$EBthreshold = $ref->[3];
	$EBtriggertype = $ref->[4];
	$EBruntype = $ref->[5];
	
	if($EBruntype eq "P") {  #if run is PHYS
	    $DAQFolder = "OVDAQ/DATA";
	}
	elsif($EBruntype eq "D") { #if run is DEBUG
	    $DAQFolder = "OVDAQ/DATA";
	}
	elsif($EBruntype eq "C") { #if run is CALIB
	    $DAQFolder = "OVDAQ/DATA";
	}

	$DAQdisk = $ref->[6];
	$stop_time = $ref->[7];
	$altertable = 0;

	if(defined($ref->[0])) { #check if the flag is defined
	    if($ref->[0] == 0) { #processing of this run was not completed - try to re-EB and/or re-DF
               ######
		print "Debug: Run=$Run_number processing was not completed: (EB flag=$ref->[2]) \n";
		if($ref->[2] == -1 || $ref->[2] == 0 || !defined($stop_time)){   #EB was not running or EB exited with Fatal
		    my $file_count = 0;
		    my %lasttimestamp;
		    #counting how many .bin file are in the DAQ folder if any then the data processing
                    #has not been completed
		    my $binpath = "/dcfovdaq/data$DAQdisk/$DAQFolder/Run_$Run_number/binary";
		    print "Run=$Run_number, Binary Path:$binpath \n";
		    open IN, "cd $binpath ; ls -ltr --time-style=long-iso *|";
		    while(<IN>) {
			if(/ \S+ \d+-\d+-\d+ \d+:\d+ (\d+)_(\d+).wr/) {
			    system qq| mv ${binpath}/$1_$2.wr ${binpath}/$1_$2 |;
			}
		    }
		    open IN, "cd $binpath ; ls -ltr --time-style=long-iso *|";
		    while(<IN>) {
			if(/ \S+ \d+-\d+-\d+ \d+:\d+ (\d+)_(\d+)/) {
			    ++$file_count;
			    #print "Debug: binary usb = $2, timestamp = $1\n";
			    if(!defined($lasttimestamp{$2})){$lasttimestamp{$2} = $1; }
			    elsif($1 < $lasttimestamp{$2}) {$lasttimestamp{$2} = $1; }
			} 
		    }
#		    my $usbnumber1;
#		    foreach $usbnumber1 (keys(%lasttimestamp)){
#			print "$usbnumber1, $lasttimestamp{$usbnumber1}\n";
#		    }
		    if($file_count > 0 and $file_count == 5) {
			#if they are only 5 this is the end of the run, concatenate thse files to the last decoded files
			# move the last decoded files
			#get the max timestamp for the file in the decoded folder
			my $decodedpath = "/dcfovdaq/data$DAQdisk/$DAQFolder/Run_$Run_number/decoded";
			print "Run=$Run_number, decoded Path:$decodedpath \n";
			my %maxtimestamp = ();
			my $usbnumber;
		        open IN, "cd $decodedpath ; ls -ltr --time-style=long-iso *.done|";
		        while(<IN>) {
			    if(/ \S+ \d+-\d+-\d+ \d+:\d+ (\d+)_(\d+)/) {
#				print "Debug: decoded usb = $2, timestamp = $1\n";
				if(!defined($maxtimestamp{$2})){$maxtimestamp{$2} = $1; }
				if($1 > $maxtimestamp{$2}) {$maxtimestamp{$2} = $1; }
			    }
			}
			foreach $usbnumber (keys(%maxtimestamp)){
			    system qq| cat ${binpath}/$lasttimestamp{$usbnumber}_${usbnumber} >> ${decodedpath}/$maxtimestamp{$usbnumber}_${usbnumber}.done |;
			    system qq| mv ${decodedpath}/$maxtimestamp{$usbnumber}_${usbnumber}.done ${binpath}/$maxtimestamp{$usbnumber}_${usbnumber} |;
			    system qq| rm ${binpath}/$lasttimestamp{$usbnumber}_${usbnumber} |;
			}

		    }
		    elsif($file_count == 0 && $ref->[2] == -1) { # data need to be reprocessed 
			print "Found condition for reprocessing data with EB \n";
			# goto skipEB;
                        # need to modify some table to be able to have the EB reprocess the data
			$altertable = 1;
			$dbh->do("UPDATE OV_runsummary set SW_threshold = 72 where Run_number = $Run_number;");
			$dbh->do("UPDATE OV_ebuilder set Res1 = SW_threshold where Run_number = $Run_number;");
			$dbh->do("UPDATE OV_ebuilder set SW_threshold = 72 where Run_number = $Run_number;");
			unless(-e "/dcfovmon/data1/OVDAQ/REP/Run_${Run_number}") {
			    print "Folder not found - creating folder \n";
			    system qq| mkdir /dcfovmon/data1/OVDAQ/REP/Run_${Run_number}; chmod 777 /dcfovmon/data1/OVDAQ/REP/Run_${Run_number} |;
			}
		    }
		    elsif($file_count == 0) { #EB was success
			$dbh->do("UPDATE OV_runsummary set EBretval=1 where Run_number = $Run_number;");
			goto skipEB; 
		    }

# try to rerun EB:
                #update the stop_time if not present                                                                             
		    if(!defined($stop_time)) {
			$dbh->do("UPDATE OV_runsummary set stop_time = '25:25:25' where Run_number = ${Run_number}");
		    }

		    print "Try to rerun EB with following parameters: D 0, -t $EBthreshold, -T $EBtriggertype, -R $EBruntype -\n";
		    $dbh->do("UPDATE OV_runsummary set EBretval = -2 where Run_number = $Run_number;");
		    eval {
			system qq|cd ${remote_path}/DCOV/EBuilder/bin; ./EventBuilder -r $Run_number -D 0 -t $EBthreshold -T $EBtriggertype -R $EBruntype >> ${remote_path}/DCOV/log/EventBuilder_repro.log 2>&1 |;
		    };
skipEB:
		    if($altertable == 1) {
			#put back the original value in MySQL
			$dbh->do("UPDATE OV_runsummary set SW_threshold = $EBthreshold where Run_number = $Run_number;");
			$dbh->do("UPDATE OV_ebuilder set SW_threshold = Res1 where Run_number = $Run_number;");
			#move the data from /dcfovmon/OVDAQ/REP to /dcfovmon/OVDAQ/DATA  
			system qq| mv /dcfovmon/data1/OVDAQ/REP/Run_${Run_number}/T2ADC0073P100P200/Run_${Run_number} /dcfovmon/data1/OVDAQ/DATA/.\
			    |;
			system qq| rm -rf /dcfovmon/data1/OVDAQ/REP/Run_${Run_number} |;
		    }

		    #check if the EB completed the reprocess successfully
		    $dsn2 = "dbi:mysql:database=$database;host=$DBhostname";
		    $dbh2 = DBI->connect($dsn2, $user, $password);
		    $drh2 = DBI->install_driver("mysql");
		    $sth2 = $dbh2->prepare("SELECT EBretval FROM OV_runsummary WHERE Run_number='$Run_number'");
		    $sth2->execute();
		    while (my $ref2 = $sth2->fetchrow_arrayref()) {
			$EBflag = $ref2->[0];
			if($EBflag == -1 || $EBflag == 0){
			    print "EB for Run=$Run_number reported Fatal Error again \n";
			    #EB fails again and need some expert to look into what is happening
			    sendmail("EBDF_reprocess","ERROR","EB for Run=$Run_number reported Fatal Error");
			    # set value to -2 in such a way the run would not be reprocessed a second time
			    $dbh->do("UPDATE OV_runsummary set EBretval = -2 where Run_number = $Run_number;");
			    $DFok = 0;
			    next();
			}
			elsif($EBflag == 1) { #EB reprocess was ok
			    #run the DF then
			    print "Try to run the DF after EB\n";
			    $DFok = checkDF($Run_number);
			    if($DFok == -1){ #DF need to rerun			
				eval {
				    print "Going to rerun DF for Run=$Run_number \n";
				    system qq| cd ${remote_path}/DCOV/readout/macro; perl OfflineDOGSifier.pl $Run_number 1|;
				    $DFok = checkDF($Run_number); #recheck if the DF completed successfully
				    if($DFok == -1) { #try once more the DF
					sleep(5);
					system qq| cd ${remote_path}/DCOV/readout/macro; perl OfflineDOGSifier.pl $Run_number 1|;
					$DFok = checkDF($Run_number); #recheck if the DF completed successfully
				    }
				};
			    }			    
			}
		    } 
		    $sth2->finish();
		    $dbh2->disconnect();
		}
		elsif($ref->[2] == 1) {#EB was ok, the problem is going to be the DF
		    #try to rerun the DF
		    print "EB was ok, try to rerun the DF\n";
		    $DFok = checkDF($Run_number);
		    if($DFok == -1){ #DF need to rerun			
			eval {
			    print "Going to rerun DF for Run=$Run_number \n";
			    system qq| cd ${remote_path}/DCOV/readout/macro; perl OfflineDOGSifier.pl $Run_number|;
			    $DFok = checkDF($Run_number); #recheck if the DF completed successfully
			    if($DFok == -1) { #try once more the DF
				sleep(5);
				system qq| cd ${remote_path}/DCOV/readout/macro; perl OfflineDOGSifier.pl $Run_number|;
				$DFok = checkDF($Run_number); #recheck if the DF completed successfully
			    }
			};
		    }

		}#if on EB not executed or exit with Fatal
		elsif($ref->[2] == -2) {next();} #avoid to reprocess twice

		if($DFok == 1) { #everything went well now run pre-datamigro
		    print "Now going to run pre-datamigro \n";
		    system qq| cd ${remote_path}/DCOV/readout/macro/; perl pre-datamigro.pl ${Run_number} ${remote_path} |;
                    # run OVDataMigro on the reformatted run
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

#		    system qq| ${nfs_path}/DCDataMigro/scripts/runTransferOV -f ${DM_Run_number},export DCONLINE_PATH=/local/home/dconline/myOnline |;
		}
		else {
		    print "DOGSifier reprocess has failed processing for run $Run_number \n";
		    #notify expert via email
		    sendmail("EBDF_reprocess","ERROR","DF for Run=$Run_number reported Fatal Error");
		    $dbh->do("UPDATE OV_runsummary set EBretval = -2 where Run_number = $Run_number;");
		}
	    }
	}

#this should be empty;
#      skiprun:
#	print "Skipped run = $Run_number \n";
    }
    $sth->finish();	
    $dbh->disconnect();
};	  


#####################################################################################################################

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
	print "Run: ${DF_Run_number}, Path for BIN data not found \n";
	return -1;
    }    
    
    if($DF_file_count > 2) {
	    return -1;
    }
    else {
	my $logfile = "/local/home/dconline/myOnline/DCOV/log/DOGSifier/DOGSifierOV_${DF_Run_number}.log";
	if(-e "$logfile") {
	    my $log_check = 0;
	    open IN, $logfile;
	    while(<IN>) {
		if($_ =~ /Deleting Monitor.../) { 
		    $log_check = 1;
		}
	    }
	    if($log_check == 0) {return -1;}  #no end of DF found
	    else {
		system qq| mv /dcfovmon${EB_path}Run_${DF_Run_number}/DCRunF${DF_Run_number}* /dcfovmon${EB_path}Run_${DF_Run_number}/processed/.|;  #move last file to processed folder
	    }
	}    
	else {
	    return -1;  # no log file found
	}
	    
	print "DOGSifier have finished processing run $DF_Run_number\n";
	
#now check the quality of the DF data
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
	    print "Found $error_cnt Bad files from the DF during reprocessing of Run=$Run_number\n";
	    return -1;
	}
    }
    return 1;
}
