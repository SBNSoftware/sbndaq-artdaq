
set grid
set datafile separator ","
set term png size 1024,768

set title "ADC pulse heights"
set xlabel "Channel #"
set ylabel "ADC counts"
set autoscale
set xrange [0.5:64.5]
set yrange [9:*]
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/mean.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/summary.csv" using 1:2 title "Mean" with histeps

set title "ADC sigma"
set xlabel "Channel #"
set ylabel "ADC counts"
set autoscale
set xrange [0.5:64.5]
set yrange [0:*]
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/sigma.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/summary.csv" using 1:3 title "Sigma" with histeps

set title "Hits by channel"
set xlabel "Channel #"
set ylabel "Count"
set autoscale
set xrange [0.5:64.5]
set yrange [0:*]
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/per_channel.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/summary.csv" using 1:4 title "Hits" with histeps

set title "Hits in ADC packet"
set xlabel "Number #"
set ylabel "Count"
set autoscale
set xrange [0.5:64.5]
set yrange [0:*]
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/per_packet.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/summary.csv" using 1:5 title "Hits" with histeps

set title "Photoelectrons"
set xlabel "Channel #"
set ylabel "Number"
set autoscale
set xrange [0.5:64.5]
set yrange [0:*]
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/photo_e.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/summary.csv" using 1:6 title "Photoelectrons" with histeps

set title "Gain"
set xlabel "Channel #"
set ylabel "Number"
set autoscale
set xrange [0.5:64.5]
set yrange [0:*]
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/gain.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/summary.csv" using 1:7 title "Gain" with histeps

set title "Data acquisition rate per channel"
set xlabel "Channel #"
set ylabel "Rate [HZ]"
set autoscale
set xrange [0.5:64.5]
set yrange [0:*]
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/rate_per_channel.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/summary.csv" using 1:8 title "Rate (HZ)" with histeps

set title "Maroc Gain Constants"
set xlabel "Channel #"
set ylabel "Number"
set autoscale
set xrange [0.5:64.5]
set yrange [0:42]
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/gainconst.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/summary.csv" using 1:9 title "Maroc Gain Constants" with histeps

set title "Data acquisition rate per channel (normalized)"
set xlabel "Channel #"
set ylabel "Normalized Rate"
set autoscale
set xrange [0.5:64.5]
set yrange [0:*]
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/rate_per_channel_normalized.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/summary.csv" using 1:10 title "Normalized Rate" with histeps

set title "Channel 1  Mean = 1561.183709  Std Dev = 31.116149  Total = 2308.000000"
set autoscale
set yrange [0:1729]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/adc_01.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:2 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 0 title "Raw data" with impulses

set title "Channel 2  Mean = 12.531206  Std Dev = 30.790128  Total = 2852.000000"
set autoscale
set yrange [0:1729]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/adc_02.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:3 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 1 title "Raw data" with impulses

set title "Channel 3  Mean = 14.068144  Std Dev = 33.535702  Total = 3023.000000"
set autoscale
set yrange [0:1729]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/adc_03.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:4 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 2 title "Raw data" with impulses

set title "Channel 4  Mean = 17.475581  Std Dev = 40.254935  Total = 3358.000000"
set autoscale
set yrange [0:1729]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/adc_04.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:5 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 3 title "Raw data" with impulses

set title "Channel 5  Mean = 16.868940  Std Dev = 33.609895  Total = 2991.000000"
set autoscale
set yrange [0:1729]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/adc_05.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:6 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 4 title "Raw data" with impulses

set title "Channel 6  Mean = 10.483492  Std Dev = 26.468538  Total = 2726.000000"
set autoscale
set yrange [0:1729]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/adc_06.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:7 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 5 title "Raw data" with impulses

set title "Channel 7  Mean = 12.182378  Std Dev = 27.104159  Total = 2599.000000"
set autoscale
set yrange [0:1729]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/adc_07.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:8 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 6 title "Raw data" with impulses

set title "Channel 8  Mean = 10.818847  Std Dev = 24.692802  Total = 2186.000000"
set autoscale
set yrange [0:1729]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/adc_08.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:9 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 7 title "Raw data" with impulses

set title "Channel 9  Mean = 14.348883  Std Dev = 35.093931  Total = 3044.000000"
set autoscale
set yrange [0:1729]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/adc_09.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:10 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 8 title "Raw data" with impulses

set title "Channel 10  Mean = 44.386847  Std Dev = 57.414078  Total = 3619.000000"
set autoscale
set yrange [0:1729]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/adc_10.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:11 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 9 title "Raw data" with impulses

set title "Channel 11  Mean = 20.752347  Std Dev = 46.729975  Total = 3622.000000"
set autoscale
set yrange [0:1729]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/adc_11.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:12 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 10 title "Raw data" with impulses

set title "Channel 12  Mean = 13.359389  Std Dev = 37.737489  Total = 3403.000000"
set autoscale
set yrange [0:1729]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/adc_12.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:13 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 11 title "Raw data" with impulses

set title "Channel 13  Mean = 38.260222  Std Dev = 39.565997  Total = 3155.000000"
set autoscale
set yrange [0:1729]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/adc_13.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:14 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 12 title "Raw data" with impulses

set title "Channel 14  Mean = 12.698599  Std Dev = 29.331349  Total = 3069.000000"
set autoscale
set yrange [0:1729]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/adc_14.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:15 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 13 title "Raw data" with impulses

set title "Channel 15  Mean = 12.802232  Std Dev = 27.967402  Total = 2867.000000"
set autoscale
set yrange [0:1729]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/adc_15.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:16 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 14 title "Raw data" with impulses

set title "Channel 16  Mean = 11.767757  Std Dev = 24.363933  Total = 2140.000000"
set autoscale
set yrange [0:1729]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/adc_16.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:17 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 15 title "Raw data" with impulses

set title "Channel 17  Mean = 17.360985  Std Dev = 42.152681  Total = 3086.000000"
set autoscale
set yrange [0:1729]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/adc_17.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:18 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 16 title "Raw data" with impulses

set title "Channel 18  Mean = 23.599567  Std Dev = 48.261985  Total = 3696.000000"
set autoscale
set yrange [0:1729]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/adc_18.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:19 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 17 title "Raw data" with impulses

set title "Channel 19  Mean = 23.484581  Std Dev = 46.269352  Total = 3405.000000"
set autoscale
set yrange [0:1729]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/adc_19.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:20 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 18 title "Raw data" with impulses

set title "Channel 20  Mean = 18.711595  Std Dev = 38.139698  Total = 3079.000000"
set autoscale
set yrange [0:1729]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/adc_20.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:21 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 19 title "Raw data" with impulses

set title "Channel 21  Mean = 14.379772  Std Dev = 30.590677  Total = 3065.000000"
set autoscale
set yrange [0:1729]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/adc_21.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:22 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 20 title "Raw data" with impulses

set title "Channel 22  Mean = 15.310752  Std Dev = 31.824389  Total = 2967.000000"
set autoscale
set yrange [0:1729]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/adc_22.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:23 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 21 title "Raw data" with impulses

set title "Channel 23  Mean = 32.942507  Std Dev = 33.636354  Total = 2696.000000"
set autoscale
set yrange [0:1729]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/adc_23.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:24 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 22 title "Raw data" with impulses

set title "Channel 24  Mean = 12.798165  Std Dev = 23.861714  Total = 1635.000000"
set autoscale
set yrange [0:1729]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/adc_24.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:25 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 23 title "Raw data" with impulses

set title "Channel 25  Mean = 20.252739  Std Dev = 41.418749  Total = 2829.000000"
set autoscale
set yrange [0:1729]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/adc_25.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:26 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 24 title "Raw data" with impulses

set title "Channel 26  Mean = 19.180190  Std Dev = 46.576676  Total = 3574.000000"
set autoscale
set yrange [0:1729]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/adc_26.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:27 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 25 title "Raw data" with impulses

set title "Channel 27  Mean = 21.608776  Std Dev = 41.735126  Total = 3236.000000"
set autoscale
set yrange [0:1729]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/adc_27.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:28 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 26 title "Raw data" with impulses

set title "Channel 28  Mean = 18.894640  Std Dev = 38.406239  Total = 3265.000000"
set autoscale
set yrange [0:1729]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/adc_28.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:29 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 27 title "Raw data" with impulses

set title "Channel 29  Mean = 28.584350  Std Dev = 41.753338  Total = 2952.000000"
set autoscale
set yrange [0:1729]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/adc_29.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:30 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 28 title "Raw data" with impulses

set title "Channel 30  Mean = 11.441869  Std Dev = 26.850503  Total = 2804.000000"
set autoscale
set yrange [0:1729]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/adc_30.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:31 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 29 title "Raw data" with impulses

set title "Channel 31  Mean = 13.632189  Std Dev = 27.415504  Total = 2069.000000"
set autoscale
set yrange [0:1729]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/adc_31.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:32 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 30 title "Raw data" with impulses

set title "Channel 32  Mean = 9.203008  Std Dev = 19.608108  Total = 1330.000000"
set autoscale
set yrange [0:1729]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/adc_32.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:33 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 31 title "Raw data" with impulses

set title "Channel 33  Mean = 38.919687  Std Dev = 51.266982  Total = 2814.000000"
set autoscale
set yrange [0:1729]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/adc_33.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:34 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 32 title "Raw data" with impulses

set title "Channel 34  Mean = 28.138421  Std Dev = 49.996283  Total = 3446.000000"
set autoscale
set yrange [0:1729]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/adc_34.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:35 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 33 title "Raw data" with impulses

set title "Channel 35  Mean = 24.095223  Std Dev = 46.810293  Total = 3140.000000"
set autoscale
set yrange [0:1729]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/adc_35.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:36 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 34 title "Raw data" with impulses

set title "Channel 36  Mean = 20.731431  Std Dev = 40.073505  Total = 2558.000000"
set autoscale
set yrange [0:1729]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/adc_36.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:37 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 35 title "Raw data" with impulses

set title "Channel 37  Mean = 17.091155  Std Dev = 38.038969  Total = 2589.000000"
set autoscale
set yrange [0:1729]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/adc_37.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:38 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 36 title "Raw data" with impulses

set title "Channel 38  Mean = 13.551305  Std Dev = 29.949298  Total = 2222.000000"
set autoscale
set yrange [0:1729]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/adc_38.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:39 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 37 title "Raw data" with impulses

set title "Channel 39  Mean = 14.374656  Std Dev = 27.512830  Total = 2178.000000"
set autoscale
set yrange [0:1729]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/adc_39.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:40 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 38 title "Raw data" with impulses

set title "Channel 40  Mean = 11.639924  Std Dev = 21.712773  Total = 1047.000000"
set autoscale
set yrange [0:1729]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/adc_40.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:41 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 39 title "Raw data" with impulses

set title "Channel 41  Mean = 18.749129  Std Dev = 37.596500  Total = 2583.000000"
set autoscale
set yrange [0:1729]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/adc_41.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:42 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 40 title "Raw data" with impulses

set title "Channel 42  Mean = 45.502505  Std Dev = 65.357240  Total = 3393.000000"
set autoscale
set yrange [0:1729]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/adc_42.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:43 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 41 title "Raw data" with impulses

set title "Channel 43  Mean = 26.377551  Std Dev = 46.513571  Total = 3038.000000"
set autoscale
set yrange [0:1729]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/adc_43.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:44 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 42 title "Raw data" with impulses

set title "Channel 44  Mean = 26.913515  Std Dev = 47.825404  Total = 2856.000000"
set autoscale
set yrange [0:1729]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/adc_44.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:45 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 43 title "Raw data" with impulses

set title "Channel 45  Mean = 45.042346  Std Dev = 44.411188  Total = 2881.000000"
set autoscale
set yrange [0:1729]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/adc_45.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:46 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 44 title "Raw data" with impulses

set title "Channel 46  Mean = 196.962244  Std Dev = 247.697718  Total = 4688.000000"
set autoscale
set yrange [0:1729]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/adc_46.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:47 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 45 title "Raw data" with impulses

set title "Channel 47  Mean = 33.818182  Std Dev = 26.964010  Total = 11.000000"
set autoscale
set yrange [0:1729]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/adc_47.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:48 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 46 title "Raw data" with impulses

set title "Channel 48  Mean = 27.818158  Std Dev = 57.982167  Total = 3855.000000"
set autoscale
set yrange [0:1729]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/adc_48.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:49 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 47 title "Raw data" with impulses

set title "Channel 49  Mean = 430.936146  Std Dev = 418.289561  Total = 4479.000000"
set autoscale
set yrange [0:1729]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/adc_49.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:50 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 48 title "Raw data" with impulses

set title "Channel 50  Mean = 499.694283  Std Dev = 453.512690  Total = 4985.000000"
set autoscale
set yrange [0:1729]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/adc_50.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:51 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 49 title "Raw data" with impulses

set title "Channel 51  Mean = 463.124208  Std Dev = 431.978853  Total = 5048.000000"
set autoscale
set yrange [0:1729]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/adc_51.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:52 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 50 title "Raw data" with impulses

set title "Channel 52  Mean = 397.635823  Std Dev = 401.536066  Total = 5047.000000"
set autoscale
set yrange [0:1729]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/adc_52.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:53 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 51 title "Raw data" with impulses

set title "Channel 53  Mean = 350.663212  Std Dev = 366.296177  Total = 4825.000000"
set autoscale
set yrange [0:1729]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/adc_53.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:54 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 52 title "Raw data" with impulses

set title "Channel 54  Mean = 367.760486  Std Dev = 364.929888  Total = 4935.000000"
set autoscale
set yrange [0:1729]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/adc_54.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:55 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 53 title "Raw data" with impulses

set title "Channel 55  Mean = 273.508525  Std Dev = 295.187551  Total = 4692.000000"
set autoscale
set yrange [0:1729]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/adc_55.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:56 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 54 title "Raw data" with impulses

set title "Stefan Simion's Empty Histogram (tm). Formerly known as channel 56"
set autoscale
set yrange [0:1729]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/adc_56.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:57 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 55 title "Raw data" with impulses

set title "Channel 57  Mean = 220.148304  Std Dev = 249.009834  Total = 4039.000000"
set autoscale
set yrange [0:1729]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/adc_57.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:58 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 56 title "Raw data" with impulses

set title "Channel 58  Mean = 110.954751  Std Dev = 134.581480  Total = 4199.000000"
set autoscale
set yrange [0:1729]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/adc_58.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:59 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 57 title "Raw data" with impulses

set title "Channel 59  Mean = 95.995473  Std Dev = 126.828168  Total = 4197.000000"
set autoscale
set yrange [0:1729]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/adc_59.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:60 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 58 title "Raw data" with impulses

set title "Channel 60  Mean = 85.086649  Std Dev = 124.670839  Total = 4524.000000"
set autoscale
set yrange [0:1729]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/adc_60.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:61 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 59 title "Raw data" with impulses

set title "Channel 61  Mean = 81.116847  Std Dev = 120.034951  Total = 4262.000000"
set autoscale
set yrange [0:1729]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/adc_61.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:62 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 60 title "Raw data" with impulses

set title "Channel 62  Mean = 79.424800  Std Dev = 109.136572  Total = 4129.000000"
set autoscale
set yrange [0:1729]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/adc_62.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:63 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 61 title "Raw data" with impulses

set title "Channel 63  Mean = 56.738331  Std Dev = 96.852696  Total = 4135.000000"
set autoscale
set yrange [0:1729]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/adc_63.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:64 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 62 title "Raw data" with impulses

set title "Channel 64  Mean = 70.529039  Std Dev = 96.206248  Total = 4098.000000"
set autoscale
set yrange [0:1729]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/adc_64.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:65 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 63 title "Raw data" with impulses

set title "All channels"
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/all_in_one.png"
set autoscale
set yrange [0:1729]
set key spacing 0.5
plot\
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:2 title "Channel 1" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:3 title "Channel 2" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:4 title "Channel 3" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:5 title "Channel 4" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:6 title "Channel 5" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:7 title "Channel 6" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:8 title "Channel 7" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:9 title "Channel 8" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:10 title "Channel 9" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:11 title "Channel 10" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:12 title "Channel 11" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:13 title "Channel 12" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:14 title "Channel 13" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:15 title "Channel 14" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:16 title "Channel 15" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:17 title "Channel 16" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:18 title "Channel 17" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:19 title "Channel 18" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:20 title "Channel 19" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:21 title "Channel 20" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:22 title "Channel 21" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:23 title "Channel 22" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:24 title "Channel 23" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:25 title "Channel 24" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:26 title "Channel 25" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:27 title "Channel 26" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:28 title "Channel 27" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:29 title "Channel 28" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:30 title "Channel 29" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:31 title "Channel 30" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:32 title "Channel 31" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:33 title "Channel 32" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:34 title "Channel 33" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:35 title "Channel 34" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:36 title "Channel 35" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:37 title "Channel 36" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:38 title "Channel 37" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:39 title "Channel 38" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:40 title "Channel 39" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:41 title "Channel 40" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:42 title "Channel 41" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:43 title "Channel 42" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:44 title "Channel 43" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:45 title "Channel 44" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:46 title "Channel 45" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:47 title "Channel 46" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:48 title "Channel 47" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:49 title "Channel 48" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:50 title "Channel 49" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:51 title "Channel 50" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:52 title "Channel 51" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:53 title "Channel 52" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:54 title "Channel 53" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:55 title "Channel 54" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:56 title "Channel 55" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:57 title "Channel 56" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:58 title "Channel 57" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:59 title "Channel 58" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:60 title "Channel 59" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:61 title "Channel 60" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:62 title "Channel 61" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:63 title "Channel 62" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:64 title "Channel 63" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:65 title "Channel 64" with histeps

set title "ADC pulse heights"
unset grid
set xlabel "X"
set ylabel "Y"
set autoscale
set xrange [0.5:8.5]
set yrange [0.5:8.5]
set pm3d at b corners2color c1
set view map
set style data pm3d
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/mean_2d.png"
splot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/summary_2d.csv" using 1:2:3 title ""
unset pm3d
set grid

set title "Normalized ADC pulse heights"
unset grid
set xlabel "X"
set ylabel "Y"
set autoscale
set xrange [0.5:8.5]
set yrange [0.5:8.5]
set pm3d at b corners2color c1
set view map
set style data pm3d
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/mean_normalized_2d.png"
splot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/summary_2d.csv" using 1:2:9 title ""
unset pm3d
set grid

set title "Histograms"
unset grid
set xlabel "Channel"
set ylabel "X"
set autoscale
set xrange [0.5:64.5]
set yrange [9:1907]
set pm3d at b corners2color c1
set view map
set style data pm3d
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist_2d.png"
splot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist_2d.csv" using 1:2:3 title ""
unset pm3d
set grid

set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/all_in_8x8.png"
unset grid
unset title
unset xlabel
unset ylabel
unset key
set autoscale
set yrange [0:1729]
set noxtics
set noytics
set format x ""
set format y ""
set multiplot layout 8,8

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:2 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 0 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:3 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 1 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:4 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 2 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:5 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 3 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:6 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 4 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:7 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 5 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:8 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 6 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:9 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 7 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:10 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 8 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:11 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 9 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:12 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 10 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:13 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 11 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:14 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 12 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:15 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 13 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:16 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 14 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:17 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 15 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:18 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 16 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:19 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 17 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:20 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 18 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:21 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 19 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:22 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 20 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:23 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 21 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:24 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 22 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:25 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 23 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:26 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 24 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:27 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 25 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:28 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 26 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:29 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 27 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:30 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 28 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:31 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 29 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:32 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 30 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:33 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 31 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:34 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 32 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:35 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 33 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:36 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 34 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:37 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 35 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:38 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 36 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:39 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 37 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:40 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 38 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:41 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 39 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:42 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 40 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:43 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 41 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:44 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 42 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:45 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 43 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:46 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 44 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:47 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 45 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:48 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 46 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:49 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 47 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:50 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 48 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:51 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 49 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:52 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 50 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:53 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 51 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:54 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 52 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:55 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 53 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:56 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 54 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:57 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 55 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:58 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 56 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:59 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 57 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:60 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 58 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:61 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 59 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:62 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 60 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:63 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 61 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:64 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 62 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/hist.csv" using 1:65 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000094/USB_39/PMT_5/raw.csv" index 63 with impulses

unset multiplot
