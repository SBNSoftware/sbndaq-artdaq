use strict;
use warnings;

my $dir = shift;
my $file = shift;
my $col = shift;
my $name = shift;

my(@data, @bins);

# read input file
open IN, "$dir/$file" or die $!;
while(<IN>) {
    chomp;
    s/\s*#.*//;
    next if /^\s*$/;
    my @line = split /\s*,\s*/;
    my $d = $line[$col];
    push(@data, $d);
}
close IN;

# generate histogram
my $n = @data;
exit 0 if !$n;
my($avg, $dev) = (0, 0);
open OUT, ">$dir/histogram.dat" or die $!;
if($n < 3) {
    print OUT "0,0\n0.5,1\n1,0\n";
} else {
    my($min, $max) = ($data[0], $data[0]);
    for(@data) {
        $min = $_ if $_ < $min;
        $max = $_ if $_ > $max;
        $avg += $_;
    }
    $avg = $avg / $n;
    for(@data) { $dev += ($_ - $avg) ** 2; }
    print "dev = $dev\n";
    $dev = sqrt($dev / $n);
    my $nbins = 50;
    my $wbins = ($max - $min) / $nbins;
    if($wbins == 0) { $wbins = 1; }
    $min -= 0.0001 * $wbins;
    $max += 0.0001 * $wbins;
    $wbins = ($max - $min) / $nbins;
    for(0..$nbins-1) { $bins[$_] = 0 };
    for(@data) { $bins[int(($_ - $min) / $wbins)]++; }
    $min += 0.5 * $wbins;
    for (0..$nbins-1) { print OUT "$min,$bins[$_]\n"; $min += $wbins; }
}
close OUT;

my $per = 0;
if($avg) { $per = int(0.5 + 100 * $dev / $avg); }
print "$per\n";
my $title = sprintf("$name    Mean = %.3f    Std Dev = %.3f (%.2f %%)    Total = %d", 
                    $avg, $dev, $per, $n);

open PLOT, ">$dir/plot.gp" or die $!;
print PLOT qq|
    set grid
    set datafile separator ","
    set term png size 1024,768
    set title "${title}"
    set xlabel "${name}"
    set ylabel "Count"
    set autoscale
|;
if($title =~ /Gains/) { print PLOT "set xrange [0:1000000]\n"; }
print PLOT qq|
    set yrange [0:*]
    set output "${dir}/${name}.png"
    plot "histogram.dat" using 1:2 title "Histogram" with histeps
|;
close PLOT;

system("/usr/bin/gnuplot plot.gp") == 0 or die "Error running gnuplot: $?";
