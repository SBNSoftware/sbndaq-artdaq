use strict;
use warnings;

my $buf_size = 16 * 1024;
my $file = shift;
open IN, $file or die $!;
binmode(IN);


my $word;           # holds 24-bit word being built
my $exp = 0;        # expecting this type next
my $buffer;
my @bytes;

my @data;           # 11
my @extra;          # leftovers
my $words = 0;      # word counter

my $time_hi;
my $got_hi = 0;     # hee hee
my @word_count = (0, 0, 0, 0);
my $word_index = 0;

while(1) {
    my $len = read(IN, $buffer, $buf_size);
    die $! unless defined $len;
    last unless $len;
    @bytes = unpack('C*', $buffer);
    for (@bytes) {
        my $payload = $_ & 63;
        my $type = $_ >> 6;
        if($type == 0) {
            $exp = 1;
            $word = $payload;
        } else {
            if($type == $exp) {
                $word = ($word << 6) | $payload;
                if(++$exp == 4) {
                    $exp = 0;
                    got_word($word);
                }
            } else {
                $exp = 0;
            }
        }
    }
}
push @extra, @data;
flush_extra();
close IN or die $!;

sub got_word {
    my $d = shift;
    $words++;
    my $type = ($d >> 22) & 3;
    if($type == 1) {                    # command word
        my $b1 = ($d >> 16) & 63;
        my $b2 = ($d >> 8) & 255;
        my $b3 = $d & 255;
        flush_extra();
        print "c,$b1,$b2,$b3\n";
    } elsif($type == 3) {
        return if check_debug($d);
        push @data, $d & 65535;
        check_data();
    } else {
        printf "?,%06x\n", $d;
    }
}

sub check_data {
    while(1) {
        my $got_packet = 0;
        my $got = @data;
        last unless $got;
        if($data[0] == 0xffff) {            # check header
            last unless $got >= 2;
            my $len = $data[1] & 255;
            if($len > 1) {
                last unless $got >= $len + 1;
                my $par = 0;
                for(my $i = 1; $i <= $len; $i++) { $par ^= $data[$i]; }
                if(!$par) {                 # check parity
                    $got_packet = 1;
                    flush_extra();
                    print 'p,', join(',', @data[1..$len-1]), "\n";
                    splice @data, 0, $len + 1;
                }
            }
        }
        if(!$got_packet) { push @extra, shift @data;}
    }
}

# c1 dac pmt
# c2 dac
# c5 XXXX  -skipped
# c6 data_hi    - number of received data words
# c6 data_lo
# c6 lost_hi     - number of lost data words
# c6 lost_lo
# c8 timestamp_hi
# c9 timestamp_lo

sub check_debug {
    my $d = shift;
    my $a = ($d >> 16) & 255;
    $d = $d & 65535;

    if($a == 0xc8) {
        $time_hi = $d;
        $got_hi = 1;
        return 1;
    } elsif($a == 0xc9) {
        if($got_hi) {
            my $time = ($time_hi << 16) | $d;
            $got_hi = 0;
            flush_extra();
            print "t,$time\n";
        }
        return 1;
    } elsif($a == 0xc5) {
        $word_index = 0;
        return 1;
    } elsif($a == 0xc6) {
        $word_count[$word_index++] = $d;
        if($word_index == 4) {
            my $t = ($word_count[0] << 16) + $word_count[1];
            my $v = ($word_count[2] << 16) + $word_count[3];
            my $diff = $t - $v - $words;
            if($diff < 0) { $diff += (1 << 32); }
            flush_extra();
            print "n,$t,$v,$words,$diff\n";
        }
        return 1;
    } elsif($a == 0xc1) {
        flush_extra();
        print "dac,$d\n";
        return 1;
    } elsif($a == 0xc2) {
        flush_extra();
        print "delay,$d\n";
        return 1;
    } else {
        return 0;
    }
}

sub flush_extra {
    if(@extra) {
        print 'x,', join(',', @extra), "\n";
        @extra = ();
    }
}
