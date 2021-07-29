#!/usr/bin/bash
echo "Starting CRT file size monitor.  It will not exit by itself, so use Ctrl+C to exit.  0 file size means that a USB channel is probably dead.";
while(true) do
  sleep 10; #wait 10 seconds because the old OM process knew that it would get a new file every 5 seconds
  perl -S check_file_size.pl
done;
