#!/usr/bin/expect --

spawn ssh dcdaq@dcfwti.in2p3.fr
expect "Password:"
send "doublechooz\r"
#expect "login:"
#send "dcdaq\r"
#expect "Password:"
#send "doublechooz\r"
expect "MPC>"
send "/S\r"
expect "MPC>"
sleep 2
send "/OFF OVDAQ,Y\r"
expect "MPC>"
sleep 5
send "/ON OVDAQ,Y\r"
sleep2
expect "MPC>"
send "/S\r"
sleep 20
send "/S\r"
expect "MPC>"
sleep 2
send "/X\r"
# end of expect script.