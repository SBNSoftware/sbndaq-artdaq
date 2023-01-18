package DCfunctions;


our (@ISA, @EXPORT, @EXPORT_OK, %EXPORT_TAGS, $VERSION);
use Exporter;
$VERSION = 1.00;
@ISA = qw(Exporter);

@EXPORT = qw(plotdata plotdatamb generatecsv generatebaseline getavg loadconfigmodule generatesummaryplot
checkeff hashValueAscendingNum $average $elapsed_time $rate $baselines1 $DataFolder $run $dcmacropath);


@EXPORT_OK = ();
%EXPORT_TAGS = ();

use lib $ENV{DCONLINE_PATH} . "/macro";

use IO::Socket;
use Time::HiRes;
use File::Path;
use Cwd;
use strict;
use warnings;
use usb_readout;
use DBI;
use Sys::Hostname;
use Net::SMTP;
use Getopt::Std;
use Sys::Syslog qw( :DEFAULT setlogsock);	
#use Filesys::DiskSpace;
use English;
use List::Util qw(sum);

require DCFOV;
require CRT;

setlogsock('unix');
openlog('OV Run Control','','user');

$OUTPUT_AUTOFLUSH = 1; #writing messages immediately to STDOUT

our ($DataFolder,$run,$dcmacropath,$baselines,$baselines1,$filename,$full_path);
our ($elapsed_time,$average);
our ($pmtserialnumber,$usemaroc2gainconstantsmb,$date,$laserrate);
our $module;
our @gainconst;

$laserrate = 1000;
our $online_path = $ENV{'DCONLINE_PATH'};
$dcmacropath = "../macro";


###########################################################################################
sub loadconfigmodule {


#loading info for the mysql database configuration


my $db_config_file = "DCDatabase.config";     
DCFOV::load_db_config($db_config_file);

DCFOV::loadconfig();

}


###########################################################################################
sub generatebaseline {


    my $usb = shift;
    my $pmt = shift;


    #DCFOV::getpmtdata($usb, $pmt);
    
    our $dir1 = "${DataFolder}/Run_${run}/PMT_${module}";                        # change this to create your own directory structure

    dir $dir1;                                                               # make directory

    my $filebase = "${DataFolder}/Run_${run}/binary/baseline_${usb}";    

  #  printf("Baseline file: $filebase\n");

    system qq|perl ${dcmacropath}/baselines.pl "${DataFolder}/Run_${run}/binary/" "baseline_${usb}" "${pmt}"|;

    system qq|mv ${DataFolder}/Run_${run}/binary/baselines.dat $dir1/baselines.dat|;
}

################################################################

sub generatecsv {

$DataFolder = shift;
$run = shift;

my $usb = shift;
my $pmt = shift;

# get the configuration from MySQL

#loadconfigmodule();

#generate the baseline

CRT::getpmtdata($usb, $pmt);

$module = $pmt;

generatebaseline($usb, $pmt);

@gainconst = @CRT::gain;

# print "Processing data for USB=$usb and PMT=$pmt\n";
# print "\n";

$full_path = "${DataFolder}/Run_${run}/PMT_${module}";

$baselines = "${DataFolder}/Run_${run}/PMT_${module}/baselines.dat";
#$filename = "${DataFolder}/Run_${run}/Module_${module}/signal.dec";

#$baselines = "${DataFolder}/Run_${run}/binary/baselines.dat";
$filename = "${DataFolder}/Run_${run}/signal.dec";

my $tempfile;

#check if signal.dec already exists, if so skip
if(-e "$filename") {
    print "Signal File Found\n";
} else {

#my @files = <${DataFolder}/Run_${run}/decoded/*_${usb}.done>; #exclude baselines
    my @files = <${DataFolder}/Run_${run}/binary/1*_${usb}>; #exclude baselines
    foreach $tempfile (@files) {
	system qq|/usr/bin/perl ${dcmacropath}/decode.pl "$tempfile" >> "${filename}"|;
    }
}

signal(${full_path}, ${filename}, $baselines, $pmt);
# system qq|/usr/bin/perl ${dcmacropath}/signal.pl "${full_path}" "${filename}" "$baselines" "join(" ",@gain)" "$pmt" >> ${full_path}/log_${usb}_${pmt}.txt|;

}

######################################################################################################################

sub plotdatamb {

my $usb = shift;
my $pmt = shift;
my $run_number = shift;
DCFOV::getpmtdata($usb, $pmt);

print "Plotting data for USB=$usb and PMT=$pmt\n";

$full_path = "/e/h.0/localdev/readout/data1/OVDAQ/DATA/Run_${run_number}";

# make histograms
system qq|/usr/bin/perl histogram.pl "$full_path" "summary.csv" 1 "Pulse heights" >> log_${pmtserialnumber}_${date}.txt|;
system qq|/usr/bin/perl histogram.pl "$full_path" "summary.csv" 2 "Sigma" >> log_${pmtserialnumber}_${date}.txt|;
system qq|/usr/bin/perl histogram.pl "$full_path" "summary.csv" 3 "Hits by channel" >> log_${pmtserialnumber}_${date}.txt|;
system qq|/usr/bin/perl histogram.pl "$full_path" "summary.csv" 5 "Photoelectrons" >> log_${pmtserialnumber}_${date}.txt|;
system qq|/usr/bin/perl histogram.pl "$full_path" "summary.csv" 6 "Gains" >> log_${pmtserialnumber}_${date}.txt |;
system qq|/usr/bin/perl histogram.pl "$full_path" "summary.csv" 7 "Rate" >> log_${pmtserialnumber}_${date}.txt|;
system qq|/usr/bin/perl histogram.pl "$full_path" "summary.csv" 8 "Gain constants" >> log_${pmtserialnumber}_${date}.txt|;

}

###########################################################################3

sub getavg {

my $sumfile = "summary.csv";
my $pmtmb = shift;
my $col = shift;

my @data;

# read summary file
if($pmtmb eq ""){
open IN, "$full_path/$sumfile" or die $!;
}
else {
open IN, "$full_path/$sumfile" or die $!;
}

while(<IN>) {
    chomp;
    s/\s*#.*//;
    next if /^\s*$/;
    my @line = split /\s*,\s*/;
    my $d = $line[$col];
    push(@data, $d);
}
close IN;

# calculate average
my $n = @data;
exit 0 if !$n;
$average = 0;
for(@data) {$average += $_;}
$average = $average / $n;
my $colname = "";
if($col ==1 ) { $colname = "pulse height"; }
elsif($col ==2 ) { $colname = "pulse height sigma"; }
elsif($col ==3 ) { $colname = "hits per channel"; }
elsif($col ==4 ) { $colname = "hits per packet"; }
elsif($col ==5 ) { $colname = "number of photo-electrons"; }
elsif($col ==6 ) { $colname = "gain"; }
elsif($col ==7 ) { $colname = "rate per channel"; }
printf "PMT ${pmtmb}:\t average ${colname} is $average\n";
return $average;
}


######################################################################################################
sub checkeff {

#my $file = shift;
my $pmt3 = shift;

my $argpath = shift;
my $localpath;
if( $argpath ) { $localpath = $argpath; }
else { $localpath = $full_path; }

my $file = $localpath . "/signal.dec";


#printf "localpath = $file\n";

my $TimeStampPerSecond = 1;  # Take into account the time interval between timestamps as well as the number of timestamps sent each time interval
my $countert = 0;
my $counterx = 0;
my $counterp = 0;
my $initial = 0;
my $final = 0;
my $flag = 0;
my $dummy = 0;

open IN, "$file" or die $!;
while(<IN>) {
	if(/^(t,)(\d+)/) {
		$dummy = $2;
		if(!$flag) {
			$flag=1;
			$initial=$dummy;
		}
		elsif($flag) {
			$final=$dummy;
		}
		$countert++;
	}
	elsif(/^(x,)/) {$counterx++;}
	elsif(/^(p,)/) {$counterp++;}
}
close IN;

$dummy = $final-$initial+1;
$dummy *= $TimeStampPerSecond;

open OUT, ">>${localpath}/Timestamps.txt" or die $!;
print OUT "\nData for $file\n";
print OUT "Initial time:               $initial\n";
print OUT "Final time:                 $final\n";
print OUT "Total timestamps expected:  $dummy\n";
print OUT "Total timestamps found:     $countert\n";
print OUT "Total x packets found:      $counterx\n";
print OUT "Total p packets found:      $counterp\n";

if($counterx > 0) {print "PMT: ${pmt3}\t Total x packets found:      $counterx\n"};
 
#if($counterx >= 3) { sendmail("Found $counterx x packets in file $file."); }

if($dummy-$countert) {
	$final = $counterx/($dummy-$countert)*100;
}
else {
	$final = "N/A";
}
$initial = $counterx/($counterp+$countert)*100;
$dummy = ($dummy-$countert)/$dummy*100;

#if($dummy > 15.0)  { sendmail("Percentage timestamps lost is larger than expected at $dummy percent in file $file."); }
print OUT "Percentage timestamps lost: $dummy\n\n";
print OUT "Extra packets as a percentage of:\n\n";
print OUT "Timestamps lost:           $final\n";
print OUT "Time + data packets found:  $initial\n";
close OUT;

}

######################################################################################################
sub signal {

my $nbins = 100;        # number of bins in histograms
my $gain_c = 1.45e4;    # constant used in gain calculation



my $dir = shift;
my $file = shift;
my $baselines1 = shift;
my $pmt_board = shift;

my $stefan = "Stefan Simion's Empty Histogram (tm). Formerly known as channel %d.";

my($x, $f, $i, $j);
my(@sig, @sig_dev, @base, @base_dev, @hist, @hist2, @hits_pc, @bins, @title, @photo, @gain, @gain_pmt);
for $i (1..64) {
    $sig[$i] = 0;
    $sig_dev[$i] = 0;
    $base[$i] = 0;
    $base_dev[$i] = 0;
    $hits_pc[$i] = 0;
    $photo[$i] = 0;
    $gain[$i] = 0;
    $hist[0]{$i} = 0;
    $gain_pmt[$i] = 0;
}

# get baselines1
print "Getting baselines...\n";

#print("Baseline address: $baselines1\n");


if(defined $baselines1) {
    open IN, $baselines1 or die $!;
    while(<IN>) {
        chomp;
        next if /^\s*$/;
        my($i, $m, $d) = split /,/;
        $base[$i] = $m;
        $base_dev[$i] = $d;
    }
    close IN or die $!;
}

# parse data file

#print "Getting signal file...$file\n";
# parse data file
open IN, "/usr/bin/perl ${dcmacropath}/unpacksignal.pl \"$file\" \"$pmt_board\" |" or die $!;
my ($first, $last, $count, $countovf);
$count = 0;
$last = 0;
$first = 0;
$countovf = 0;

while(<IN>) {
    chomp;
    my ($c, $d, $t) = split ",";
    if($count == 0) { 
        $first = $t; 
        $count++;
    }
	if($t < $last){
	  $countovf++;
	}
    $last = $t;
    if($c == 0) {
        $hist[$c]{$d}++;            # $hist[0] = hits per packet  
    } else {
#        $d -= $base[$c];
        $hist[$c]{$d}++;
        $hits_pc[$c]++;
    }
}
#print "number of ovf $countovf\n";
$countovf = $countovf * 2**32;
#print "number of ovl after multiply it: $countovf \n";
my $rate = ($last + $countovf - $first) * 16e-9;
#print "rate=${rate}, last packet = ${last}, first packet =${first}\n";
$rate = 1 / $rate if $rate;

close IN or die $!;

    
# calculate
print "Calculating...\n";
my($min, $max, $max_c) = (10000, -10000, -10000);
for $i (1..64) {
    my($avg, $dev) = (0, 0);
    my $n = $hits_pc[$i];
    next if $n == 0;
    while(($x, $f) = each(%{$hist[$i]})) {
        $avg += $f * $x;
        $min = $x if $x < $min;
        $max = $x if $x > $max;
    }
    $avg = $avg / $n;
    while(($x, $f) = each(%{$hist[$i]})) { $dev += $f * (($x - $avg) ** 2); }
    $dev = sqrt($dev / $n);
    $sig[$i] = $avg;
    $sig_dev[$i] = $dev;
    if($sig_dev[$i]) { $photo[$i] = ($sig[$i] / $sig_dev[$i]) ** 2; }
    if($photo[$i]) { $gain[$i] = $gain_c * $sig[$i] / $photo[$i]; }
}

# histogram hits
print "Histogramming...\n";
my $wbins = ($max - $min) / $nbins;
$wbins = 1 if $wbins == 0;
$min -= 0.0001 * $wbins;
$max += 0.0001 * $wbins;
#$min = -20; $max = 180; $nbins = 200;    ###################################
$wbins = ($max - $min) / $nbins;
for (0..$nbins-1) { push @{$hist2[0]}, $min + ($_ + 0.5) * $wbins; }
for $i (1..64) {
    for (0..$nbins-1) { $bins[$_] = 0; }
    while(($x, $f) = each(%{$hist[$i]})) { $bins[int(($x - $min) / $wbins)] += $f; }
    if($hits_pc[$i] == 0) { 
        $bins[0] += 0.00001; 
        $title[$i] = sprintf($stefan, $i);
    } else {
        $title[$i] = sprintf("Channel %02d    Mean = %.3f    Std Dev = %.3f    Total = %d",
            $i, $sig[$i], $sig_dev[$i], $hits_pc[$i]);
    }
    for (0..$nbins-1) { 
        push @{$hist2[$i]}, $bins[$_]; 
        $max_c = $bins[$_] if $bins[$_] > $max_c;
    }
}

# raw data file
open OUT, ">$dir/raw.csv" or die $!;
for $i (1..64) {
    print OUT "# Channel $i\n";
    if($hits_pc[$i] == 0) { print OUT "$min,0\n$max,0\n"; }
    else { while(($x, $f) = each(%{$hist[$i]})) { print OUT "$x,$f\n"; } }
    print OUT "\n\n";
}
close OUT or die $!;

# histogram file
open OUT, ">$dir/hist.csv" or die $!;
print OUT "#x,", (join ",", (1..64)), "\n";
for $j (0..$nbins-1) {
    print OUT $hist2[0][$j];
    for $i (1..64) { print OUT ",$hist2[$i][$j]"; }
    print OUT "\n";
}
close OUT or die $!;

# summary file
open OUT, ">$dir/summary.csv" or die $!;
print OUT "#channel,mean,dev,hits_per_channel,hits_per_packet,photo_e,gain,rate_per_channel,gain_const\n";
for $i (1..64) {
    my $tmp1 = $hits_pc[$i]*$rate;
    print OUT "$i,$sig[$i],$sig_dev[$i],$hits_pc[$i],$hist[0]{$i},$photo[$i],$gain[$i],$tmp1\n";    #$gainconst[$i]
}
close OUT or die $!;

# 2d summary
open OUT, ">$dir/summary_2d.csv" or die $!;
print OUT "#x,y,mean,dev,hits_per_channel,hits_per_packet,photo_e,gain\n";
for $i (0..8) {
    for $j (0..8) {
        my $xx = $i + 0.5;
        my $yy = $j + 0.5;
        my $c = 8 * $i + $j + 1;
        if($c <= 64) {
            print OUT "$xx,$yy,$sig[$c],$sig_dev[$c],$hits_pc[$c],$hist[0]{$c},$photo[$c],$gain[$c]\n";
        } else {
            print OUT "$xx,$yy,0,0,0,0,0,0\n";
        }
    }
    print OUT "\n";
}
close OUT or die $!;

# 2d histograms
open OUT, ">$dir/hist_2d.csv" or die $!;
print OUT "#x,y,z\n";
for $i (0..64) {
    for $j (0..$nbins) {
        my $xx = $i + 0.5;
        my $yy = $min + ($j + 0.5) * $wbins;
        if(($i < 64) && ($j < $nbins)) {
            print OUT "$xx,$yy,$hist2[$i + 1][$j]\n";
        } else {
            print OUT "$xx,$yy,0\n";
        }
    }
    print OUT "\n";
}
close OUT or die $!;

# change min and max so we can use them as ranges
$min += 0.5 * $wbins;
$max -= 0.5 * $wbins;

# generate plots
#$dir =~ s///g;
print "Plotting...\n";
open PLOT, ">plot.gp" or die $!;
print PLOT qq|
    set grid
    set datafile separator ","
    set term png size 1024,768
|;

# summary
print PLOT qq|
    set title "ADC pulse heights"
    set xlabel "Channel #"
    set ylabel "ADC counts"
    set autoscale
    set xrange [0.5:64.5]
    set yrange [$min:$max]
    set output "${dir}/mean.png"
    plot "${dir}/summary.csv" using 1:2 title "Mean" with histeps

    set title "ADC sigma"
    set xlabel "Channel #"
    set ylabel "ADC counts"
    set autoscale
    set xrange [0.5:64.5]
    set yrange [0:*]
    set output "${dir}/sigma.png"
    plot "${dir}/summary.csv" using 1:3 title "Sigma" with histeps

    set title "Hits by channel"
    set xlabel "Channel #"
    set ylabel "Count"
    set autoscale
    set xrange [0.5:64.5]
    set yrange [0:*]
    set output "${dir}/per_channel.png"
    plot "${dir}/summary.csv" using 1:4 title "Hits" with histeps
    
    set title "Hits in ADC packet"
    set xlabel "Number"
    set ylabel "Count"
    set autoscale
    set xrange [0.5:64.5]
    set yrange [0:*]
    set output \"${dir}/per_packet.png\"
    plot "${dir}/summary.csv" using 1:5 title "Hits" with histeps

    set title "Photoelectrons"
    set xlabel "Channel #"
    set ylabel "Number"
    set autoscale
    set xrange [0.5:64.5]
    set yrange [0:*]
    set output "${dir}/photo_e.png"
    plot "${dir}/summary.csv" using 1:6 title "Photoelectrons" with histeps

    set title "Gain"
    set xlabel "Channel #"
    set ylabel "Number"
    set autoscale
    set xrange [0.5:64.5]
    set yrange [0:*]
    set output "${dir}/gain.png"
    plot "${dir}/summary.csv" using 1:7 title "Gain" with histeps

    set title "Data acquisition rate per channel"
    set xlabel "Channel #"
    set ylabel "Rate [%]"
    set autoscale
    set xrange [0.5:64.5]
    set yrange [0:*]
    set output "${dir}/rate_per_channel.png"
    plot "${dir}/summary.csv" using 1:8 title "Rate (%)" with histeps
            
    set title "Maroc Gain Constants"
    set xlabel "Channel #"
    set ylabel "Number"
    set autoscale
    set xrange [0.5:64.5]
    set yrange [0:42]
    set output "${dir}/gain_const.png"
    plot "${dir}/summary.csv" using 1:9 title "Maroc Gain Constant" with histeps
|; 

# histograms - individual channels
for $i (1..64) {
    my $minus1 = $i - 1;
    my $plus1 = $i + 1;
    my $tmp = ($i < 10) ? "0$i" : $i;
    print PLOT qq|
        set title "$title[$i]"
        set autoscale
        set yrange [0:$max_c]
        set xlabel "ADC counts"
        set ylabel "Count"
        set output "${dir}/adc_$tmp.png"
        plot "${dir}/hist.csv" using 1:$plus1 title "Histogram" with histeps, \\
             "${dir}/raw.csv" index $minus1 title "Raw data" with impulses
    |;
}

# histograms - all in one
print PLOT qq|
    set title "All channels"
    set xlabel "ADC counts"
    set ylabel "Count"
    set output "${dir}/all_in_one.png"
    set autoscale
    set yrange [0:$max_c]
    set key spacing 0.5
    plot \\|;
for $i (1..64) {
    my $plus1 = $i + 1;
    print PLOT qq|
        "${dir}/hist.csv" using 1:$plus1 title "Channel $i" with histeps|;
    if($i < 64) { print PLOT ", \\"; }
}
print PLOT "\n";

# 2d pulse heights
print PLOT qq|
    set title "ADC pulse heights"
    unset grid
    set xlabel "X"
    set ylabel "Y"
    set autoscale
    set xrange [0.5:8.5]
    set yrange [0.5:8.5]
    set pm3d at b corners2color c1
    set view map
    set style data pm3d
    set output "${dir}/mean_2d.png"
    splot "${dir}/summary_2d.csv" using 1:2:3 title ""
    unset pm3d
    set grid
|;

# 2d all in one
print PLOT qq|
    set title "Histograms"
    unset grid
    set xlabel "Channel #"
    set ylabel "X"
    set autoscale
    set xrange [0.5:64.5]
    set yrange [$min:$max]
    set pm3d at b corners2color c1
    set view map
    set style data pm3d
    set output "${dir}/hist_2d.png"
    splot "${dir}/hist_2d.csv" using 1:2:3 title ""
    unset pm3d
    set grid
|;

# histograms - 8x8 individual channels
print PLOT qq|
    set output "${dir}/all_in_8x8.png"
    unset grid
    unset title
    unset xlabel
    unset ylabel
    unset key
    set autoscale
    set yrange [0:$max_c]
    set noxtics
    set noytics
    set format x ""
    set format y ""
    set multiplot layout 8,8
|;
for $i (0..63) {
    my $plus2 = $i + 2;
    print PLOT qq|
        plot "${dir}/hist.csv" using 1:$plus2 with histeps, \\
             "${dir}/raw.csv" index $i with impulses
    |;
}
print PLOT "unset multiplot\n";

close PLOT or die $!;

system("/usr/bin/gnuplot plot.gp") == 0 or die "Error running gnuplot: $?";

}

##################################################################################################


sub generatesummaryplot {

$DataFolder = shift;
$run = shift;
my @mods;
my %ratemap;
my @rateavg;

# This loop pipes the output of an ls shell command into IN,
# captures the module number using a regular expression, and
# puts it in the mods array. 
my $i = 0;
open IN, "ls ${DataFolder}/Run_${run}/PMT_* |" or die $!;
while(<IN>) {
    if($_ =~ /PMT_(\d+)/) {
	$mods[$i] = $1;
	$i++;
    }
}
close IN or die $!;

# This loop opens up the summary.csv files for each of the modules
# found in the above loop and splits each line on the comma, and
# then grabs the 1st and 3rd columns which correspond to channel
# number and hits per channel and puts them into a hash of a hash
foreach my $j (@mods) {
    open IN, "${DataFolder}/Run_${run}/PMT_$j/summary.csv" or die $!;
    while(<IN>) {
	next if($_ =~ /^#/);
	my @line = split(/,/); 
	$ratemap{$line[0]}{$j} = $line[3]; # 3: hits_pc, 8: gain_const
	$rateavg[$line[0]] += $line[3]; # 3: hits_pc, 8: gain_const
    }
    close IN or die $!;
}

# This final loop calculates the per channel averages
# by first dividing each channels rateavg array by the
# number of modules and then divides each channel in
# ratemap by the average of that channel over all the modules 
for(my $i = 1; $i <= 64; $i++) {

    $rateavg[$i] /= keys %{$ratemap{$i}};

    while(my ($key, $value) = each %{$ratemap{$i}}) {
	$ratemap{$i}{$key} /= $rateavg[$i];
    }
}

# This loop writes the rate_summary.csv file based on ratemap
open OUT, ">${DataFolder}/Run_${run}/rate_summary.csv" or die $!;
for(my $i = 1; $i <= 64; $i++) {
    print OUT "$i," . join(",",values %{$ratemap{$i}}) . "\n";
}
close OUT or die $!;

# This section creates a text file, plot.gp which is a macro
# for gnuplot to use in plotting. There should be a section for 
# each module.
open PLOT, ">${DataFolder}/Run_${run}/plot.gp" or die $!;
print PLOT qq|
    set grid
    set datafile separator ","
    set term png size 1024,768
|;
my $k = 1;
foreach my $j (@mods) {
    $k++;
    print PLOT qq|

    set title "PMT ${j} Hits by Channel Normalized by Channel Average"
    set xlabel "Channel #"
    set ylabel "Count"
    set autoscale
    set xrange [0.5:64.5]
    set yrange [0:*]
    set output "${DataFolder}/Run_${run}/mod_${j}_avg_norm_rate.png"
    plot "${DataFolder}/Run_${run}/rate_summary.csv" using 1:${k} title "Hits" with histeps

|;
}
close PLOT or die $!;

system("/usr/bin/gnuplot ${DataFolder}/Run_${run}/plot.gp") == 0 or die "Error running gnuplot: $?";

}
################################################################
##LG2018
#############################################################################################
=pod
sub generatecsv2 {

$usb = shift;
$pmt = shift;

DCFOV::getpmtdata $usb, $pmt;

# print "Processing data for USB=$usb and PMT=$pmt\n";
# print "\n";
$full_path = "/home/camillo/readout/DATA/Run_${run}";

my $full_path_temp = "$full_path/USB_${usb}/PMT_${pmt}";

dir $full_path_temp;                              # set usb to (create and) write to this folder

$baselines = "$full_path/USB_${usb}/PMT_${pmt}/baselines.dat";
$filename = "$full_path/USB_${usb}/signal";

for(my $i=0;$i<$newrunlength;$i++){
    system qq|cmd /C decode.pl "${filename}_${i}.bin" >> "${filename}.dec"|;
}

my $temp = "$filename.dec";
my $temp1 = "$full_path/USB_${usb}/PMT_${pmt}/signal.dec";

system qq|cmd /C move "$temp" "$temp1"|;

signal (${full_path_temp},$temp1,$baselines,$pmt);
#system qq|cmd /C signal.pl "${full_path_temp}" "$temp1" "$baselines" "@gain" "$pmt" ${homedir} >> log_${pmtserialnumber}_${date}.txt|;

}


#######################################################################################################################



sub signal2 {

my $nbins = 100;        # number of bins in histograms
my $gain_c = 1.45e4;    # constant used in gain calculation

my $dir = shift;
my $file = shift;
my $baselines = shift;

my $pmt_board = shift;
#my $laserrate = shift;
#my $homedir = shift;
my $homedir;

my $stefan = "Stefan Simion's Empty Histogram (tm). Formerly known as channel %d.";

my($x, $f, $i, $j);
my(@sig, @sig_dev, @base, @base_dev, @hist, @hist2, @hits_pc, @bins, @title, @photo, @gain_pmt, @mean_norm, @rate_norm);
for $i (1..64) {
    $sig[$i] = 0;
    $sig_dev[$i] = 0;
    $base[$i] = 0;
    $base_dev[$i] = 0;
    $hits_pc[$i] = 0;
    $photo[$i] = 0;
    $gain_pmt[$i] = 0;
    $hist[0]{$i} = 0;
        $mean_norm[$i] = 0;
        $rate_norm[$i] = 0;
}

#get "typical module" data
print "Getting data for typical module...\n";

if(defined($homedir)) {
    open IN, "${homedir}/macro/normalized.txt" or goto nohomedir;
    while(<IN>) {
        chomp;
        next if /^\s*$/;
                next if /^#/;
        my($i, $m, $d) = split ",";
        $mean_norm[$i] = $m;
        $rate_norm[$i] = $d;
    }
    close IN or die $!;
}

nohomedir:

# get baselines
print "Getting baselines...\n";

if(defined $baselines) {
    open IN, $baselines or die $!;
    while(<IN>){
        chomp;
        next if /^\s*$/;
        my($i, $m, $d) = split ",";
        $base[$i] = $m;
        $base_dev[$i] = $d;
    }
    close IN or die $!;
}


# parse data file

print "Getting signal file...$file\n";
# parse data file
open IN, "/usr/bin/perl ${dcmacropath}/unpacksignal.pl \"$file\" \"$pmt_board\" |" or die $!;
my ($first, $last, $count, $countovf);
$count = 0;
$last = 0;
$first = 0;
$countovf = 0;


while(<IN>) {
    chomp;
    my ($c, $d, $t) = split ",";
    if($count == 0) {
        $first = $t;
        $count++;
    }
        if($t < $last){
          $countovf++;
        }
    $last = $t;
    if($c == 0) {
        $hist[$c]{$d}++;            # $hist[0] = hits per packet  
    } else {
        $d -= $base[$c];
        $hist[$c]{$d}++;
        $hits_pc[$c]++;
    }
}

my $rate;
if($sync eq 'on') { $rate = 1/$runlength if $runlength; }
else { #only works if sync pulse is OFF!
#print "number of ovf $countovf\n";
$countovf = $countovf * 2**32;
##print "number of ovl after multiply it: $countovf \n";
$rate = ($last + $countovf - $first) * 16e-9;
##print "rate=${rate}, last packet = ${last}, first packet =${first}\n";
$rate = 1 / $rate if $rate;
}
#if(${rate} < 0) {${rate} = -${rate};}
close IN or die $!;

#$dir = "$dir/$pmt_board";

# calculate
print "Calculating...\n";
my($min, $max, $max_c) = (10000, -10000, -10000);
for $i (1..64) {
    my($avg, $dev) = (0, 0);
    my $n = $hits_pc[$i];
    next if $n == 0;
    while(($x, $f) = each(%{$hist[$i]})) {
        $avg += $f * $x;
        $min = $x if $x < $min;
        $max = $x if $x > $max;
    }
    $avg = $avg / $n;
    while(($x, $f) = each(%{$hist[$i]})) { $dev += $f * (($x - $avg) ** 2); }
    $dev = sqrt($dev / $n);
    $sig[$i] = $avg;
    $sig_dev[$i] = $dev;
    if($sig_dev[$i]) { $photo[$i] = ($sig[$i] / $sig_dev[$i]) ** 2; }
    if($photo[$i]) { $gain_pmt[$i] = $gain_c * $sig[$i] / $photo[$i]; }
}

# histogram hits
print "Histogramming...\n";
my $wbins = ($max - $min) / $nbins;
$wbins = 1 if $wbins == 0;
$min -= 0.0001 * $wbins;
$max += 0.0001 * $wbins;
#$min = -20; $max = 180; $nbins = 200;    ###################################
$wbins = ($max - $min) / $nbins;
for (0..$nbins-1) { push @{$hist2[0]}, $min + ($_ + 0.5) * $wbins; }
for $i (1..64) {
    for (0..$nbins-1) { $bins[$_] = 0; }
    while(($x, $f) = each(%{$hist[$i]})) { $bins[int(($x - $min) / $wbins)] += $f; }
    if($hits_pc[$i] == 0) {
        $bins[0] += 0.00001;
        $title[$i] = sprintf($stefan, $i);
    } else {
        $title[$i] = sprintf("Channel %02d    Mean = %.3f    Std Dev = %.3f    Total = %d",
            $i, $sig[$i], $sig_dev[$i], $hits_pc[$i]);
    }
    for (0..$nbins-1) {
        push @{$hist2[$i]}, $bins[$_];
        $max_c = $bins[$_] if $bins[$_] > $max_c;
    }
}


# raw data file
open OUT, ">$dir/raw.csv" or die $!;
for $i (1..64) {
    print OUT "# Channel $i\n";
    if($hits_pc[$i] == 0) { print OUT "$min,0\n$max,0\n"; }
    else { while(($x, $f) = each(%{$hist[$i]})) { print OUT "$x,$f\n"; } }
    print OUT "\n\n";
}
close OUT or die $!;

# histogram file
open OUT, ">$dir/hist.csv" or die $!;
print OUT "#x,", (join ",", (1..64)), "\n";
for $j (0..$nbins-1) {
    print OUT $hist2[0][$j];
    for $i (1..64) { print OUT ",$hist2[$i][$j]"; }
    print OUT "\n";
}
close OUT or die $!;

# summary file
open OUT, ">$dir/summary.csv" or die $!;
print OUT "#channel,mean,dev,hits_per_channel,hits_per_packet,photo_e,gain_pmt,rate_per_channel,gain_constant,normalized_rate\n";
for $i (1..64) {
    my $tmp1 = $hits_pc[$i]*$rate;
        my $tmp2 = 0;
        if($rate_norm[$i]) { $tmp2 = $tmp1/$rate_norm[$i]; }
    print OUT "$i,$sig[$i],$sig_dev[$i],$hits_pc[$i],$hist[0]{$i},$photo[$i],$gain_pmt[$i],$tmp1,$gain[$i],$tmp2\n";
}
close OUT or die $!;


# 2d summary
open OUT, ">$dir/summary_2d.csv" or die $!;
print OUT "#x,y,mean,dev,hits_per_channel,hits_per_packet,photo_e,gain_pmt,norm_mean\n";
for $i (0..8) {
    for $j (0..8) {
        my $xx = $i + 0.5;
        my $yy = $j + 0.5;
        my $c = 8 * $i + $j + 1;
        if($c <= 64) {
                        my $tmp3 = 0;
                        if($mean_norm[$c]) { $tmp3 = $sig[$c]/$mean_norm[$c]; }
            print OUT "$xx,$yy,$sig[$c],$sig_dev[$c],$hits_pc[$c],$hist[0]{$c},$photo[$c],$gain_pmt[$c],$tmp3\n";
        } else {
            print OUT "$xx,$yy,0,0,0,0,0,0,0\n";
        }
    }
    print OUT "\n";
}
close OUT or die $!;

# 2d histograms
open OUT, ">$dir/hist_2d.csv" or die $!;
print OUT "#x,y,z\n";
for $i (0..64) {
    for $j (0..$nbins) {
        my $xx = $i + 0.5;
        my $yy = $min + ($j + 0.5) * $wbins;
        if(($i < 64) && ($j < $nbins)) {
            print OUT "$xx,$yy,$hist2[$i + 1][$j]\n";
        } else {
            print OUT "$xx,$yy,0\n";
        }
    }
    print OUT "\n";
}
close OUT or die $!;
           
# change min and max so we can use them as ranges
$min += 0.5 * $wbins;
$max -= 0.5 * $wbins;

# generate plots
$dir =~ s///g;
print "Plotting...\n";
open PLOT, ">plot.gp" or die $!;
print PLOT qq|
    set grid
    set datafile separator ","
    set term png size 1024,768
|;


# summary
print PLOT qq|
    set title "ADC pulse heights"
    set xlabel "Channel #"
    set ylabel "ADC counts"
    set autoscale
    set xrange [0.5:64.5]
    set yrange [$min:*]
    set output "${dir}/mean.png"
    plot "${dir}/summary.csv" using 1:2 title "Mean" with histeps

    set title "ADC sigma"
    set xlabel "Channel #"
    set ylabel "ADC counts"
    set autoscale
    set xrange [0.5:64.5]
    set yrange [0:*]
    set output "${dir}/sigma.png"
    plot "${dir}/summary.csv" using 1:3 title "Sigma" with histeps

    set title "Hits by channel"
    set xlabel "Channel #"
    set ylabel "Count"
    set autoscale
    set xrange [0.5:64.5]
    set yrange [0:*]
    set output "${dir}/per_channel.png"
    plot "${dir}/summary.csv" using 1:4 title "Hits" with histeps

    set title "Hits in ADC packet"
    set xlabel "Number"
    set ylabel "Count"
    set autoscale
    set xrange [0.5:64.5]
    set yrange [0:*]
    set output \"${dir}/per_packet.png\"
    plot "${dir}/summary.csv" using 1:5 title "Hits" with histeps

    set title "Photoelectrons"
    set xlabel "Channel #"
    set ylabel "Number"
    set autoscale
    set xrange [0.5:64.5]
    set yrange [0:*]
    set output "${dir}/photo_e.png"
    plot "${dir}/summary.csv" using 1:6 title "Photoelectrons" with histeps

    set title "Gain"
    set xlabel "Channel #"
    set ylabel "Number"
    set autoscale
    set xrange [0.5:64.5]
    set yrange [0:*]
    set output "${dir}/gain.png"
    plot "${dir}/summary.csv" using 1:7 title "Gain" with histeps

    set title "Data acquisition rate per channel"
    set xlabel "Channel #"
    set ylabel "Rate [Hz]"
    set autoscale
    set xrange [0.5:64.5]
    set yrange [0:*]
    set output "${dir}/rate_per_channel.png"
    plot "${dir}/summary.csv" using 1:8 title "Rate (Hz)" with histeps

    set title "Maroc Gain Constants"
    set xlabel "Channel #"
    set ylabel "Number"
    set autoscale
    set xrange [0.5:64.5]
    set yrange [0:42]
    set output "${dir}/gainconst.png"
    plot "${dir}/summary.csv" using 1:9 title "Maroc Gain Constants" with histeps

    set title "Data acquisition rate per channel (normalized)"
    set xlabel "Channel #"
    set ylabel "Normalized Rate"
    set autoscale
    set xrange [0.5:64.5]
    set yrange [0:*]
    set output "${dir}/rate_per_channel_normalized.png"
    plot "${dir}/summary.csv" using 1:10 title "Normalized Rate" with histeps

|;



# histograms - individual channels
for $i (1..64) {
    my $minus1 = $i - 1;
    my $plus1 = $i + 1;
    my $tmp = ($i < 10) ? "0$i" : $i;
    print PLOT qq|
        set title "$title[$i]"
        set autoscale
        set yrange [0:$max_c]
        set xlabel "ADC counts"
        set ylabel "Count"
        set output "${dir}/adc_$tmp.png"
        plot "${dir}/hist.csv" using 1:$plus1 title "Histogram" with histeps, \\
             "${dir}/raw.csv" index $minus1 title "Raw data" with impulses
    |;
}


# histograms - all in one
print PLOT qq|
    set title "All channels"
    set xlabel "ADC counts"
    set ylabel "Count"
    set output "${dir}/all_in_one.png"
    set autoscale
    set yrange [0:$max_c]
    set key spacing 0.5
    plot \\|;
for $i (1..64) {
    my $plus1 = $i + 1;
    print PLOT qq|
        "${dir}/hist.csv" using 1:$plus1 title "Channel $i" with histeps|;
    if($i < 64) { print PLOT ", \\"; }
}
print PLOT "\n";

# 2d pulse heights
print PLOT qq|
    set title "ADC pulse heights"
    unset grid
    set xlabel "X"
    set ylabel "Y"
    set autoscale
    set xrange [0.5:8.5]
    set yrange [0.5:8.5]
    set pm3d at b corners2color c1
    set view map
    set style data pm3d
    set output "${dir}/mean_2d.png"
    splot "${dir}/summary_2d.csv" using 1:2:3 title ""
    unset pm3d
    set grid
|;

# 2d pulse heights normalized
print PLOT qq|
    set title "Normalized ADC pulse heights"
    unset grid
    set xlabel "X"
    set ylabel "Y"
    set autoscale
    set xrange [0.5:8.5]
    set yrange [0.5:8.5]
    set pm3d at b corners2color c1
    set view map
    set style data pm3d
    set output "${dir}/mean_normalized_2d.png"
    splot "${dir}/summary_2d.csv" using 1:2:9 title ""
    unset pm3d
    set grid
|;

# 2d all in one
print PLOT qq|
    set title "Histograms"
    unset grid
    set xlabel "Channel #"
    set ylabel "X"
    set autoscale
    set xrange [0.5:64.5]
    set yrange [$min:$max]
    set pm3d at b corners2color c1
    set view map
    set style data pm3d
    set output "${dir}/hist_2d.png"
    splot "${dir}/hist_2d.csv" using 1:2:3 title ""
    unset pm3d
    set grid
|;

# histograms - 8x8 individual channels
print PLOT qq|
    set output "${dir}/all_in_8x8.png"
    unset grid
    unset title
    unset xlabel
    unset ylabel
    unset key
    set autoscale
    set yrange [0:$max_c]
    set noxtics
    set noytics
    set format x ""
    set format y ""
    set multiplot layout 8,8
|;
for $i (0..63) {
    my $plus2 = $i + 2;
    print PLOT qq|
        plot "${dir}/hist.csv" using 1:$plus2 with histeps, \\
             "${dir}/raw.csv" index $i with impulses
    |;
}
print PLOT "unset multiplot\n";

close PLOT or die $!;
system("wgnuplot.exe plot.gp") == 0 or die "Error running gnuplot: $?";

}

=cut










1;
