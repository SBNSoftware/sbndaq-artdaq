#!/bin/bash

source /software/products/setup
source ./setup

while [ 1 ]; do 
  date >> ./gps.log 
  ./gps
done 

