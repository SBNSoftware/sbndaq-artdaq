use lib $ENV{DCONLINE_PATH} . "/DCOV/readout/macro";

use strict;
use warnings;
use DCFOV;

my ($sth,$drh,$Path);
#reading the IP configuration files
my $ip_config_file = "DCSpaceIP.config";      load_ip_config($ip_config_file);
my $port_config_file = "DCSpacePort.config";  load_port_config($port_config_file);
my $db_config_file = "DCDatabase.config";     load_db_config($db_config_file);

#look for the current date
my ($sec,$min,$hour,$day,$month,$yr19,@rest) = localtime(time);
my $date_old;

$yr19 = $yr19 + 1900; # + 1900; # hack to get something for no                                        
$month = sprintf("%0.2d",${month}+1);
$day = sprintf("%0.2d",$day);
$hour = sprintf("%0.2d",$hour);
$min = sprintf("%0.2d",$min);

my $date_now = "${yr19}${month}${day}";

$month = $month - 1; #delete data of the previous month

$month = sprintf("%0.2d",$month);

my ($daq_disk,$daq_disk_inuse);

eval {
    $dsn = "dbi:mysql:database=$database;host=$DBhostname";

    $dbh = DBI->connect($dsn, $user, $password);

    $drh = DBI->install_driver("mysql");

    $sth = $dbh->prepare("SELECT daq_disk from OV_runsummary where Run_number=(select max(Run_number) from OV_runsummary)");   
# this will get the DAQ disk not used by the DAQ where the old data is located                             
     $sth->execute();
    while (my $ref = $sth->fetchrow_arrayref()) {
	$daq_disk_inuse = $ref->[0];
    }
};

system qq| date |;

print "DAQ disk in use by DAQ: $daq_disk_inuse \n";

if($daq_disk_inuse == 1) { $daq_disk = 2; }
else { $daq_disk = 1; }
    
print "The timing data will be deleted on disk: $daq_disk \n";

#Path for the Timing Data is fixed
my $path = "/dcfovdaq/data${daq_disk}/OVTime/DATA";

if(-e $path) { #check if the data path exists
    open IN, "cd $path ; ls -ltr --time-style=long-iso |";
    while(<IN>) {
	if(/ (\S+) (\d+)-(\d+)-(\d+) \d+:\d+ Run_(\d+)/) {
            #print "$2-$yr19 $3-$month $4-$day, $5\n";                                                                        
	    print "Path $path/Run_$5 taken on $2$3$4 is going to be deleted ...\n";
            system qq| time rm -rf $path/Run_$5 |;
	}
    }
    close IN;
}    

print "Timing data deleting done \n";








