#!/usr/bin/perl

use strict;
use warnings;
use DBI;
use File::Copy;

my $ovbuffsize = 5000;
my $run = shift;
if(!defined($run)) {
    print "Which run would you like to DOGSify?\n";
    chomp($run = <STDIN>);
}
my $skip_stop_time = shift;

if(!defined($skip_stop_time)) {
    $skip_stop_time = 0;
}

my $dsn = 'dbi:mysql:dc_ov_fd:dcfmysql.in2p3.fr:3306';
my $user = 'OVuser';
my $pass = 'thetaOV13';
my ($ref,$myquery, $sqlQuery);

my $dbh = DBI->connect($dsn,$user,$pass) or 
    die "Can't connect to the DB: $DBI::errstr\n";

$myquery = sprintf("select stop_time from OV_runsummary where Run_number = '$run'");
$sqlQuery = $dbh->prepare($myquery);
$sqlQuery->execute();

if($ref = $sqlQuery->fetchrow_arrayref()) { 
    if($ref->[0] || $skip_stop_time == 1) {
	$myquery = sprintf("select Path,Run_number,Entry from OV_ebuilder where Run_number = '$run' order by Entry desc");
	$sqlQuery = $dbh->prepare($myquery);
	$sqlQuery->execute();
	if($ref = $sqlQuery->fetchrow_arrayref()) {
	    my $dir = "$ref->[0]Run_$ref->[1]";
	    foreach(<${dir}/processed/*>) {
		if($_ =~ /(.*)\/processed\/(.*)\.bin/) {
		    move("$_", "$1/$2");
		}
	    }
#delete previous DOGSified data
	    system qq| rm -f $ENV{'DOGS_PATH'}/DATA/DCRun$ref->[1]* |;
	    sleep(5);
	    my @files = <${dir}/*>;
	    print "Found $#files files.\n";
	    my $path = $ENV{'DCONLINE_PATH'};
	    my $exe = "cd $path/DCDOGSifier/bin && ";
	    $exe .= "nohup ./DOGSifierOV $run $ref->[0] $#files $ovbuffsize >> $path/DCOV/log/DOGSifier/DOGSifierOV_$run.log ";
	    $exe .= "2>>$path/DCOV/log/DOGSifier/DOGSifierOV_$run.log < /dev/null &";	    
	    print "$exe\n";
	    print "Offline DOGSifier is processing run $run...\n";
	    my $ans = `$exe`;
	}
    } else { 
	print "Run $run has not finished processing. Cannot be DOGSified offline.\n"; exit(1); 
    }
} else {
    print "Run $run not found in OV_runsummary table. Please select another run.\n"; exit(1);
}
