//R__LOAD_LIBRARY(/usr/lib64/mysql/libmysqlclient.so)

#include <map>
#include <TROOT.h>
#include <TChain.h>
#include <TStyle.h>
#include <TNtuple.h>
#include <TFile.h>
#include <TString.h>
#include <TF1.h>
#include <stdio.h>
#include <iostream>
#include "functions.C"
#include "fstream"
#include "sstream"
#include <string>
#include <vector>
#include <sstream>
#include <time.h>
#include <dirent.h>
using namespace std;

/////need root version >=5.26 to run

void sync_check(const int subrunnumber=171)
{

  const int usbnum = 39;
  const int smodnum = 1;
  //const char* lab = "1";
  //const char* date = "20190709";
  //const int howmanyrun=1;
  //const char* onlinedb = "doublechooz_ov_near";
  time_t start_t;
  time( &start_t );

  //gROOT->Reset();
  gStyle->SetOptFit(1111);
  gStyle->SetOptStat(1111111);

  gErrorIgnoreLevel = kWarning;

  char RunDir[512];
  sprintf(RunDir,"/home/nfs/icarus/DAQ_DevAreas/DAQ_17Feb2023_MM/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA/Run_0000%i",subrunnumber);
  
  int numberofboardused = 0;
  int pmtboardnumber[100];

  char sfile[100];
  char sfilename[512];
  char bfilename[512];

  //define chain for the signal and baseline

  TChain schain("ntuple_sig");
  TChain bchain("ntuple_sig");

  vector<string> fileList;
  long sfile_name;
  char filepath[512];
  sprintf(filepath,"%s/binary",RunDir);

  scanFiles(fileList,filepath);  //this now produces the list fo files with usb as the last character
 
  //for(int i=0; i<fileList.size(); i++){
  //  cout << "fileList[" << i << "]: " << fileList[i] << endl;
  //}
 

  for(int i=0;i<fileList.size();i++){
    sprintf(sfilename,"%s/binary/%s",RunDir,fileList[i].c_str());
    schain.Add(sfilename);
  }
  sprintf(bfilename,"%s/binary/baseline_39.root",RunDir);
  float stime_sec_high,stime_sec_low,stime_16ns_high,stime_16ns_low, smodule, slen;
  
  //change k to maximum file number to run over
  //for(Int_t k = 0; k<howmanyrun; k++) { 
  //  sprintf(sfilename, "%s%d%s", sfile, k,".root");
  //  schain.Add(sfilename);
  //}
  //cout << "sfilename: " << sfilename << endl;
  //cout << "bfilename: " << bfilename << endl;
 //linking to ntuples

  schain.SetBranchStatus( "*", 1 ); 	  
  schain.SetBranchStatus( "smodule", 1 );	  
  schain.SetBranchStatus( "slen", 1 );	  
  schain.SetBranchStatus( "stime_sec_low", 1 );	  
  schain.SetBranchStatus( "stime_sec_high", 1 );	  
  schain.SetBranchStatus( "stime_16ns_low", 1 );	  
  schain.SetBranchStatus( "stime_16ns_high", 1 );

  schain.SetBranchAddress("stime_sec_high",&stime_sec_high);
  schain.SetBranchAddress("stime_sec_low",&stime_sec_low);
  schain.SetBranchAddress("stime_16ns_high",&stime_16ns_high);
  schain.SetBranchAddress("stime_16ns_low",&stime_16ns_low);
  schain.SetBranchAddress("smodule",&smodule);
  schain.SetBranchAddress("slen",&slen); //added for room 1 data
 
  bchain.SetBranchAddress("smodule",&smodule);
  TCanvas *c1 = new TCanvas( "c1", "" );  

  cout << "schain size = " << schain.GetEntries()<<endl;

////////Quick and dirty max time test///////////////
uint32_t max[10] = {0};
int evt_count[10] = {0};
for(int i=0;i<schain.GetEntries();i++){
  schain.GetEntry(i);
  for(int j=0; j<10; j++){
    uint32_t clocktime = stime_16ns_high*65536 + stime_16ns_low; 
    if(smodule == j+1){
      if(clocktime > max[j]){
        max[j] = clocktime;
      }
    evt_count[j]++;
    }
  }
}

for(int k=0; k<10; k++){
  cout << "Max 32-bit counter time for PMT " << k+1 << " is " << max[k] << ", or " << max[k]*(16.0e-9) << " seconds. (" << evt_count[k] << " events)\n";
}
///////////////////////////////////////////////////
 
  time_t end_t;
  time( &end_t);

  cout << "Total runtime: " << difftime(end_t,start_t) << " seconds\n";

  exit(0);
  
}
 
