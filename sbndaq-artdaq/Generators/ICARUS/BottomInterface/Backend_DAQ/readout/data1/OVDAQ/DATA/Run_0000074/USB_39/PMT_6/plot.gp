
set grid
set datafile separator ","
set term png size 1024,768

set title "ADC pulse heights"
set xlabel "Channel #"
set ylabel "ADC counts"
set autoscale
set xrange [0.5:64.5]
set yrange [20:*]
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/mean.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/summary.csv" using 1:2 title "Mean" with histeps

set title "ADC sigma"
set xlabel "Channel #"
set ylabel "ADC counts"
set autoscale
set xrange [0.5:64.5]
set yrange [0:*]
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/sigma.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/summary.csv" using 1:3 title "Sigma" with histeps

set title "Hits by channel"
set xlabel "Channel #"
set ylabel "Count"
set autoscale
set xrange [0.5:64.5]
set yrange [0:*]
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/per_channel.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/summary.csv" using 1:4 title "Hits" with histeps

set title "Hits in ADC packet"
set xlabel "Number #"
set ylabel "Count"
set autoscale
set xrange [0.5:64.5]
set yrange [0:*]
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/per_packet.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/summary.csv" using 1:5 title "Hits" with histeps

set title "Photoelectrons"
set xlabel "Channel #"
set ylabel "Number"
set autoscale
set xrange [0.5:64.5]
set yrange [0:*]
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/photo_e.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/summary.csv" using 1:6 title "Photoelectrons" with histeps

set title "Gain"
set xlabel "Channel #"
set ylabel "Number"
set autoscale
set xrange [0.5:64.5]
set yrange [0:*]
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/gain.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/summary.csv" using 1:7 title "Gain" with histeps

set title "Data acquisition rate per channel"
set xlabel "Channel #"
set ylabel "Rate [HZ]"
set autoscale
set xrange [0.5:64.5]
set yrange [0:*]
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/rate_per_channel.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/summary.csv" using 1:8 title "Rate (HZ)" with histeps

set title "Maroc Gain Constants"
set xlabel "Channel #"
set ylabel "Number"
set autoscale
set xrange [0.5:64.5]
set yrange [0:42]
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/gainconst.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/summary.csv" using 1:9 title "Maroc Gain Constants" with histeps

set title "Data acquisition rate per channel (normalized)"
set xlabel "Channel #"
set ylabel "Normalized Rate"
set autoscale
set xrange [0.5:64.5]
set yrange [0:*]
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/rate_per_channel_normalized.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/summary.csv" using 1:10 title "Normalized Rate" with histeps

set title "Channel 1  Mean = 1591.806385  Std Dev = 35.901528  Total = 5294.000000"
set autoscale
set yrange [0:2287]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/adc_01.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:2 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 0 title "Raw data" with impulses

set title "Channel 2  Mean = 45.204040  Std Dev = 29.765441  Total = 3416.000000"
set autoscale
set yrange [0:2287]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/adc_02.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:3 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 1 title "Raw data" with impulses

set title "Channel 3  Mean = 47.380834  Std Dev = 30.211811  Total = 3214.000000"
set autoscale
set yrange [0:2287]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/adc_03.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:4 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 2 title "Raw data" with impulses

set title "Channel 4  Mean = 47.127347  Std Dev = 30.341785  Total = 2929.000000"
set autoscale
set yrange [0:2287]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/adc_04.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:5 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 3 title "Raw data" with impulses

set title "Channel 5  Mean = 43.743439  Std Dev = 26.408463  Total = 2896.000000"
set autoscale
set yrange [0:2287]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/adc_05.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:6 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 4 title "Raw data" with impulses

set title "Channel 6  Mean = 45.845655  Std Dev = 26.421709  Total = 2773.000000"
set autoscale
set yrange [0:2287]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/adc_06.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:7 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 5 title "Raw data" with impulses

set title "Channel 7  Mean = 49.798095  Std Dev = 29.046305  Total = 2100.000000"
set autoscale
set yrange [0:2287]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/adc_07.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:8 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 6 title "Raw data" with impulses

set title "Channel 8  Mean = 47.863065  Std Dev = 28.303431  Total = 2147.000000"
set autoscale
set yrange [0:2287]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/adc_08.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:9 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 7 title "Raw data" with impulses

set title "Channel 9  Mean = 46.960756  Std Dev = 36.219061  Total = 5504.000000"
set autoscale
set yrange [0:2287]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/adc_09.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:10 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 8 title "Raw data" with impulses

set title "Channel 10  Mean = 49.630570  Std Dev = 36.470908  Total = 5633.000000"
set autoscale
set yrange [0:2287]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/adc_10.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:11 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 9 title "Raw data" with impulses

set title "Channel 11  Mean = 50.038690  Std Dev = 39.228538  Total = 5893.000000"
set autoscale
set yrange [0:2287]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/adc_11.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:12 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 10 title "Raw data" with impulses

set title "Channel 12  Mean = 48.437008  Std Dev = 36.704833  Total = 4826.000000"
set autoscale
set yrange [0:2287]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/adc_12.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:13 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 11 title "Raw data" with impulses

set title "Channel 13  Mean = 51.311985  Std Dev = 34.770443  Total = 3763.000000"
set autoscale
set yrange [0:2287]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/adc_13.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:14 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 12 title "Raw data" with impulses

set title "Channel 14  Mean = 51.469276  Std Dev = 32.403904  Total = 3092.000000"
set autoscale
set yrange [0:2287]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/adc_14.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:15 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 13 title "Raw data" with impulses

set title "Channel 15  Mean = 49.234851  Std Dev = 31.590590  Total = 2921.000000"
set autoscale
set yrange [0:2287]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/adc_15.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:16 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 14 title "Raw data" with impulses

set title "Channel 16  Mean = 55.112150  Std Dev = 31.159838  Total = 1926.000000"
set autoscale
set yrange [0:2287]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/adc_16.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:17 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 15 title "Raw data" with impulses

set title "Channel 17  Mean = 54.216924  Std Dev = 40.427202  Total = 5117.000000"
set autoscale
set yrange [0:2287]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/adc_17.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:18 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 16 title "Raw data" with impulses

set title "Channel 18  Mean = 53.360573  Std Dev = 47.923441  Total = 5935.000000"
set autoscale
set yrange [0:2287]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/adc_18.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:19 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 17 title "Raw data" with impulses

set title "Channel 19  Mean = 58.110472  Std Dev = 46.075196  Total = 4870.000000"
set autoscale
set yrange [0:2287]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/adc_19.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:20 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 18 title "Raw data" with impulses

set title "Channel 20  Mean = 55.928790  Std Dev = 48.761828  Total = 3272.000000"
set autoscale
set yrange [0:2287]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/adc_20.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:21 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 19 title "Raw data" with impulses

set title "Channel 21  Mean = 59.942470  Std Dev = 43.088631  Total = 3320.000000"
set autoscale
set yrange [0:2287]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/adc_21.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:22 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 20 title "Raw data" with impulses

set title "Channel 22  Mean = 55.426662  Std Dev = 38.024879  Total = 3143.000000"
set autoscale
set yrange [0:2287]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/adc_22.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:23 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 21 title "Raw data" with impulses

set title "Channel 23  Mean = 53.385250  Std Dev = 33.312906  Total = 2061.000000"
set autoscale
set yrange [0:2287]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/adc_23.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:24 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 22 title "Raw data" with impulses

set title "Channel 24  Mean = 55.236951  Std Dev = 34.891740  Total = 1207.000000"
set autoscale
set yrange [0:2287]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/adc_24.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:25 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 23 title "Raw data" with impulses

set title "Channel 25  Mean = 56.222976  Std Dev = 46.311814  Total = 4718.000000"
set autoscale
set yrange [0:2287]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/adc_25.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:26 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 24 title "Raw data" with impulses

set title "Channel 26  Mean = 60.194948  Std Dev = 47.147711  Total = 5186.000000"
set autoscale
set yrange [0:2287]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/adc_26.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:27 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 25 title "Raw data" with impulses

set title "Channel 27  Mean = 57.487407  Std Dev = 43.530939  Total = 4050.000000"
set autoscale
set yrange [0:2287]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/adc_27.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:28 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 26 title "Raw data" with impulses

set title "Channel 28  Mean = 56.794857  Std Dev = 40.781327  Total = 3578.000000"
set autoscale
set yrange [0:2287]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/adc_28.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:29 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 27 title "Raw data" with impulses

set title "Channel 29  Mean = 69.697567  Std Dev = 50.865313  Total = 2589.000000"
set autoscale
set yrange [0:2287]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/adc_29.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:30 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 28 title "Raw data" with impulses

set title "Channel 30  Mean = 63.870263  Std Dev = 41.618650  Total = 2243.000000"
set autoscale
set yrange [0:2287]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/adc_30.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:31 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 29 title "Raw data" with impulses

set title "Channel 31  Mean = 59.845575  Std Dev = 35.154042  Total = 1729.000000"
set autoscale
set yrange [0:2287]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/adc_31.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:32 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 30 title "Raw data" with impulses

set title "Channel 32  Mean = 58.585921  Std Dev = 28.938472  Total = 966.000000"
set autoscale
set yrange [0:2287]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/adc_32.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:33 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 31 title "Raw data" with impulses

set title "Channel 33  Mean = 65.706989  Std Dev = 46.976707  Total = 3935.000000"
set autoscale
set yrange [0:2287]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/adc_33.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:34 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 32 title "Raw data" with impulses

set title "Channel 34  Mean = 70.667889  Std Dev = 50.012892  Total = 3818.000000"
set autoscale
set yrange [0:2287]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/adc_34.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:35 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 33 title "Raw data" with impulses

set title "Channel 35  Mean = 68.194521  Std Dev = 48.380584  Total = 3979.000000"
set autoscale
set yrange [0:2287]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/adc_35.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:36 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 34 title "Raw data" with impulses

set title "Channel 36  Mean = 62.066227  Std Dev = 44.772533  Total = 3337.000000"
set autoscale
set yrange [0:2287]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/adc_36.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:37 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 35 title "Raw data" with impulses

set title "Channel 37  Mean = 61.852324  Std Dev = 36.963243  Total = 2431.000000"
set autoscale
set yrange [0:2287]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/adc_37.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:38 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 36 title "Raw data" with impulses

set title "Channel 38  Mean = 66.435626  Std Dev = 39.754369  Total = 2268.000000"
set autoscale
set yrange [0:2287]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/adc_38.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:39 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 37 title "Raw data" with impulses

set title "Channel 39  Mean = 66.401386  Std Dev = 37.167660  Total = 1587.000000"
set autoscale
set yrange [0:2287]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/adc_39.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:40 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 38 title "Raw data" with impulses

set title "Channel 40  Mean = 59.911128  Std Dev = 28.525115  Total = 1339.000000"
set autoscale
set yrange [0:2287]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/adc_40.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:41 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 39 title "Raw data" with impulses

set title "Channel 41  Mean = 83.836369  Std Dev = 54.674589  Total = 3239.000000"
set autoscale
set yrange [0:2287]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/adc_41.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:42 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 40 title "Raw data" with impulses

set title "Channel 42  Mean = 79.119617  Std Dev = 60.396999  Total = 3762.000000"
set autoscale
set yrange [0:2287]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/adc_42.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:43 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 41 title "Raw data" with impulses

set title "Channel 43  Mean = 88.227918  Std Dev = 59.494551  Total = 3238.000000"
set autoscale
set yrange [0:2287]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/adc_43.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:44 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 42 title "Raw data" with impulses

set title "Channel 44  Mean = 68.841865  Std Dev = 43.545618  Total = 3282.000000"
set autoscale
set yrange [0:2287]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/adc_44.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:45 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 43 title "Raw data" with impulses

set title "Channel 45  Mean = 70.688544  Std Dev = 43.327907  Total = 2514.000000"
set autoscale
set yrange [0:2287]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/adc_45.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:46 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 44 title "Raw data" with impulses

set title "Channel 46  Mean = 195.947401  Std Dev = 205.575773  Total = 11293.000000"
set autoscale
set yrange [0:2287]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/adc_46.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:47 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 45 title "Raw data" with impulses

set title "Channel 47  Mean = 61.762238  Std Dev = 29.819192  Total = 1001.000000"
set autoscale
set yrange [0:2287]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/adc_47.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:48 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 46 title "Raw data" with impulses

set title "Channel 48  Mean = 132.789267  Std Dev = 138.612673  Total = 9951.000000"
set autoscale
set yrange [0:2287]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/adc_48.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:49 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 47 title "Raw data" with impulses

set title "Channel 49  Mean = 506.799090  Std Dev = 430.395971  Total = 11204.000000"
set autoscale
set yrange [0:2287]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/adc_49.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:50 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 48 title "Raw data" with impulses

set title "Channel 50  Mean = 483.629465  Std Dev = 423.067969  Total = 12347.000000"
set autoscale
set yrange [0:2287]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/adc_50.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:51 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 49 title "Raw data" with impulses

set title "Channel 51  Mean = 469.919712  Std Dev = 410.748426  Total = 11683.000000"
set autoscale
set yrange [0:2287]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/adc_51.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:52 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 50 title "Raw data" with impulses

set title "Channel 52  Mean = 443.979604  Std Dev = 391.291300  Total = 11473.000000"
set autoscale
set yrange [0:2287]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/adc_52.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:53 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 51 title "Raw data" with impulses

set title "Channel 53  Mean = 420.668516  Std Dev = 387.446432  Total = 12438.000000"
set autoscale
set yrange [0:2287]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/adc_53.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:54 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 52 title "Raw data" with impulses

set title "Channel 54  Mean = 383.141259  Std Dev = 362.283587  Total = 12155.000000"
set autoscale
set yrange [0:2287]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/adc_54.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:55 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 53 title "Raw data" with impulses

set title "Channel 55  Mean = 395.983539  Std Dev = 372.789834  Total = 11968.000000"
set autoscale
set yrange [0:2287]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/adc_55.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:56 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 54 title "Raw data" with impulses

set title "Stefan Simion's Empty Histogram (tm). Formerly known as channel 56"
set autoscale
set yrange [0:2287]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/adc_56.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:57 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 55 title "Raw data" with impulses

set title "Channel 57  Mean = 309.835354  Std Dev = 313.244373  Total = 9815.000000"
set autoscale
set yrange [0:2287]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/adc_57.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:58 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 56 title "Raw data" with impulses

set title "Channel 58  Mean = 130.053476  Std Dev = 135.286742  Total = 9163.000000"
set autoscale
set yrange [0:2287]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/adc_58.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:59 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 57 title "Raw data" with impulses

set title "Channel 59  Mean = 129.446190  Std Dev = 123.658900  Total = 7638.000000"
set autoscale
set yrange [0:2287]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/adc_59.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:60 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 58 title "Raw data" with impulses

set title "Channel 60  Mean = 123.500258  Std Dev = 117.863926  Total = 7740.000000"
set autoscale
set yrange [0:2287]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/adc_60.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:61 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 59 title "Raw data" with impulses

set title "Channel 61  Mean = 127.074951  Std Dev = 130.813987  Total = 8739.000000"
set autoscale
set yrange [0:2287]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/adc_61.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:62 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 60 title "Raw data" with impulses

set title "Channel 62  Mean = 113.971407  Std Dev = 114.492752  Total = 8114.000000"
set autoscale
set yrange [0:2287]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/adc_62.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:63 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 61 title "Raw data" with impulses

set title "Channel 63  Mean = 123.921907  Std Dev = 118.163382  Total = 7363.000000"
set autoscale
set yrange [0:2287]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/adc_63.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:64 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 62 title "Raw data" with impulses

set title "Channel 64  Mean = 107.335514  Std Dev = 102.925966  Total = 7338.000000"
set autoscale
set yrange [0:2287]
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/adc_64.png"
plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:65 title "Histogram" with histeps, "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 63 title "Raw data" with impulses

set title "All channels"
set xlabel "ADC counts"
set ylabel "Count"
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/all_in_one.png"
set autoscale
set yrange [0:2287]
set key spacing 0.5
plot\
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:2 title "Channel 1" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:3 title "Channel 2" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:4 title "Channel 3" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:5 title "Channel 4" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:6 title "Channel 5" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:7 title "Channel 6" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:8 title "Channel 7" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:9 title "Channel 8" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:10 title "Channel 9" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:11 title "Channel 10" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:12 title "Channel 11" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:13 title "Channel 12" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:14 title "Channel 13" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:15 title "Channel 14" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:16 title "Channel 15" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:17 title "Channel 16" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:18 title "Channel 17" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:19 title "Channel 18" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:20 title "Channel 19" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:21 title "Channel 20" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:22 title "Channel 21" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:23 title "Channel 22" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:24 title "Channel 23" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:25 title "Channel 24" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:26 title "Channel 25" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:27 title "Channel 26" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:28 title "Channel 27" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:29 title "Channel 28" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:30 title "Channel 29" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:31 title "Channel 30" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:32 title "Channel 31" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:33 title "Channel 32" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:34 title "Channel 33" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:35 title "Channel 34" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:36 title "Channel 35" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:37 title "Channel 36" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:38 title "Channel 37" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:39 title "Channel 38" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:40 title "Channel 39" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:41 title "Channel 40" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:42 title "Channel 41" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:43 title "Channel 42" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:44 title "Channel 43" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:45 title "Channel 44" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:46 title "Channel 45" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:47 title "Channel 46" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:48 title "Channel 47" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:49 title "Channel 48" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:50 title "Channel 49" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:51 title "Channel 50" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:52 title "Channel 51" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:53 title "Channel 52" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:54 title "Channel 53" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:55 title "Channel 54" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:56 title "Channel 55" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:57 title "Channel 56" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:58 title "Channel 57" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:59 title "Channel 58" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:60 title "Channel 59" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:61 title "Channel 60" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:62 title "Channel 61" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:63 title "Channel 62" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:64 title "Channel 63" with histeps, \
"/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:65 title "Channel 64" with histeps

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
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/mean_2d.png"
splot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/summary_2d.csv" using 1:2:3 title ""
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
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/mean_normalized_2d.png"
splot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/summary_2d.csv" using 1:2:9 title ""
unset pm3d
set grid

set title "Histograms"
unset grid
set xlabel "Channel"
set ylabel "X"
set autoscale
set xrange [0.5:64.5]
set yrange [20:1913]
set pm3d at b corners2color c1
set view map
set style data pm3d
set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist_2d.png"
splot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist_2d.csv" using 1:2:3 title ""
unset pm3d
set grid

set output "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/all_in_8x8.png"
unset grid
unset title
unset xlabel
unset ylabel
unset key
set autoscale
set yrange [0:2287]
set noxtics
set noytics
set format x ""
set format y ""
set multiplot layout 8,8

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:2 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 0 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:3 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 1 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:4 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 2 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:5 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 3 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:6 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 4 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:7 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 5 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:8 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 6 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:9 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 7 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:10 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 8 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:11 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 9 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:12 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 10 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:13 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 11 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:14 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 12 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:15 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 13 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:16 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 14 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:17 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 15 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:18 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 16 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:19 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 17 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:20 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 18 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:21 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 19 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:22 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 20 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:23 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 21 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:24 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 22 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:25 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 23 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:26 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 24 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:27 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 25 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:28 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 26 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:29 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 27 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:30 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 28 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:31 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 29 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:32 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 30 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:33 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 31 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:34 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 32 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:35 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 33 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:36 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 34 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:37 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 35 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:38 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 36 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:39 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 37 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:40 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 38 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:41 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 39 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:42 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 40 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:43 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 41 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:44 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 42 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:45 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 43 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:46 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 44 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:47 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 45 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:48 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 46 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:49 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 47 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:50 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 48 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:51 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 49 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:52 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 50 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:53 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 51 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:54 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 52 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:55 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 53 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:56 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 54 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:57 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 55 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:58 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 56 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:59 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 57 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:60 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 58 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:61 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 59 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:62 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 60 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:63 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 61 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:64 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 62 with impulses

plot "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/hist.csv" using 1:65 with histeps,  "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA//Run_0000074/USB_39/PMT_6/raw.csv" index 63 with impulses

unset multiplot
