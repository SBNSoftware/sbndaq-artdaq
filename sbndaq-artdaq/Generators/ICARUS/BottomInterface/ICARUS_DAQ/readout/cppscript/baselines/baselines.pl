use strict;
use warnings;

my $dir = shift;
my $file = shift;
my $pmt_board = shift;
my $min = 1000;

#my $dcmacropath = $ENV{DCONLINE_PATH} . "/DCOV/readout/macro";
my $dcmacropath = "/e/h.0/localdev/readout/cppscript/unpackbaseline";

my @hist;         
open IN, "/usr/bin/perl ${dcmacropath}/unpackbaseline.pl \"$dir/$file\" \"$pmt_board\" |" or die $!;
while(<IN>) {
    chomp;
    my ($c, $d) = split ",";
    $hist[$c]{$d}++;
}
close IN;

open OUT, ">$dir/baselines.dat" or die $!;
for my $i (1..64) {
    my($x, $f);
    my($n, $dev, $avg) = (0, 0, 0);
    while(($x, $f) = each(%{$hist[$i]})) { $avg += $f * $x; $n += $f; }
    if($n) {
        $avg = $avg / $n;
        while(($x, $f) = each(%{$hist[$i]})) { $dev += $f * (($x - $avg) ** 2); }
        $dev = sqrt($dev / $n);
    }
    print OUT "$i,$avg,$dev,$n\n";
    $min = $n;
}
close OUT;
print "PMT ${pmt_board}:\t hits: ${min}\n";
