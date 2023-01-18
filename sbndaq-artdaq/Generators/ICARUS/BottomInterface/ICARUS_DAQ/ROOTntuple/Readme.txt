# Matt Toups
# Columbia U.
# 6/15/2009
# 
# Updated 7/31/2009 MT
# ChipProgram is now a compiled program incorporating a ROOT decoder (instead of a perl script).
# Loop.pl was heavily modified to account for the fact that diagnostics can no
# longer be run on the intermediate text files as before.
# Code was tested on 90 13MB files (30 minutes of cosmic data) and processed
# all files in 14 minutes.
#
# Updated 11/18/2010 MT & CM
# Updated to work on crackle at nevis
# To port to Chooz, must 
#   1. Enter the path of the Windows DAQ data synced using rsync in Wrapper.pl and AutoWrapper.pl (here listed as /data1/raphael/DATA)
#   2. Change the USB # hardcoded into AutoProcessData.sh (currently 7)
#   3. If using interactive ProcessData.sh you must pass the USB number as the only argument
#

ProcessData.sh takes the raw binary files output from the windows DAQ and converts them into root Ntuples.

It assumes the raw data files have base path="/cp/s1/dchooz/data/ModuleTest/ChicagoLab/run", where run is a folder of the form Run_20090603_0 and that all the processing scripts are located in "/cp/s1/dchooz/data/ModuleTest/ProcessData".

It then writes root files of the same name as the binary files into the same folder.

To run, type ./ProcessData.sh into the command line.
Follow the directions on the screen.  It will begin processing the data in the background.

AutoProcessData.sh is intended to work similarly to ProcessData.sh, but with a crontab.
This processes any runs taken in the last week (using the date in the run name).
I have set this script to only allow at most 2 batch jobs to run at once.
Upon completion the crontab will send an automated email to the account associate with dchooz at UChicago.

A note on crontabs.  Here is the latest crontab file I am using (foo.cron):
58 * * * * /cp/s1/dchooz/data/ModuleTest/ProcessData/AutoProcessData.sh
28 * * * * /cp/s1/dchooz/data/ModuleTest/ProcessData/AutoProcessData.sh

This runs AutoProcessData every 30 minutes, 2 minutes before the hour or half-hour.
To modify this, execute any of the following commands:
crontab -l (list crontabs for this user on this machine)
crontab -r (remove all crontabs for this user on this machine)
crontab foo.cron (load crontabs specified in file foo.cron)
