use strict;
use warnings;

my $dcmacropath = "/e/h.0/localdev/readout/cppscript/decode";

my $file = shift;
my $pmt_board = shift;
my $otime = 0;
my $delta = 0;

if($file =~ /\.csv/) { 
    # 64-column format
    while(<>) {
        chomp;
        next if /^\s*$/;
        my @line = split /\s*,\s*/;
        if($line[1]) {  # adc packet
            my $len = 0;
            for my $i (1..64) {
                my $d = $line[3 + $i];
                if($d) { print "$i,$d\n"; $len++; }
            }
            print "0,$len\n";
        }
    }
} else {
# } elsif($file =~ /\.hex/) {
    # # hex format
    # while(<>) {
        # chomp;
        # next if /^\s*$/;
        # my @line = split /\s*,\s*/;
        # my $tmp = hex($line[0]);
        # my $len = ($tmp & 0xff) - 4;
        # #my $pmt = ($tmp >> 8) & 0x7f;
        # my $type = $tmp >> 15;
        # #my $time = hex($line[1]);
        # #$time = ($time << 16) + hex($line[2]);
        # #my $unix = $line[$len + 3];
        # #my $usb = $line[$len + 4];
        # if($type) {     # adc packet
            # $len >>= 1;
            # for my $i (0..$len-1) {
                # my $data = hex($line[3 + 2 * $i]);
                # my $channel = hex($line[4 + 2 * $i]) + 1;
                # print "$channel,$data\n";
            # }
            # print "0,$len\n";
        # }
    # }
# } elsif($file =~ /\.bin/) {
    # # binary format \			"$file\" \"$pmt_board\" |" or die $!;
    open IN, "/usr/bin/perl ${dcmacropath}/decode.pl \"$file\" |" or die $!;
    while(<IN>) {
        chomp;
        next if /^\s*$/;
        my @line = split /\s*,\s*/;
        if($line[0] eq "p") {
            shift @line;
            my $tmp = $line[0];
            my $mod = ($tmp >> 8) & 0x7f;
            next if (defined($pmt_board) and ($mod != $pmt_board));
            my $len = ($tmp & 0xff) - 4;
            my $time = ($line[1] << 16) + $line[2];
            if($time < $otime) { $delta++; }
            $otime = $time;
            $time = ($delta << 32) + $time;
            my $type = $tmp >> 15;
            if($type) {     # adc packet
                $len >>= 1;
                for my $i (0..$len-1) {
                    my $data = $line[3 + 2 * $i];
                    my $channel = $line[4 + 2 * $i] + 1;
                    print "$channel,$data,$time\n";
                }
                print "0,$len,$time\n";
            }
        }
    }
}
