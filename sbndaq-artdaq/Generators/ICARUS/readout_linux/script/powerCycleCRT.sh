#/bin/bash
#Brief: Script to power cycle the CRT.  Between turning the CRT off and on, 
#       allow 30 seconds for all circuits to fully power down.  
#Usage: powerCycleCRT.sh
#Author: Andrew Olivier aolivier@ur.rochester.edu

turnOffCRTLV.exp
pollAllCRTLV.sh
echo 'The CRT should now be off.  Waiting 30 seconds before turning it on again...\n'
sleep 30
turnOnCRTLV.exp
pollAllCRTLV.sh
