use strict;
use warnings;

my $dir = shift;

my $constfile = shift;

#########################
# adjust gain to be within the range ideal +- delta
my $ideal = shift; # pulse height generally 400
my $delta = 1;   # generally 5%
#########################

#print "directory in which we found summary.csv : $dir\n";

my(@gains, @line, @settings);

open IN, "$dir\\summary.csv" or die $!;
while(<IN>) {
    chomp;
    s/#.*//;
    next if /^\s*$/;
    @line = split ',';
    $gains[$line[0]] = $line[1];  #read gain value for adjustment [6],pulse height [1]
}
close IN or die $!;

open IN, $constfile or die $!;
while(<IN>) {
    chomp;
    s/#.*//;
    next if /^\s*$/;
    @line = split '\s+';
    $settings[$line[0]] = $line[1];
}
close IN or die $!;

open OUT, ">$constfile" or die $!;
for my $channel (1..64) {
    my $setting = $settings[$channel];
    if(abs($gains[$channel] - $ideal) > $delta) {
        my $t = 16;
        if($gains[$channel]) { 
            $t = ($ideal / $gains[$channel]) * $setting;
        }
        $setting = ($setting + 3 * $t) / 4;
        $setting = int($setting + 0.5);
    }
    #print "gains: $gains[$channel], setting: $setting \n";
    
    if($setting < 4) { $setting = 4; }
    if($setting > 40) { $setting = 40; }

    print OUT "$channel\t$setting\n";
}
close OUT;
