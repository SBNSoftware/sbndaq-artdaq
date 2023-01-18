use strict;
use warnings;

my $file = shift;
my $pmt_board = shift;
my $otime = 0;
my $delta = 0;

open IN, $file or die $!;
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
