#!/bin/bash
#Brief: Print out the status of all CRT low voltage channels
#Usage: pollAllCRTLV.sh
#Author: Andrew Olivier aolivier@ur.rochester.edu

echo '#Top power supply:'
pollCRTLV.exp np04-ps-crt-01.cern.ch
echo '#Bottom power supply:'
pollCRTLV.exp np04-ps-crt-02.cern.ch
