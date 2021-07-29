package usb_readout;
use strict;
use warnings;

# exporter
our (@ISA, @EXPORT, @EXPORT_OK, %EXPORT_TAGS, $VERSION);
use Exporter;
$VERSION = 1.00;
@ISA = qw(Exporter);
@EXPORT = qw(init_readout Var dir new shut daemonize im_alive db_connect set_run_number set_inhibit
            latency send_out com usb tarry dac fake_adc fake_trigger
            get_status send_token set_data_disk set_data_path
            take_baselines
            flag process com_usb set_inhibit_usb dac_usb usb_usb com_usb_1
            );
#   dir new shut com data usb dac delay tarry);
@EXPORT_OK = ();
%EXPORT_TAGS = ();

# libraries
use IO::Socket;
use Time::HiRes;
use File::Path;
use IPC::SysV qw(IPC_NOWAIT IPC_PRIVATE IPC_CREAT S_IRUSR S_IWUSR);
use IPC::Msg;
use DBI;
use POSIX qw(setsid);

my($Host_name, $Port_no, %Var);

sub init_readout { $Host_name = shift; $Port_no = 3306; }

# global variables
my (@MQs, $DBH);
my $Msg_Base = 10000;

# CM &MT
sub dir {
    my $dir = shift;
    mkpath $dir;
    #chmod 0777,$dir;
}


sub new {
    my($usb, $file) = @_;
    my $msg = new IPC::Msg($Msg_Base + $usb, 0666 | IPC_CREAT);
    $msg->snd(3, $file);
}

=junk
sub new {
    my($usb, $file) = @_;
    send_out("file,$usb,$file\n");
}

sub send_out {
    my $to_send = shift;
    return unless defined $to_send;
    my $socket = IO::Socket::INET->new(PeerAddr => $Host_name,
    				       PeerPort => $Port_no,
				       Proto    => "tcp",
				       Type     => SOCK_STREAM)
    	or die "Couldn`t connect to $Host_name:$Port_no: $@\n";
    while(<$socket>) {
        s/[\r\n]//g;
        last if /^\s*$/;
        if(/([^,]*),([^,]*)/) { $Var{$1} = $2; }
    }
    print $socket $to_send;
    close($socket);
}
=cut


sub var { $Var{$_[0]}; }

sub shut {
    my $usb = shift; 
    new $usb, "";
}

# daemonize $stdout_log, $stderr_log;
sub daemonize {
    my($out_log, $err_log) = @_;

    defined(my $pid = fork) or die "Couldn't fork: $!";
    exit if $pid;
    setsid or die "Couldn't start a new session: $!";
    umask 0;
    open STDIN, '/dev/null' or die $!;
    if(defined $out_log) { open STDOUT, '>', $out_log or die $!; }
    else                 { open STDOUT, '>/dev/null'   or die $!; }
    if(defined $err_log) { open STDERR, '>>', $err_log or die $!; }
    else                 { open STDOUT, '>/dev/null'   or die $!; }
    #chdir '/' or die $!;
}

# connect to db
sub db_connect {
    $DBH = DBI->connect_cached(
        'dbi:mysql:double_chooz:localhost:3306',
        'daq',  # user name
        '2ch',  # password
        { AutoCommit => 1, RaiseError => 1, PrintError => 1 }
    );
    return $DBH;
}

# find out which message queues exist
sub find_mqs {
    @MQs = ();

    open IN, "ipcs -q |" or die $!;
    while(<IN>) {
        if(/^([\dxA-Fa-f]+)\s+/) {
            my $tmp = hex($1);
            if($tmp >= $Msg_Base) {
                my $msg = new IPC::Msg($tmp, 0666 | IPC_CREAT);
                push(@MQs, [$msg, $tmp]);
            }
        }
    }
}

# mq_send $msg_id, $data;
sub mq_send {
    my ($msg, $data) = @_; my $mq; find_mqs unless @MQs;

    while(1) {
        eval {
            local $SIG{ALRM} = sub { die "mq timeout"; };
            alarm 1;
            eval { for $mq (@MQs) { $mq->[0]->snd($msg, $data); } };
            alarm 0;
        };
        alarm 0;
        return unless $@;               # success
        die if $@ !~ /mq timeout/;      # reraise other exceptions
        warn "Killing MQ $mq->[1]\n";
        system "ipcrm -Q $mq->[1]";     # kill blocking mq
        find_mqs;                       # remove it from MQs
    }
}

# mq_send_usb $msg_id, $usb_num_$data;
sub mq_send_usb {
    my ($msg, $usb_num, $data) = @_; my $mq; find_mqs unless @MQs;
    $usb_num += $Msg_Base;
    while(1) {
        eval {
            local $SIG{ALRM} = sub { die "mq timeout"; };
            alarm 1;
            eval { 
		for $mq (@MQs) { 
		    if($mq->[1] eq $usb_num) { $mq->[0]->snd($msg, $data); } 
		} 
	    };
            alarm 0;
        };
        alarm 0;
        return unless $@;               # success
        die if $@ !~ /mq timeout/;      # reraise other exceptions
        warn "Killing MQ $mq->[1]\n";
        system "ipcrm -Q $mq->[1]";     # kill blocking mq
        find_mqs;                       # remove it from MQs
    }
}

# set data disk
sub set_data_disk { mq_send 6, pack('I', $_[0]) if @_; }

# set run number
sub set_run_number { 
    my $run_no = shift;
    mq_send 5, pack('a100', "$run_no") if $run_no; 
}

# set data path
sub set_data_path { 
    my $data_path = shift;
    mq_send 7, pack('a100', "$data_path") if $data_path; 
}

# latency $number_of_seconds_to_write_to_file;
sub set_inhibit { mq_send 4, pack('I', $_[0]) if @_; }

# latency $number_of_seconds_to_write_to_file;
sub set_inhibit_usb { 
    my ($usb, @usbdata) = @_;
    mq_send_usb 4, $usb, pack('I', $usbdata[0]) if @usbdata; 
}

# latency $number_of_seconds_to_write_to_file;
sub latency { mq_send 2, pack('I', $_[0]) if @_; }


# send_out (array of 24 bit words);
sub send_out { mq_send 1, pack('I*', @_) if @_; }


# send_out (array of 24 bit words);
sub send_out_usb { 
    my ($usb, @usbdata) = @_;
    mq_send_usb 1, $usb, pack('I*', @usbdata) if @usbdata;
}

# com $pmt, $com, $data;
sub com {
    my($pmt, $b2, $b3) = @_;
    send_out(0x400000 + (($pmt & 63) * 256 + $b2) * 256 + $b3);
}

# com $pmt, $com, $data;
sub com_usb {
    my($usb, $pmt, $b2, $b3) = @_;
    send_out_usb($usb,0x400000 + (($pmt & 63) * 256 + $b2) * 256 + $b3);
}

sub com_usb_1{
    my($usb, $pmt, $b2, $b3) = @_;
    my $b1 = ($pmt & 0x3f) | 0x40;
    send_out("send, $usb, $b1, $b2, $b3\n");
}

# usb $adr_8, $data_16;
sub usb {
    my($adr, $data) = @_;
    send_out(($adr << 16) + ($data & 0xffff));
}

# usb $adr_8, $data_16;
sub usb_usb {
    my($usb, $adr, $data) = @_;
    send_out_usb($usb, ($adr << 16) + ($data & 0xffff));
}

# flag $type_16;
sub flag {
    my $flag = shift;
    send_out(0xca0000 | ($flag & 0xffff));
}

# process $type_16;
sub process {
    my $flag = shift;
    send_out(0xcb0000 | ($flag & 0xffff));
}

# send token manually;
sub send_token {
    send_out(0x800000);    
}


# tarry $number_of_seconds_to_wait;
sub tarry { Time::HiRes::sleep($_[0]); }

# dac $pmt_6, $data_12;
sub dac {
    my($pmt, $data) = @_;
    my $orig = $data;
    com($pmt, 64, $data & 0xff);
    $data >>= 8;
    $data = (1 << ($data & 0x1f)) - 1;
    com($pmt, 65, $data & 0xff);
    com($pmt, 66, ($data >> 8) & 0xff);
}
# dac $pmt_6, $data_12;
sub dac_usb {
    my($usb,$pmt,$data) = @_;
    my $orig = $data;
    com_usb($usb,$pmt, 64, $data & 0xff);
    $data >>= 8;
    $data = (1 << ($data & 0x1f)) - 1;
    com_usb($usb,$pmt, 65, $data & 0xff);
    com_usb($usb,$pmt, 66, ($data >> 8) & 0xff);
}

# fake_trigger [module];
sub fake_trigger {
    my $mod = shift || int(rand(128));
    my @res;
    $#res = 8;
    $res[0] = 0xc0ffff;
    $res[1] = 0xc00000 + ($mod & 127) * 256 + 8;
    for (2..7) { $res[$_] = 0xc00000 + int(rand(65536)); }
    for (1..7) { $res[8] ^= $res[$_]; }
    $res[8] |= 0xc00000;
    send_out @res;
}

# fake_adc [module], [hits], [avg], [sigma];
sub fake_adc {
    my $mod   = shift || int(rand(128));
    my $hits  = shift || int(rand(64)) + 1;
    my $avg   = shift || 1600;
    my $sigma = shift || 100;

    my $last = $hits * 2 + 4;
    my @res;
    $#res = $last;
    $res[0] = 0xc0ffff;
    $res[1] = 0xc08000 + ($mod & 127) * 256 + $hits * 2 + 4;
    for (2..3) { $res[$_] = 0xc00000 + int(rand(65536)); }
    for (1..$hits) {
        $res[$_ * 2 + 2] = 0xc00000 + (int($avg - $sigma + rand(2 * $sigma)) & 0xfff);
        $res[$_ * 2 + 3] = 0xc00000 + $_ - 1;
    }
    for (1..($last - 1)) { $res[$last] ^= $res[$_]; }
    $res[$last] |= 0xc00000;
    if(@res > 128) {
        send_out @res[0..127];
        send_out @res[128..$#res];
    } else { 
        send_out @res;
    }
}

# im_alive $who_am_i, $process_id, $status;
sub im_alive { }
sub im_alive2 {
    my $name   = shift || "???";
    my $pid    = shift || $$;
    my $status = shift || "OK";
    eval {
        db_connect;
        $DBH->do("replace processes (time, name, status, pid)
                    values (?, ?, ?, ?)",
                    undef, time, $name, $status, $pid);
        $DBH->do("delete from processes where time < ?",
                    undef, time - 10);
    }; warn $@ if $@;
}

# get status info from message queue
sub get_status {
    my($buf, $len);
    my $msg = new IPC::Msg($Msg_Base - 1, 0666 | IPC_CREAT);
    my @res = ();

    while($len = $msg->rcv($buf, 256, 0, IPC_NOWAIT)) { push @res, $buf; }
    return \@res;
}

# take_baselines [$number_of_samples];
sub take_baselines {
    my $n = shift || 1000;
    com 63, 73, 0b00110;
    flag 1;
    for (my $i = 0; $i < $n; $i++) {
        com 63, 81, 0;
        fake_adc 2, 64, 1550, 2;
        fake_adc 3, 64, 1550, 2;
    }
    flag 0;
    process 1;


    # restore configuration


}




=sql






DELIMITER $$
drop procedure if exists process$$
CREATE PROCEDURE double_chooz.process_it (f INT, u INT)
BEGIN
    if f = 0 then
        select * from baselines;

    elseif f = 1 then



    elseif f > 1 and f < 16 then



    elseif f >= 16 then



    end if;
    call delete_data(f, u);
END$$
DELIMITER ;


sub call_proc {
       eval {
        db_connect;
        $DBH->do("replace processes (time, name, status, pid)
                    values (?, ?, ?, ?)",
                    undef, time, $name, $status, $pid);
        $DBH->do("delete from processes where time < ?",
                    undef, time - 10);
    }; warn $@ if $@;
    
}









baselines:

# insert
insert into double_chooz.baselines (adr, ch, mean, sigma, count)
select adr, ch, avg(adc), std(adc), count(adc)
from double_chooz.run_data1
where flag = 1
group by adr, ch

# update baselines
update
double_chooz.run_data1 r,
double_chooz.baselines b1,
(
    select adr, ch, max(time) as t
    from double_chooz.baselines
    group by adr, ch
) as b2
set r.base=b1.mean
where
(r.base is null) and
( r.adr = b1.adr) and ( r.ch = b1.ch) and
(b1.adr = b2.adr) and (b1.ch = b2.ch) and
(b1.time = b2.t);






"update
double_chooz.run_data1 r,
double_chooz.baselines b1,
(
    select adr, ch, max(time) as t
    from double_chooz.baselines
    group by adr, ch
) as b2
set r.base=b1.mean
where
(r.base is null) and
( r.adr = b1.adr) and ( r.ch = b1.ch) and
(b1.adr = b2.adr) and (b1.ch = b2.ch) and
(b1.time = b2.t)"












# find latest baselines
select b1.mean
from double_chooz.baselines b1
join (
    select adr, ch, max(time) as t
    from double_chooz.baselines
    group by adr, ch
) as b2
on b1.adr = b2.adr and b1.ch = b2.ch and b1.time = b2.t


(select

)
where 

where () and exists (
select null
from double_chooz.baselines
where (r.base is null) and (r.adr = b1.adr) and (r.ch = b1.ch)



# update baselines:













# newest baselines view:
VIEW current_baselines AS select b1.adr AS adr,b1.ch AS ch,b1.mean AS mean from baselines b1 where (b1.time = (select max(b2.time) AS max(b2.time) from baselines b2 where ((b1.adr = b2.adr) and (b1.ch = b2.ch))))


































=junk

# send_out_usb $usb, (array of 24 bit words);
sub send_out_usb {
    return if @_ < 2;
    my $usb = shift;
    my $mq = new IPC::Msg($Msg_Base + $usb, 0666 | IPC_CREAT);
    $mq->snd(1, pack('I*', @_));
}


/

# create new run table

"create table if not exists $run_name select * from run_template
 engine = MyISAM default charset = latin1 max_rows = 4294967295"







# return list of alive processes
sub are_alive {
    my $res;
    open IN, "ps -eo cmd |" or die $!;
    while(<IN>) { $res .= $_; }
    return $res;
}






















sub is_alive {
    open(PS, "/bin/ps x|") || die "Can't Open PS";
while(<PS>) {
if (/usr\/sbin\/named/) { close PS; exit;}
}
close PS;
system("/usr/sbin/named");

}


use DBI;

print "Connecting to database...\n";
my $dbh = DBI->connect('dbi:mysql:double_chooz:localhost:3306',
                       'remote', '2ch', { RaiseError => 1, AutoCommit => 1 } );

print "Adding a row...\n";
$dbh->do(qq|REPLACE INTO processes SET pid=$$, name="bin2hex", status="OK"|);

print "Success!\n";
<STDIN>;


# im_alive $who_am_i;
sub im_alive {
    my $txt = shift;
    my $mq = new IPC::Msg($Msg_Base - 1, 0666 | IPC_CREAT);
    pid, txt, time
    $mq->snd(1, $txt);
}



#   new $usb, $file_to_write_to;
#   shut $usb;
# 
#   data $usb, $data_16;


#   delay $usb, $pmt_6, $data_8;







sub new {
    my($usb, $file) = @_;
    my $msg = new IPC::Msg($Msg_Base + $usb, 0666 | IPC_CREAT);
    $msg->snd(3, $file);
}
sub shut {
    my $usb = shift; 
    new $usb, "";
}



# new $usb, $file_to_write_to;
sub new {
    my($usb, $file) = @_;
    my $msg = new IPC::Msg($Msg_Base + $usb, 0666 | IPC_CREAT);
    $msg->snd(3, $file);
}

# shut $usb;
sub shut {
    my $usb = shift; 
    new $usb, "";
}

sub dir {
    my $dir = shift;
    mkpath $dir;
}





sub data {
    my($usb, $data) = @_;
    send_out($usb, 0xc00000 + ($data & 0xffff));
}















        socket(SERVER, PF_INET, SOCK_STREAM, getprotobyname('tcp'));
        my $ip_addr = inet_aton($Host_name) 
            or die "Couldn't convert $Host_name into an Internet address: $!\n";
        my $port_addr = sockaddr_in($Port_no, $ip_addr);
        #my $attempts = 3;
        #while($attempts) { connect(SERVER, $port_addr) or $attempts--; }
        #die "Couldn't connect to $Host_name:$Port_no: $!\n" if !$attempts;
        connect(SERVER, $port_addr) 
            or die "Couldn't connect to $Host_name:$Port_no: $!\n";
        SERVER->autoflush(1);
        while(<SERVER>) {
            s/[\r\n]//g;
            last if /^\s*$/;
            if(/([^,]*),([^,]*)/) { $Var{$1} = $2; }
        }
        print SERVER $to_send;
        my $error = <SERVER>;
        print "API error: $error\n" unless $error eq "\n";
        close(SERVER);
        #Time::HiRes::sleep(0.002);
    }

    my $to_send = shift;
    return unless defined $to_send;
    my $socket = IO::Socket::INET->new(PeerAddr => $Host_name,
                                       PeerPort => $Port_no,
                                       Proto    => "tcp",
                                       Type     => SOCK_STREAM)
        or die "Couldn't connect to $Host_name:$Port_no: $@\n";
    while(<$socket>) {
        s/[\r\n]//g;
        last if /^\s*$/;
        if(/([^,]*),([^,]*)/) { $Var{$1} = $2; }
    }
    print $socket $to_send;
    close($socket);







    my $file = shift;
    die "Expecting: init_readout \"file\"" unless defined($file);
    %Routing = ();
    @USBs = ();
    my %collision = ();
    open IN, $file or die $!;
    while(<IN>) {
        chomp;              # remove newline
        s/\s*#.*//;         # remove comments
        next if /^\s*$/;    # skip empty lines
        my($usb, @data) = split /\s*,\s*/;
        for my $pmt (@data) {
            $Routing{$pmt} = $usb;
            my $six_bits = $pmt & 63;
            if($six_bits == 63) {
                print "Can't have address 63\n";
                exit(1);
            }
            my $key = "$usb $six_bits";
            if(exists($collision{$key})) {
                my $tmp = $collision{$key};
                print "Fatal error: Bottom six bits collision: usb $usb, ";
                print "modules $pmt and $tmp\n";
                exit(1);
            } else {
                $collision{$key} = $pmt;
            }
        }
        my $got_it = 0;
        for (@USBs) { $got_it = 1 if $_ == $usb; }
        if (!$got_it) { push @USBs, $usb; }
    }
    close IN or die $!;





    my($usb, $mod);
    if($pmt == -1) {
        while(($pmt, $usb) = each(%Routing)) { com_int $usb, $pmt, $b2, $b3; }
    } elsif($pmt == 63) {
        for (@USBs) { com_int $_, 63, $b2, $b3; }
    } else {
        if(exists($Routing{$pmt})) {
            com_int $Routing{$pmt}, $pmt, $b2, $b3;
        } else {
            print "Fatal error: Module $pmt has no USB in routing table.\n";
            exit(1);
        }
    }



#send_out(0xc10000 + $pmt, 0xc20000 + $orig);



sub db_connect {
    my $dbh;
    my $crash = shift;
    $crash = 1 unless defined $crash;
again:
    $dbh = DBI->connect_cached($Database, $User, $Password,
            { AutoCommit => 1, RaiseError => $crash, PrintError => $crash } );
    if(!$dbh) { 
        warn "Cannot connect to DB: $DBI::errstr\n"; 
        sleep(1); 
        goto again; 
    }
    return $dbh;
}

    $DBH->do("create table processes
            (time int, pid int, name int, status int)");
    $DBH->do("create table usbs
            (time int, serial int, usb_rate int, disk_rate int, flags int)");

        #warn "Cannot connect to DB: $DBI::errstr\n";


sub create_tables {
    dbc_connect;

    $DBC->do("create table processes
            (time int, pid int, name int, status int)");
    $DBC->do("create table usbs
            (time int, serial int, usb_rate int, disk_rate int, flags int)");
}







    open IN, "ipcs -q |" or die $!;
    while(<IN>) {
        if(/^([\dxA-Fa-f]+)\s+/) {
            my $tmp = hex($1);
            if($tmp >= $Msg_Base) {
                my $msg = new IPC::Msg($tmp, 0666 | IPC_CREAT);
                push(@MQs, $msg);
            }
        }
    }
    if(0) {
        mkpath "./data";
        $Database = 'dbi:CSV:f_dir=./data';
        $User = '';
        $Password = '';
    } else {
        $Database = 'dbi:mysql:double_chooz:localhost:3306';
        $User = 'daq';
        $Password = '2ch';
    }
    create_tables();


# connect to control db (param = blocking)
sub dbc_connect {
    my $block = shift;
    $block = 1 unless defined $block;
    my $db = 'dbi:mysql:dc_control:localhost:3306';
    my $user = 'daq';
    my $password = '2ch';
    while(1) {
        eval {
            $DBC = DBI->connect_cached($db, $user, $password, \%Attr);
        };
        last unless ($@ && $block);
        sleep(10);
    }
}


# find out which message queues exist
sub find_mqs {
    @MQs = ();
    for (0..20) {
        my $msg = new IPC::Msg($Msg_Base + $_, 0666 | IPC_CREAT);
        push(@MQs, $msg) if defined $msg;
    }
}

    eval {
        dbc_connect;
        my $sth = $DBC->prepare_cached("select serial from usbs");
        $sth->execute();
        my $usb;
        $sth->bind_col(1, \$usb);
        while($sth->fetch) {
            my $msg = new IPC::Msg($Msg_Base + $usb, 0666 | IPC_CREAT);
            push(@MQs, $msg);
        }
    };
    return unless $@;

# set_param $pmt, $param_16;
sub set_param {
    my ($adr, $data) = @_;
    $adr = ($adr & 0xff) << 8;
    send_out(0xca0000 | $adr | (($data >> 8) & 0xff),
             0xcb0000 | $adr | ($data & 0xff));
}

# set_param_global $param_16;
sub set_param_global {
    my $data = shift;
    set_param 255, $data;
}

# set_type_global $type_8;
sub set_type_global {
    my $data = shift;
    set_type 255, $data;
}

#sub end_run {
#    my($adr, $data) = @_;
#    send_out(($adr << 16) + ($data & 0xffff));
#}


=cut

1;
