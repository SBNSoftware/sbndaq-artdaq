use lib $ENV{DCONLINE_PATH} . "/DCOV/readout/macro";

use strict;
use warnings;
use DCFOV;

my ($sth,$sth1,$drh,$Path);
#reading the IP configuration files
my $ip_config_file = "DCSpaceIP.config";      load_ip_config($ip_config_file);
my $port_config_file = "DCSpacePort.config";  load_port_config($port_config_file);
my $db_config_file = "DCDatabase.config";     load_db_config($db_config_file);

#check on the OV_runsummary table if there is any data that has been already deleted 
#on dcfovmon by DataMigro and that has been reprocessed successfully and 
#in case delete the data and update the OV_runsummary table

system qq|date |;

my $file_size;

my ($path,$run_number,$run_date,$run_erased,$run_disk_number,$pre_datamigro_flag);

eval {
    $dsn = "dbi:mysql:database=$database;host=$DBhostname";

    $dbh = DBI->connect($dsn, $user, $password);

    $drh = DBI->install_driver("mysql");

    $sth = $dbh->prepare("SELECT Run_number,DATE,bin_erased,daq_disk FROM OV_runsummary WHERE (bin_erased = 0 and daq_disk != (SELECT daq_disk from OV_runsummary where Run_number=(select max(Run_number) from OV_runsummary)) and Run_number < (select max(Run_number) from OV_runsummary)-700) ORDER by Run_number");   # this will get one row only with the minimum run number in the interval and will retrieve only run present in the disk not currently in use by the DAQ

    $sth->execute();
    while (my $ref = $sth->fetchrow_arrayref()) {
	$run_number= $ref->[0];
	$run_date = $ref->[1];
	$run_erased = $ref->[2];
	$run_disk_number = $ref->[3];

	if($run_disk_number !=1 and $run_disk_number !=2 ) {
	    print "Disk used from MySQL does not exist: $run_disk_number \n";
	}

	$path = "/dcfovdaq/data${run_disk_number}/OVDAQ/DATA/Run_${run_number}";

	if(-e $path) { #check if the data path exists
	    print "Run $run_number taken on $run_date and \n";
	    print "Path= $path is going to be deleted ...\n";
#check that the data is already been processed completely by EB and DF
	    $sth1 = $dbh->prepare("SELECT process_finish FROM OV_runsummary WHERE Run_number = $run_number");
	    $sth1->execute();
	    while (my $ref = $sth1->fetchrow_arrayref()) {
		$pre_datamigro_flag = $ref->[0];
		if(!defined($pre_datamigro_flag)) { $pre_datamigro_flag = 0; }
	    }
	    $sth1->finish();
	    
	    if($pre_datamigro_flag == 1) {
		#going to check if the run appears in the DataMigro delete table
		$sth1 = $dbh->prepare("SELECT SIZE FROM FileTrackTable_DeleteOV WHERE Run_number = $run_number");
		$sth1->execute();
		$file_size = -1;
		while (my $ref = $sth1->fetchrow_arrayref()) {
		    $file_size += $ref->[0];
		}
		$sth1->finish();
		if($file_size >0) {
		    #print "Going to delete from disk ${run_disk_number} Run number = $run_number -\n";
		    #print "Path = $path \n";
		    print "File Size from Delete Table: $file_size \n";
		    system qq| time rm -rf $path |;
		    print "-----------------------------------------------\n";
		    #now update the OV_runsummary table
		    $dbh->do("UPDATE OV_runsummary set bin_erased = 1 where Run_number = $run_number;");
		}
		else { # data not yet deleted there was a probem with DataMigro
                    sendmail("OVDAQ_bin_removal","WARNING","Data on disk-$run_disk_number for Run_number = $run_number not deleted - DataMigro problem");
		}
	    }
	    else {
		print "Warning data on disk=$run_disk_number for Run=$run_number cannot be deleted EB-DF not ended properly\n";
		sendmail("OVDAQ_bin_removal","WARNING","Data on disk-$run_disk_number for Run_number = $run_number not deleted EB and DF did not run properly");
	    }
	}    
	else {
	    print "Path $path not found on data disk -- Data deleted already for $run_number --\n";
	    $dbh->do("UPDATE OV_runsummary set bin_erased = 1 where Run_number = $run_number;");
	}
    
	sleep(1);
    }

    $sth->finish();
    
    $dbh->disconnect();
};


