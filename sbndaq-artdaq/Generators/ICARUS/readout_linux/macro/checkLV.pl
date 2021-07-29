use lib $ENV{DCONLINE_PATH} . "/DCOV/readout/macro";

use strict;
use warnings;
use DCFOV;
use usb_readout;

#reading the IP configuration files
my $ip_config_file = "DCSpaceIP.config";      load_ip_config($ip_config_file);
my $port_config_file = "DCSpacePort.config";  load_port_config($port_config_file);
my $db_config_file = "DCDatabase.config";     load_db_config($db_config_file);

$OVSERV_HOST = "dcfovserv.in2p3.fr";
$user_online = "dconline";

checkLVstatus();
