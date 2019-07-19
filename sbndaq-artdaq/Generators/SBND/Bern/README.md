# febdrv
DAQ software package to use with Bern FEB based CRT redout

the software packages received from Igor Kreslo.
This version of the software has to used with the latest firmware FEB.
--> coincidence flag included

ConvertBin2Root.C 
	--> convert binary file to root files
root file format:
        TTree *tr=new TTree("mppc","mppc");
        tr->Branch("mac5",&mac5,"mac5/b");
        tr->Branch("flags",&flags,"flags/b");
        tr->Branch("chg",adc,"chg[32]/s");
        tr->Branch("ts0",&ts0,"ts0/i");
        tr->Branch("ts1",&ts1,"ts1/i");
        tr->Branch("coincidence",&coinc,"coinc/i");

