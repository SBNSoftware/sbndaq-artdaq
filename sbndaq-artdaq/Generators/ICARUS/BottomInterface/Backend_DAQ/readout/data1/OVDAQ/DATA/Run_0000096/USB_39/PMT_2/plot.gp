
set grid
set datafile separator ","
set term png size 1024,768

set title "ADC pulse heights"
set xlabel "Channel #"
set ylabel "ADC counts"
set autoscale
set xrange [0.5:64.5]
set yrange [8:*]
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/mean.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/summary.csv" using 1:2 title "Mean" with histeps

set title "ADC sigma"
set xlabel "Channel #"
set ylabel "ADC counts"
set autoscale
set xrange [0.5:64.5]
set yrange [0:*]
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/sigma.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/summary.csv" using 1:3 title "Sigma" with histeps

set title "Hits by channel"
set xlabel "Channel #"
set ylabel "Count"
set autoscale
set xrange [0.5:64.5]
set yrange [0:*]
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/per_channel.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/summary.csv" using 1:4 title "Hits" with histeps

set title "Hits in ADC packet"
set xlabel "Number #"
set ylabel "Count"
set autoscale
set xrange [0.5:64.5]
set yrange [0:*]
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/per_packet.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/summary.csv" using 1:5 title "Hits" with histeps

set title "Photoelectrons"
set xlabel "Channel #"
set ylabel "Number"
set autoscale
set xrange [0.5:64.5]
set yrange [0:*]
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/photo_e.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/summary.csv" using 1:6 title "Photoelectrons" with histeps

set title "Gain"
set xlabel "Channel #"
set ylabel "Number"
set autoscale
set xrange [0.5:64.5]
set yrange [0:*]
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/gain.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/summary.csv" using 1:7 title "Gain" with histeps

set title "Data acquisition rate per channel"
set xlabel "Channel #"
set ylabel "Rate [HZ]"
set autoscale
set xrange [0.5:64.5]
set yrange [0:*]
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/rate_per_channel.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/summary.csv" using 1:8 title "Rate (HZ)" with histeps

set title "Maroc Gain Constants"
set xlabel "Channel #"
set ylabel "Number"
set autoscale
set xrange [0.5:64.5]
set yrange [0:42]
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/gainconst.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/summary.csv" using 1:9 title "Maroc Gain Constants" with histeps

set title "Data acquisition rate per channel (normalized)"
set xlabel "Channel #"
set ylabel "Normalized Rate"
set autoscale
set xrange [0.5:64.5]
set yrange [0:*]
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/rate_per_channel_normalized.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/summary.csv" using 1:10 title "Normalized Rate" with histeps

set title "Channel 1  Mean = 38.289870  Std Dev = 33.488725  Total = 1994.000000"
set autoscale
set yrange [0:1130]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/adc_01.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:2 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 0 title "Raw data" with impulses

set title "Channel 2  Mean = 39.536467  Std Dev = 32.524008  Total = 2468.000000"
set autoscale
set yrange [0:1130]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/adc_02.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:3 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 1 title "Raw data" with impulses

set title "Channel 3  Mean = 44.268210  Std Dev = 38.183115  Total = 2073.000000"
set autoscale
set yrange [0:1130]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/adc_03.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:4 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 2 title "Raw data" with impulses

set title "Channel 4  Mean = 45.867490  Std Dev = 42.917524  Total = 2347.000000"
set autoscale
set yrange [0:1130]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/adc_04.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:5 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 3 title "Raw data" with impulses

set title "Channel 5  Mean = 45.161570  Std Dev = 39.621629  Total = 2191.000000"
set autoscale
set yrange [0:1130]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/adc_05.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:6 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 4 title "Raw data" with impulses

set title "Channel 6  Mean = 42.288338  Std Dev = 39.449860  Total = 2341.000000"
set autoscale
set yrange [0:1130]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/adc_06.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:7 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 5 title "Raw data" with impulses

set title "Channel 7  Mean = 44.483244  Std Dev = 37.107186  Total = 2059.000000"
set autoscale
set yrange [0:1130]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/adc_07.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:8 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 6 title "Raw data" with impulses

set title "Channel 8  Mean = 45.176337  Std Dev = 34.408568  Total = 1758.000000"
set autoscale
set yrange [0:1130]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/adc_08.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:9 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 7 title "Raw data" with impulses

set title "Channel 9  Mean = 43.570700  Std Dev = 39.654783  Total = 2157.000000"
set autoscale
set yrange [0:1130]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/adc_09.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:10 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 8 title "Raw data" with impulses

set title "Channel 10  Mean = 47.675755  Std Dev = 40.325513  Total = 2384.000000"
set autoscale
set yrange [0:1130]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/adc_10.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:11 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 9 title "Raw data" with impulses

set title "Channel 11  Mean = 45.245794  Std Dev = 39.844210  Total = 2140.000000"
set autoscale
set yrange [0:1130]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/adc_11.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:12 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 10 title "Raw data" with impulses

set title "Channel 12  Mean = 48.129979  Std Dev = 45.538721  Total = 2385.000000"
set autoscale
set yrange [0:1130]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/adc_12.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:13 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 11 title "Raw data" with impulses

set title "Channel 13  Mean = 47.271628  Std Dev = 40.133100  Total = 2150.000000"
set autoscale
set yrange [0:1130]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/adc_13.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:14 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 12 title "Raw data" with impulses

set title "Channel 14  Mean = 48.590449  Std Dev = 41.924399  Total = 2073.000000"
set autoscale
set yrange [0:1130]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/adc_14.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:15 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 13 title "Raw data" with impulses

set title "Channel 15  Mean = 47.321171  Std Dev = 43.489323  Total = 2083.000000"
set autoscale
set yrange [0:1130]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/adc_15.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:16 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 14 title "Raw data" with impulses

set title "Channel 16  Mean = 48.991786  Std Dev = 41.564328  Total = 1948.000000"
set autoscale
set yrange [0:1130]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/adc_16.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:17 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 15 title "Raw data" with impulses

set title "Channel 17  Mean = 45.302337  Std Dev = 38.628914  Total = 2011.000000"
set autoscale
set yrange [0:1130]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/adc_17.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:18 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 16 title "Raw data" with impulses

set title "Channel 18  Mean = 50.540665  Std Dev = 45.784078  Total = 2164.000000"
set autoscale
set yrange [0:1130]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/adc_18.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:19 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 17 title "Raw data" with impulses

set title "Channel 19  Mean = 48.702075  Std Dev = 41.980735  Total = 2024.000000"
set autoscale
set yrange [0:1130]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/adc_19.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:20 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 18 title "Raw data" with impulses

set title "Channel 20  Mean = 46.646163  Std Dev = 37.736800  Total = 1772.000000"
set autoscale
set yrange [0:1130]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/adc_20.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:21 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 19 title "Raw data" with impulses

set title "Channel 21  Mean = 48.662243  Std Dev = 45.288235  Total = 2336.000000"
set autoscale
set yrange [0:1130]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/adc_21.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:22 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 20 title "Raw data" with impulses

set title "Channel 22  Mean = 51.251432  Std Dev = 39.703329  Total = 1746.000000"
set autoscale
set yrange [0:1130]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/adc_22.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:23 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 21 title "Raw data" with impulses

set title "Channel 23  Mean = 49.022982  Std Dev = 42.083880  Total = 1871.000000"
set autoscale
set yrange [0:1130]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/adc_23.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:24 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 22 title "Raw data" with impulses

set title "Channel 24  Mean = 47.118079  Std Dev = 37.332952  Total = 1770.000000"
set autoscale
set yrange [0:1130]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/adc_24.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:25 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 23 title "Raw data" with impulses

set title "Channel 25  Mean = 47.844538  Std Dev = 40.793907  Total = 1904.000000"
set autoscale
set yrange [0:1130]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/adc_25.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:26 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 24 title "Raw data" with impulses

set title "Channel 26  Mean = 47.016411  Std Dev = 35.938484  Total = 1889.000000"
set autoscale
set yrange [0:1130]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/adc_26.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:27 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 25 title "Raw data" with impulses

set title "Channel 27  Mean = 47.384768  Std Dev = 39.452701  Total = 2022.000000"
set autoscale
set yrange [0:1130]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/adc_27.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:28 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 26 title "Raw data" with impulses

set title "Channel 28  Mean = 46.791713  Std Dev = 36.893295  Total = 1810.000000"
set autoscale
set yrange [0:1130]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/adc_28.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:29 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 27 title "Raw data" with impulses

set title "Channel 29  Mean = 51.800338  Std Dev = 49.677467  Total = 1773.000000"
set autoscale
set yrange [0:1130]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/adc_29.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:30 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 28 title "Raw data" with impulses

set title "Channel 30  Mean = 47.636848  Std Dev = 41.202226  Total = 1688.000000"
set autoscale
set yrange [0:1130]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/adc_30.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:31 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 29 title "Raw data" with impulses

set title "Channel 31  Mean = 56.766983  Std Dev = 41.881012  Total = 1266.000000"
set autoscale
set yrange [0:1130]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/adc_31.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:32 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 30 title "Raw data" with impulses

set title "Channel 32  Mean = 49.687581  Std Dev = 37.601489  Total = 1546.000000"
set autoscale
set yrange [0:1130]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/adc_32.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:33 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 31 title "Raw data" with impulses

set title "Channel 33  Mean = 45.867492  Std Dev = 35.420391  Total = 1615.000000"
set autoscale
set yrange [0:1130]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/adc_33.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:34 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 32 title "Raw data" with impulses

set title "Channel 34  Mean = 49.711356  Std Dev = 41.388090  Total = 1902.000000"
set autoscale
set yrange [0:1130]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/adc_34.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:35 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 33 title "Raw data" with impulses

set title "Channel 35  Mean = 55.236825  Std Dev = 42.714844  Total = 1575.000000"
set autoscale
set yrange [0:1130]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/adc_35.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:36 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 34 title "Raw data" with impulses

set title "Channel 36  Mean = 48.855937  Std Dev = 37.015095  Total = 1659.000000"
set autoscale
set yrange [0:1130]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/adc_36.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:37 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 35 title "Raw data" with impulses

set title "Channel 37  Mean = 50.904094  Std Dev = 39.395265  Total = 1783.000000"
set autoscale
set yrange [0:1130]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/adc_37.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:38 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 36 title "Raw data" with impulses

set title "Channel 38  Mean = 49.484321  Std Dev = 35.667418  Total = 1435.000000"
set autoscale
set yrange [0:1130]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/adc_38.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:39 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 37 title "Raw data" with impulses

set title "Channel 39  Mean = 53.468160  Std Dev = 41.493835  Total = 1696.000000"
set autoscale
set yrange [0:1130]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/adc_39.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:40 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 38 title "Raw data" with impulses

set title "Channel 40  Mean = 55.024903  Std Dev = 42.615992  Total = 1807.000000"
set autoscale
set yrange [0:1130]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/adc_40.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:41 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 39 title "Raw data" with impulses

set title "Channel 41  Mean = 53.869511  Std Dev = 40.602828  Total = 1617.000000"
set autoscale
set yrange [0:1130]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/adc_41.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:42 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 40 title "Raw data" with impulses

set title "Channel 42  Mean = 50.429697  Std Dev = 42.009829  Total = 2411.000000"
set autoscale
set yrange [0:1130]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/adc_42.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:43 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 41 title "Raw data" with impulses

set title "Channel 43  Mean = 56.777397  Std Dev = 48.057181  Total = 1752.000000"
set autoscale
set yrange [0:1130]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/adc_43.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:44 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 42 title "Raw data" with impulses

set title "Channel 44  Mean = 59.048736  Std Dev = 45.397158  Total = 1662.000000"
set autoscale
set yrange [0:1130]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/adc_44.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:45 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 43 title "Raw data" with impulses

set title "Channel 45  Mean = 57.137073  Std Dev = 52.709230  Total = 1729.000000"
set autoscale
set yrange [0:1130]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/adc_45.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:46 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 44 title "Raw data" with impulses

set title "Channel 46  Mean = 205.735849  Std Dev = 243.075506  Total = 4876.000000"
set autoscale
set yrange [0:1130]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/adc_46.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:47 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 45 title "Raw data" with impulses

set title "Channel 47  Mean = 73.625000  Std Dev = 9.472823  Total = 8.000000"
set autoscale
set yrange [0:1130]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/adc_47.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:48 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 46 title "Raw data" with impulses

set title "Channel 48  Mean = 80.475344  Std Dev = 92.049818  Total = 3995.000000"
set autoscale
set yrange [0:1130]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/adc_48.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:49 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 47 title "Raw data" with impulses

set title "Channel 49  Mean = 348.799611  Std Dev = 360.219559  Total = 4631.000000"
set autoscale
set yrange [0:1130]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/adc_49.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:50 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 48 title "Raw data" with impulses

set title "Channel 50  Mean = 368.461954  Std Dev = 382.262453  Total = 4994.000000"
set autoscale
set yrange [0:1130]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/adc_50.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:51 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 49 title "Raw data" with impulses

set title "Channel 51  Mean = 374.605535  Std Dev = 388.708866  Total = 5131.000000"
set autoscale
set yrange [0:1130]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/adc_51.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:52 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 50 title "Raw data" with impulses

set title "Channel 52  Mean = 338.602574  Std Dev = 365.179872  Total = 4972.000000"
set autoscale
set yrange [0:1130]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/adc_52.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:53 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 51 title "Raw data" with impulses

set title "Channel 53  Mean = 359.387696  Std Dev = 387.379699  Total = 5169.000000"
set autoscale
set yrange [0:1130]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/adc_53.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:54 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 52 title "Raw data" with impulses

set title "Channel 54  Mean = 354.011734  Std Dev = 371.468128  Total = 4943.000000"
set autoscale
set yrange [0:1130]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/adc_54.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:55 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 53 title "Raw data" with impulses

set title "Channel 55  Mean = 342.943730  Std Dev = 369.318976  Total = 4976.000000"
set autoscale
set yrange [0:1130]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/adc_55.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:56 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 54 title "Raw data" with impulses

set title "Stefan Simion's Empty Histogram (tm). Formerly known as channel 56"
set autoscale
set yrange [0:1130]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/adc_56.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:57 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 55 title "Raw data" with impulses

set title "Channel 57  Mean = 182.995081  Std Dev = 213.429592  Total = 4269.000000"
set autoscale
set yrange [0:1130]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/adc_57.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:58 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 56 title "Raw data" with impulses

set title "Channel 58  Mean = 95.394092  Std Dev = 109.898479  Total = 3961.000000"
set autoscale
set yrange [0:1130]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/adc_58.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:59 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 57 title "Raw data" with impulses

set title "Channel 59  Mean = 105.079773  Std Dev = 115.991276  Total = 4049.000000"
set autoscale
set yrange [0:1130]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/adc_59.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:60 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 58 title "Raw data" with impulses

set title "Channel 60  Mean = 107.815002  Std Dev = 123.821880  Total = 4146.000000"
set autoscale
set yrange [0:1130]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/adc_60.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:61 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 59 title "Raw data" with impulses

set title "Channel 61  Mean = 107.304737  Std Dev = 123.752538  Total = 4243.000000"
set autoscale
set yrange [0:1130]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/adc_61.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:62 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 60 title "Raw data" with impulses

set title "Channel 62  Mean = 107.223503  Std Dev = 124.766019  Total = 3991.000000"
set autoscale
set yrange [0:1130]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/adc_62.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:63 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 61 title "Raw data" with impulses

set title "Channel 63  Mean = 103.468043  Std Dev = 117.825459  Total = 4021.000000"
set autoscale
set yrange [0:1130]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/adc_63.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:64 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 62 title "Raw data" with impulses

set title "Channel 64  Mean = 96.577560  Std Dev = 109.357179  Total = 3681.000000"
set autoscale
set yrange [0:1130]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/adc_64.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:65 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 63 title "Raw data" with impulses

set title "All channels"
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/all_in_one.png"
set autoscale
set yrange [0:1130]
set key spacing 0.5
plot\
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:2 title "Channel 1" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:3 title "Channel 2" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:4 title "Channel 3" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:5 title "Channel 4" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:6 title "Channel 5" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:7 title "Channel 6" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:8 title "Channel 7" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:9 title "Channel 8" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:10 title "Channel 9" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:11 title "Channel 10" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:12 title "Channel 11" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:13 title "Channel 12" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:14 title "Channel 13" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:15 title "Channel 14" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:16 title "Channel 15" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:17 title "Channel 16" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:18 title "Channel 17" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:19 title "Channel 18" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:20 title "Channel 19" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:21 title "Channel 20" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:22 title "Channel 21" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:23 title "Channel 22" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:24 title "Channel 23" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:25 title "Channel 24" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:26 title "Channel 25" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:27 title "Channel 26" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:28 title "Channel 27" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:29 title "Channel 28" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:30 title "Channel 29" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:31 title "Channel 30" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:32 title "Channel 31" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:33 title "Channel 32" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:34 title "Channel 33" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:35 title "Channel 34" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:36 title "Channel 35" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:37 title "Channel 36" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:38 title "Channel 37" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:39 title "Channel 38" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:40 title "Channel 39" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:41 title "Channel 40" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:42 title "Channel 41" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:43 title "Channel 42" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:44 title "Channel 43" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:45 title "Channel 44" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:46 title "Channel 45" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:47 title "Channel 46" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:48 title "Channel 47" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:49 title "Channel 48" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:50 title "Channel 49" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:51 title "Channel 50" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:52 title "Channel 51" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:53 title "Channel 52" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:54 title "Channel 53" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:55 title "Channel 54" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:56 title "Channel 55" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:57 title "Channel 56" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:58 title "Channel 57" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:59 title "Channel 58" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:60 title "Channel 59" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:61 title "Channel 60" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:62 title "Channel 61" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:63 title "Channel 62" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:64 title "Channel 63" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:65 title "Channel 64" with histeps

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
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/mean_2d.png"
splot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/summary_2d.csv" using 1:2:3 title ""
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
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/mean_normalized_2d.png"
splot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/summary_2d.csv" using 1:2:9 title ""
unset pm3d
set grid

set title "Histograms"
unset grid
set xlabel "Channel"
set ylabel "X"
set autoscale
set xrange [0.5:64.5]
set yrange [8:1643]
set pm3d at b corners2color c1
set view map
set style data pm3d
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist_2d.png"
splot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist_2d.csv" using 1:2:3 title ""
unset pm3d
set grid

set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/all_in_8x8.png"
unset grid
unset title
unset xlabel
unset ylabel
unset key
set autoscale
set yrange [0:1130]
set noxtics
set noytics
set format x ""
set format y ""
set multiplot layout 8,8

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:2 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 0 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:3 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 1 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:4 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 2 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:5 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 3 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:6 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 4 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:7 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 5 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:8 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 6 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:9 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 7 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:10 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 8 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:11 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 9 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:12 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 10 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:13 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 11 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:14 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 12 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:15 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 13 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:16 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 14 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:17 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 15 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:18 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 16 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:19 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 17 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:20 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 18 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:21 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 19 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:22 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 20 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:23 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 21 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:24 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 22 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:25 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 23 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:26 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 24 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:27 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 25 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:28 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 26 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:29 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 27 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:30 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 28 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:31 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 29 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:32 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 30 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:33 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 31 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:34 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 32 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:35 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 33 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:36 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 34 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:37 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 35 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:38 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 36 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:39 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 37 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:40 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 38 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:41 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 39 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:42 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 40 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:43 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 41 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:44 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 42 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:45 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 43 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:46 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 44 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:47 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 45 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:48 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 46 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:49 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 47 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:50 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 48 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:51 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 49 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:52 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 50 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:53 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 51 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:54 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 52 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:55 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 53 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:56 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 54 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:57 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 55 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:58 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 56 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:59 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 57 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:60 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 58 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:61 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 59 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:62 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 60 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:63 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 61 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:64 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 62 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/hist.csv" using 1:65 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000096/USB_39/PMT_2/raw.csv" index 63 with impulses

unset multiplot
