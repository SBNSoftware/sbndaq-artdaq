//R__LOAD_LIBRARY(/usr/lib64/mysql/libmysqlclient.so)

#include <map>
#include <TROOT.h>
#include <TH1D.h>
#include <TChain.h>
#include <TStyle.h>
#include <TNtuple.h>
#include <TFile.h>
#include <TGraphErrors.h>
#include <TH2F.h>
#include <TCanvas.h>
#include <TString.h>
#include <TF1.h>
#include <stdio.h>
#include <iostream>
#include "getgainconst22.C"
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

void muon_22(const char* lab="1", const char *date="20190709", const int subrunnumber=65, const int howmanyrun=1, const char* onlinedb="doublechooz_ov_near", const int usbnum = 39, const int smodnum = 1)
{

  time_t start_t;
  time( &start_t );

  //gROOT->Reset();
  gStyle->SetOptFit(1111);
  gStyle->SetOptStat(1111111);

  gErrorIgnoreLevel = kWarning;
 
  bool debug = false;		      // turn on/off code debugging comments and printout
  bool usesql = false; 
  bool usefcl = true;
  bool test_calib = false; //if this is active the code reads in previously calculated gain constants
			   //and attempts to get ADC values closer to the target

 
  if(debug){
    cout << "date you input:  " << date << endl;
    cout << "subrunnumber you selected:  " << subrunnumber << endl;
    cout << "howmanyrun you selected:  " << howmanyrun << endl;
  }

  char RunDir[512];
  sprintf(RunDir,"/home/nfs/icarus/DAQ_DevAreas/DAQ_17Feb2023_MM/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA/Run_00000%i",subrunnumber);
  

  double cutnpe[100][65];   //this line specify the software cut in terms of pe that you would like to apply
  double time_offset=1e10;
  double ADC2PE[100][65];
  double smean[100][65];
  double srms[100][65];
  double ADCv[100][65];
  double timediff=9999.;

  const int BIG_INT = 327680; //trying to test absolute offsets

  int three_counter = 0; //for testing
  int seven_counter = 0;

  int TDIFF_CUT = 2;
  
  int numberofboardused = 0;
  int stripnum[64];
  int time_in_s;
  int index=0;
  int index2=0;
  int topstrip=-1;
  int botstrip=-1;
  int topstrip2=-1;
  int botstrip2=-1;
  int topstrip3=-1;
  int botstrip3=-1;
  int topstrip4=-1;
  int botstrip4=-1;
  int events_strip[64][16];
  int events[16];
  int pmtboardnumber[100];
  int pmt_layer[100];
  char pmt_xy[100];
  int gain_v[100][65];
  int timecoincidence_counter = 0;
  int timecoincidence_counter2 = 0;

  char bnames[65][8];
  char snames[65][8];
  char sfile[100];
  char sfilename[512];
  char bfilename[512];
  char histname[50];
  char cutbaseline[20];
  char cutbaseline2[20];
  char modulename[10];

  float gainused[100][64];  //<--MySQL stuff
  float gain_new[100][64];
  float channel_num[16];
  float bfloats[65];
  float stime_sec_high,stime_sec_low,stime_16ns_high,stime_16ns_low, smodule, slen;
  float stime_sec_high_c,stime_sec_low_c,stime_16ns_high_c,stime_16ns_low_c, smodule_c, number_c,adc_c;
  float stime_sec_high_m,stime_sec_low_m,stime_16ns_high_m,stime_16ns_low_m, smodule_m,adc_bot_m,adc_top_m,strip_bot_m,strip_top_m;
  float stime_sec_high_m2,stime_sec_low_m2,stime_16ns_high_m2,stime_16ns_low_m2, smodule_m2,adc_bot_m2,adc_top_m2,strip_bot_m2,strip_top_m2;
  float stime_sec_high_m3,stime_sec_low_m3,stime_16ns_high_m3,stime_16ns_low_m3, smodule_m3,adc_bot_m3,adc_top_m3,strip_bot_m3,strip_top_m3;
  float stime_sec_high_m4,stime_sec_low_m4,stime_16ns_high_m4,stime_16ns_low_m4, smodule_m4,adc_bot_m4,adc_top_m4,strip_bot_m4,strip_top_m4;
  float index_top_m, index_top_m2, index_top_m3, index_top_m4;
  float index_bot_m, index_bot_m2, index_bot_m3, index_bot_m4;

  float adc_ratio_m,adc_ratio_m2,adc_ratio_m3,adc_ratio_m4;

  float ratio_cut = 0.8;

  float sfloats[65];
  float z[64];					//positions of counters
  float fiberlength[32];
  float counter_num[64];
  float bot_adc=-1;
  float top_adc=-1;
  float bot_adc2=-1;
  float top_adc2=-1;

  float bot_adc3=-1;
  float top_adc3=-1;

  float bot_adc4=-1;
  float top_adc4=-1;

  float max_adc;
  float max_adc2;
  float max_adc3;
  float max_adc4;


  float sum_adc[64];
  float num_adc[64];
  float sum_adc_c[16];
  float num_adc_c[16];
  float sfloatsmax=0; 
  float pecut = 0.333;  ////// cut in npe /////
  float ADC_calib_target = 300.;
  float baseline_channel[100][65];

  typedef std::multimap<int,int> mymap_t;

  mymap_t maptimes; 
  mymap_t maptimes2;

  //histogram definition

  TH1F *ADCvmean[100];
  TH1F *gain_p[100];

  TH1F *ADC[100][64]; //histograms of adc counts for 8 different counter positions.

  TH1F *allADCv = new TH1F("allADCv","Mean ADC values, all channels",100,0,1000);
  TH1F *allgain_p = new TH1F("allgain_p","Calculated gain, all channels",10,10,40);  

  TH1F *bmod		= new TH1F("bmod","baseline m",100,1500,1600);
  TH1F *smod		= new TH1F("smod","ADC",100,0,800);
  TH1F *smod2		= new TH1F("smod2","ADC",100,0,800);
  TH1F *smod3		= new TH1F("smod3","ADC",100,0,800);
  TH1F *smod4		= new TH1F("smod4","ADC",100,0,800);


  //define chain for the signal and baseline

  TChain schain("ntuple_sig");
  TChain bchain("ntuple_sig");

  //sprintf(sfile,"/a/data/morningside/DoubleChooz/%s/Run_%s_%d/signal_",lab,date,subrunnumber);
  //sprintf(bfilename,"/a/data/morningside/DoubleChooz/%s/Run_%s_%d/baselines.root",lab,date,subrunnumber);
  
  //sprintf(sfile,"%s/binary/signal_",RunDir);
  //sprintf(bfilename,"%s/binary/baselines.root",RunDir);

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
  
  //change k to maximum file number to run over
  //for(Int_t k = 0; k<howmanyrun; k++) { 
  //  sprintf(sfilename, "%s%d%s", sfile, k,".root");
  //  schain.Add(sfilename);
  //}
  cout << "sfilename: " << sfilename << endl;
  cout << "bfilename: " << bfilename << endl;

  bchain.Add(bfilename);


  TNtuple *module_coinc = new TNtuple("module_coinc","module_coincidence","strip_bot_m:strip_top_m:adc_bot_m:adc_top_m:stime_sec_high_m:stime_sec_low_m:stime_16ns_high_m:stime_16ns_low_m:smodule_m:adc_ratio_m"); 
  TNtuple *module_coinc2 = new TNtuple("module_coinc2","module_coincidence2","strip_bot_m2:strip_top_m2:adc_bot_m2:adc_top_m2:stime_sec_high_m2:stime_sec_low_m2:stime_16ns_high_m2:stime_16ns_low_m2:smodule_m2:adc_ratio_m2");
  TNtuple *module_coinc3 = new TNtuple("module_coinc3","module_coincidence3","strip_bot_m3:strip_top_m3:adc_bot_m3:adc_top_m3:stime_sec_high_m3:stime_sec_low_m3:stime_16ns_high_m3:stime_16ns_low_m3:smodule_m3:adc_ratio_m3");
  TNtuple *module_coinc4 = new TNtuple("module_coinc4","module_coincidence4","strip_bot_m4:strip_top_m4:adc_bot_m4:adc_top_m4:stime_sec_high_m4:stime_sec_low_m4:stime_16ns_high_m4:stime_16ns_low_m4:smodule_m4:adc_ratio_m4");


  TNtuple *counter = new TNtuple("counter","counter events","adc_c:number_c:stime_sec_high_c:stime_sec_low_c:stime_16ns_high_c:stime_16ns_low_c:smodule_c"); 

  module_coinc->SetDirectory(0);                                 // I am not sure why I need this line.
  counter->SetDirectory(0);
  
  counterpos(z);						//function that give the position in cm of the counters

  if(debug) {
    for (int j = 0; j < 32; j++) {
      printf("--paddle number=%d--position=%f\n", j, z[j]);
    }
  }


  paddle(counter_num, channel_num);  //function that give paddle number->channel number and viceversa

  if(debug) {
    for (int j = 0; j < 32; j++) {
      printf("--paddle number=%d--channel number=%f\n", j, counter_num[j]);
    }
    for (int j = 0; j < 16; j++) {
      printf("--channel number=%d--paddle number=%f\n", j, channel_num[j]);
    }
  }

  
  lenghtpigtail(fiberlength);
  if(debug){
    for(Int_t k=0; k<32; k++){
      printf("fiber number: [%d] has a length of %f cm\n",k,fiberlength[k]);
    }
  }


  ch2strip(stripnum,debug);
  
  if(debug){
    for(Int_t k=0; k<64; k++){
      printf("stripnum[%d] = %d\n",k,stripnum[k]);
    }
  }


  //Reading in channels
  //string channelfile = "test_316v2.txt";  
  string channelfile = "test_icarus.txt";
  int num_pmt = get_num_pmt(channelfile.c_str()); 
  cout << "Number of PMTs: " << num_pmt << endl;
  int num_implemented_channels = get_num_implemented_channels(channelfile.c_str());
  //Grab a vector of a vector of the channels
  vector<vector<int>> channels_in = get_channels_on_pmt(channelfile.c_str(), num_pmt, num_implemented_channels);
   
  //for(int jj=0; jj<12; jj++){
  //  int j = channels_in[0][jj]-1;
  //  for(int kk=12; kk<24; kk++){
  //    int k = channels_in[0][kk]-1;
  //    cout << "jj: " << jj << "\tkk: " << kk << "\tj+1: " << j+1 << "\tk+1: " << k+1 << "\toverlap: " << fabs(kk-12-0.5-jj) << endl; 
  //  }
  //}
 
/*for(int i=0;i<channels_in.size();i++){
    cout << "channels_in[" << i << "]: ";
    for(int j=0;j<channels_in[i].size();j++){
       cout << channels_in[i][j] << " ";
    }
    cout << endl;
  }*/

 

  get_pmt_xy_layer(channelfile.c_str(), num_pmt, pmt_xy, pmt_layer);


  usb2pmt(usbnum, smodnum, pmtboardnumber);

  if(usesql){ //Load gain constants from SQL table
    //getgainconst(onlinedb,usbnum,pmtboardnumber,gainused,&numberofboardused,debug); 
  }
  else if (usefcl){ //Read gain constants from fcl file
    //string fclfilename = "fcl_test.fcl";
    string fclfilename = "crt_daq.fcl";
    getgainfcl(fclfilename,pmtboardnumber,gainused,&numberofboardused);
  }
  else{ //hardcoded values
    numberofboardused = 7;
    pmtboardnumber[1]=1;
    pmtboardnumber[2]=2;
    pmtboardnumber[3]=3;
    pmtboardnumber[4]=4;
    pmtboardnumber[5]=5;
    pmtboardnumber[6]=6;
    pmtboardnumber[7]=55;
  }
  cout << numberofboardused << " boards used\n" << endl;

  for(int i=1; i<=num_pmt; i++){
    cout << "PMT number: " << pmtboardnumber[i] << ", " << pmt_xy[i] << ", layer " << pmt_layer[i] << endl;
  }

  if(debug) {
    for(int i=1; i<=4; i++){
      for (int j = 0; j < 64; j++) {
			printf("board -- %d, --PMT channel=%d--gain constant applied=%f\n", pmtboardnumber[i], j+1, gainused[pmtboardnumber[i]][j]);
      }
    }
  }
  
  //Get new gain constants from text file
  if(test_calib){
    ifstream gain_in("gain_file.txt");
    string sline;
    istringstream iss;
      if ( !gain_in ) return;
	
      while ( getline( gain_in, sline ) ) 
	{
	  iss.str( sline );             
	  Int_t fin_pmt, fin_ch; Double_t fin_gain;
			
	  iss >> fin_pmt >> fin_ch >> fin_gain;

	  cout << fin_pmt << " " << fin_ch << " " << fin_gain << endl;

	  gain_new[fin_pmt][fin_ch] = fin_gain;

	  iss.clear();
        }
      gain_in.clear();      
      gain_in.close();

  if(debug) {
    for(int i=1; i<=numberofboardused; i++){
      for (int j = 0; j < 64; j++) {
	if(gain_new[pmtboardnumber[i]][j] != 0){
	  printf("board -- %d, --PMT channel=%d--new gain constant applied=%f, %f\n", pmtboardnumber[i], j+1, gain_new[pmtboardnumber[i]][j], gainused[pmtboardnumber[i]][j]);
        }
      }
    }
  }
  

  }




  /////////////////////////////////////////////////////////////////////////////////////

  //creating histograms 

  for(int i=1;i<100;i++){
    for(int j=0;j<64;j++){
      sprintf(histname,"mod=%d-ch=%d",i,j);
      ADC[i][j]= new TH1F(histname,histname,100,0,1000);
      cutnpe[i][j]=0; //for now
    }
  }


  for(int i=1;i<56;i++){
    sprintf(histname,"ADC_mean_value_mod=%d",i);
    ADCvmean[i]= new TH1F(histname,histname,100,0,1000);
    sprintf(histname,"gain_distribution_mod=%d",i);
    gain_p[i]= new TH1F(histname,histname,10,10,40);
  }

  int ch_per_pmt = channels_in[0].size();
  //TH2D *channel_events = new TH2D("channel_events", "Events per channel",ch_per_pmt,0,ch_per_pmt,numberofboardused,0,numberofboardused);
  TH2D *channel_events = new TH2D("channel_events", "Events per channel",64,0,64,numberofboardused,0,numberofboardused);
  TH2D *channel_adc = new TH2D("channel_adc", "Avg. ADC per channel",64,0,64,numberofboardused,0,numberofboardused);
 
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

  module_coinc->SetBranchAddress("stime_sec_high_m",&stime_sec_high_m);
  module_coinc->SetBranchAddress("stime_sec_low_m",&stime_sec_low_m);
  module_coinc->SetBranchAddress("stime_16ns_high_m",&stime_16ns_high_m);
  module_coinc->SetBranchAddress("stime_16ns_low_m",&stime_16ns_low_m);
  module_coinc->SetBranchAddress("smodule_m",&smodule_m);
  module_coinc->SetBranchAddress("adc_bot_m",&adc_bot_m);
  module_coinc->SetBranchAddress("adc_top_m",&adc_top_m);
  module_coinc->SetBranchAddress("strip_bot_m",&strip_bot_m);
  module_coinc->SetBranchAddress("strip_top_m",&strip_top_m);
  module_coinc->SetBranchAddress("adc_ratio_m",&adc_ratio_m);
  
  module_coinc2->SetBranchAddress("stime_sec_high_m2",&stime_sec_high_m2);
  module_coinc2->SetBranchAddress("stime_sec_low_m2",&stime_sec_low_m2);
  module_coinc2->SetBranchAddress("stime_16ns_high_m2",&stime_16ns_high_m2);
  module_coinc2->SetBranchAddress("stime_16ns_low_m2",&stime_16ns_low_m2);
  module_coinc2->SetBranchAddress("smodule_m2",&smodule_m2);
  module_coinc2->SetBranchAddress("adc_bot_m2",&adc_bot_m2);
  module_coinc2->SetBranchAddress("adc_top_m2",&adc_top_m2);
  module_coinc2->SetBranchAddress("strip_bot_m2",&strip_bot_m2);
  module_coinc2->SetBranchAddress("strip_top_m2",&strip_top_m2);
  module_coinc2->SetBranchAddress("adc_ratio_m2",&adc_ratio_m2);

  module_coinc3->SetBranchAddress("stime_sec_high_m3",&stime_sec_high_m3);
  module_coinc3->SetBranchAddress("stime_sec_low_m3",&stime_sec_low_m3);
  module_coinc3->SetBranchAddress("stime_16ns_high_m3",&stime_16ns_high_m3);
  module_coinc3->SetBranchAddress("stime_16ns_low_m3",&stime_16ns_low_m3);
  module_coinc3->SetBranchAddress("smodule_m3",&smodule_m3);
  module_coinc3->SetBranchAddress("adc_bot_m3",&adc_bot_m3);
  module_coinc3->SetBranchAddress("adc_top_m3",&adc_top_m3);
  module_coinc3->SetBranchAddress("strip_bot_m3",&strip_bot_m3);
  module_coinc3->SetBranchAddress("strip_top_m3",&strip_top_m3);
  module_coinc3->SetBranchAddress("adc_ratio_m3",&adc_ratio_m3);

  module_coinc4->SetBranchAddress("stime_sec_high_m4",&stime_sec_high_m4);
  module_coinc4->SetBranchAddress("stime_sec_low_m4",&stime_sec_low_m4);
  module_coinc4->SetBranchAddress("stime_16ns_high_m4",&stime_16ns_high_m4);
  module_coinc4->SetBranchAddress("stime_16ns_low_m4",&stime_16ns_low_m4);
  module_coinc4->SetBranchAddress("smodule_m4",&smodule_m4);
  module_coinc4->SetBranchAddress("adc_bot_m4",&adc_bot_m4);
  module_coinc4->SetBranchAddress("adc_top_m4",&adc_top_m4);
  module_coinc4->SetBranchAddress("strip_bot_m4",&strip_bot_m4);
  module_coinc4->SetBranchAddress("strip_top_m4",&strip_top_m4);
  module_coinc4->SetBranchAddress("adc_ratio_m4",&adc_ratio_m4);



  counter->SetBranchAddress("adc_c", &adc_c);
  counter->SetBranchAddress("number_c", &number_c);
  counter->SetBranchAddress("stime_sec_high_c",& stime_sec_high_c);
  counter->SetBranchAddress("stime_sec_low_c",&stime_sec_low_c);
  counter->SetBranchAddress("stime_16ns_high_c",&stime_16ns_high_c);
  counter->SetBranchAddress("stime_16ns_low_c",&stime_16ns_low_c);
  counter->SetBranchAddress("smodule_c",&smodule_c);

  TCanvas *c1 = new TCanvas( "c1", "" );  

  for (int j=1; j<=numberofboardused; j++) //to be replaced after query on mysql
    {
      sprintf(modulename,"smodule==%d",pmtboardnumber[j]); 

      for (int i = 0; i < 64; i++)                  // xxxxx this line changed from (int i = 1; i <=64; i++)
		{

	  	//cout << Form("smodule==%d",pmtboardnumber[j]) << endl;

	  	sprintf(snames[i], "s%d", i+1);

	  	schain.SetBranchAddress(snames[i],&sfloats[i]);  


	  	//getting baseline values
	  	sprintf(bnames[i], "s%d", i+1);
	  	bchain.SetBranchAddress(bnames[i],&bfloats[i]);

	  	bchain.Project(Form("bmod"),bnames[i],Form("smodule==%d",pmtboardnumber[j]));
	  	//bchain.Project(Form("bmod"),bnames[i],modulename); //modulename string was getting changed for unknown reasons


	  	baseline_channel[pmtboardnumber[j]][i] = bmod->GetMean();





	  if(debug) {cout << modulename << " channel " << i << " baseline=" << baseline_channel[pmtboardnumber[j]][i]; }

	  sprintf(cutbaseline,"s%d-%f<30",i+1,baseline_channel[pmtboardnumber[j]][i]);
	  sprintf(snames[i], "s%d-%f", i+1, baseline_channel[pmtboardnumber[j]][i]);
	  schain.Project(Form("smod"),snames[i],cutbaseline,"",1000000,0); 

	  //smod->Draw();
	  //c1->Print(Form("smods/smod_%d_%d.png",j,i));

	  //sprintf(cutbaseline2,"s%d-%f>-999",i+1,baseline_channel[pmtboardnumber[j]][i]);
	  //schain.Project(Form("smod2"),snames[i],cutbaseline2);
	  //smod2->Draw();
	  //c1->Print(Form("smods/smod2_%d_%d.png",j,i));

	  smean[j][i]= smod->GetMean();
	  srms[j][i] = smod->GetRMS();
	  //ADC2PE[j][i] = trunc_mean(smod);	    //truncated mean
	  ADC2PE[j][i] = 20.0; //testing

	  if(debug){ cout << "--mean=" << smean[j][i] <<" trunc.=" << ADC2PE[j][i] << endl; }
          bmod->Reset();
          smod->Reset();
          cutnpe[pmtboardnumber[j]][i] = ADC2PE[j][i]*pecut;  //ADC2PE[j][i] is big a lot of muons
	}
    }

  time_t adc2pe_t;
  time( &adc2pe_t );

  cout << "Time to calculate adc2pe: " << difftime(adc2pe_t,start_t) << " seconds\n";



  for (Int_t i=0;i<64;i++){
    for (Int_t j=0;j<16;j++){
      events_strip[i][j]=0;
      events[j]=0;
      num_adc[i]=0;
      sum_adc[i]=0;
      sum_adc_c[j]=0;
      num_adc_c[j]=0;
    }
  }
    
  cout << "schain size = " << schain.GetEntries()<<endl;

  //do_offsets(schain);
//////////////Start Offsets/////////////////////////
/*
  cout << "Doing offsets: " << endl;
  
  //int pmt_no[4] = {3,7,1,2}; //this will be pmtboardnum when the db works
  int pmt_no[6] = {1,2,3,4,5,6};
  //int numboards = sizeof(pmt_no)/sizeof(pmt_no[0]); //and this will be numberofboardsused
  //int numpairs = numboards*(numboards-1)/2; //nC2 = n*(n-1)/2
  //int numpairs = numberofboardused*(numberofboardused-1)/2; 
  int numpairs = 6*(6-1)/2; 
  //cout << "numpairs = " << numpairs << endl;

  TH1D *h_off[numpairs]; 
  TH1D *h_off_high[numpairs]; //for testing
  //TH1D *h_off_abs[numpairs];  //testing
  int offcounter=0;
  
  for(int i=0; i<5; i++){
    for(int j=i; j<5; j++){
      h_off[offcounter] = new TH1D(Form("h_off_%d_%d",pmt_no[i],pmt_no[j+1]),Form("Offset between PMT-%d and PMT-%d",pmt_no[i],pmt_no[j+1]),100000,0.0,100000.0);
      h_off_high[offcounter] = new TH1D(Form("h_off_high_%d_%d",pmt_no[i],pmt_no[j+1]),Form("High offset between PMT-%d and PMT-%d",pmt_no[i],pmt_no[j+1]),80,-40.0,40.0);
      //h_off_abs[offcounter] = new TH1D(Form("h_off_abs_%d_%d",pmt_no[i],pmt_no[j+1]),Form("Absolute offset between PMT-%d and PMT-%d",pmt_no[i],pmt_no[j+1]),BIG_INT,-BIG_INT,BIG_INT);
cout << "Pair "<< offcounter << ": PMT-" << pmt_no[i] << " and PMT-" << pmt_no[j+1] << endl;
      offcounter++;
    }
  }

  vector< Float_t > pack;
  vector< vector< vector<Float_t> > > p_mod; // of the form mod 2, 1, 7, 3
  p_mod.resize(6);

  bool check = 0;

  int num_entries = schain.GetEntries();

  //int offset_loop_size = num_entries; //slower
  int offset_loop_size = 100000; //faster

  for(int i=0; i<offset_loop_size; i++){
    schain.GetEntry(i);
    
    pack.clear();

    pack.push_back(stime_sec_low);
    pack.push_back(stime_sec_high);
    pack.push_back(stime_16ns_low);
    pack.push_back(stime_16ns_high);

    //cout << "i=" << i << "\tstime_sec_low=" << stime_sec_low << "\tstime_sec_high=" << stime_sec_high << "\tstime_16ns_low=" << stime_16ns_low << endl;
    if(i%10000 == 9999){
      check = 1;
      cout << "Offsets " << 100*i/offset_loop_size << "% done\n";
    }

    if(slen >= 3 && slen <= 8){

      if(check == 0){  
        for(int m=0; m<6; m++){ //loop over the pmts
          if(smodule == pmt_no[m]){
	    p_mod[m].push_back(pack); //Add the time packet to the corresponding vector
	  }
	}
      }

   if(check == 1){ //Only doing this loop every 10k entries
   int counter = 0; //for counting which pair we're looking at

	for(int first_pmt=0; first_pmt<5; first_pmt++){
	  int first_size = p_mod[first_pmt].size();  //Number of time packets for the first pmt in the pair (2,1,7)
	  for(int second_pmt = first_pmt+1; second_pmt<6; second_pmt++){
	    int second_size = p_mod[second_pmt].size(); //Number of time packets for the second pmt in the pair (1 7 3, 7 3, 3)

	    for(int ii = 0; ii<first_size; ii++){
	      for(int jj = 0; jj<second_size; jj++){ //Looping over all pairs of timing packets
		if(p_mod[first_pmt][ii].size() == p_mod[second_pmt][jj].size()){ //i.e. if both packets have all 4 entries
		  if(p_mod[first_pmt][ii].at(0) == p_mod[second_pmt][jj].at(0) && p_mod[first_pmt][ii].at(1) == p_mod[second_pmt][jj].at(1)){
		    //i.e. if the stime_sec_low and stime_sec_high are the same
		    h_off[counter]->Fill(p_mod[first_pmt][ii].at(2)-p_mod[second_pmt][jj].at(2));		    
		    h_off_high[counter]->Fill(p_mod[first_pmt][ii].at(3)-p_mod[second_pmt][jj].at(3)); //for testing		    
		    //h_off_abs[counter]->Fill((p_mod[first_pmt][ii].at(3)*65536.+p_mod[first_pmt][ii].at(2))-(p_mod[second_pmt][ii].at(3)*65536.+p_mod[second_pmt][ii].at(2)));
		  }
		}
	      }
	    }

	  counter++;
	  }
	}
	for(int k=0; k<6; k++){p_mod[k].clear();} //make way for the next 10k entries
        check = 0;
      }

    }
  } //end of loop over all entries

  int offsets[numpairs];
  int offsets_high[numpairs];
  //int offsets_abs[numpairs];
  offcounter = 0;

  TCanvas *c1 = new TCanvas( "c1", "" );  

  for(int i=0; i<5; i++){
    for(int j=i; j<5; j++){
      //h_off[offcounter]->Draw("");
      //c1->Print(Form("offset_histos_%d_%d.png",pmt_no[i],pmt_no[j+1]));
      //h_off_high[offcounter]->Draw("");
      //c1->Print(Form("offset_histos_high_%d_%d.png",pmt_no[i],pmt_no[j+1]));
      //h_off_abs[offcounter]->Draw("");
      //c1->Print(Form("offset_histos_abs_%d_%d.png",pmt_no[i],pmt_no[j+1]));
     
 
      offsets[offcounter] = (int)h_off[offcounter]->GetBinLowEdge(h_off[offcounter]->GetMaximumBin());
      offsets_high[offcounter] = (int)h_off_high[offcounter]->GetBinLowEdge(h_off_high[offcounter]->GetMaximumBin()); 
      //offsets_abs[offcounter] = (int)h_off_abs[offcounter]->GetBinLowEdge(h_off_abs[offcounter]->GetMaximumBin()); 
      cout << "Offset between PMT-" << pmt_no[i] << " and PMT-" << pmt_no[j+1] << " is " << offsets[offcounter] << endl;
      cout << "High offset between PMT-" << pmt_no[i] << " and PMT-" << pmt_no[j+1] << " is " << offsets_high[offcounter] << endl;
      //cout << "Absolute offset between PMT-" << pmt_no[i] << " and PMT-" << pmt_no[j+1] << " is " << offsets_abs[offcounter] << endl << endl;

      offcounter++;
    }
  } 
*/
/////////////End Offsets////////////////////////////

////////Quick and dirty max time test///////////////
uint32_t max[6] = {0};
uint32_t max55 = 0;
for(int i=0;i<schain.GetEntries();i++){
  schain.GetEntry(i);
  for(int j=0; j<6; j++){
    uint32_t clocktime = stime_16ns_high*65536 + stime_16ns_low; 
    if(smodule == j+1 && clocktime > max[j]){
      max[j] = clocktime;
    }
  }
}

for(int k=0; k<6; k++){
  cout << "Max 32-bit counter time for PMT " << k+1 << " is " << max[k] << ", or " << max[k]*(16.0e-9) << " seconds.\n";
}
for(int i=0;i<schain.GetEntries();i++){
schain.GetEntry(i);
uint32_t clock55 = stime_16ns_high*65536 + stime_16ns_low;  
  if(smodule == 55 && clock55 > max55){
    max55 = clock55;
  }
}
cout << "Max 32-bit counter time for PMT 55 is " << max55 << ", or " << max55*(16.0e-9) << " seconds.\n";
///////////////////////////////////////////////////


/////Bilayer coincidence//////////////////////////

/*cout << "layersize: " << channels_in[0].size()/2 << endl;
for(int jj = 0; jj <= channels_in[0].size()/2 - 1; jj++){
  int j = channels_in[0][jj]-1;
  cout << "jj: " << jj << "\tj: " << j << endl;
  for(int kk = channels_in[0].size()/2; kk <= channels_in[0].size()-1; kk++){
    int k = channels_in[0][kk]-1;
    cout << " kk: " << kk << "\t k: " << k << "\t fabs(kk-layersize+0.5-jj): " << fabs(kk-32+0.5-jj) <<  endl;
  }
  cout << endl;
}
*/


  int blcount[numberofboardused];
  for(int i=0;i<numberofboardused;i++){blcount[i]=0;}
  int num_entries = schain.GetEntries();

  cout << "Looking at bilayer coincidences... read in events - " << num_entries << endl;	

  for (Int_t i = 0; i < num_entries; i++){


    //if (i%10000000==0) cout << "schain entry = " << i << endl;

    schain.GetEntry(i);

    max_adc = 0;
    max_adc2 = 0;
    max_adc3 = 0;
    max_adc4 = 0;

    ///////////////////////////////////////////////////////////////////////////

    //jj/kk will be the strip geometry (0-11, 0-31, etc.), and j/k will be the pmt channels (0-63)
    if (smodule == pmtboardnumber[1]){  ///  only for pmt board number 3 apply gain corrections

      int pmt_num = 0;//number of vector row that corresponds to smodule 3

      int layersize = channels_in[pmt_num].size()/2;      

      adc_ratio_m = 999;

      for (int jj = 0; jj <= layersize-1; jj++){ //loop over top strips

        int j = channels_in[pmt_num][jj]-1; //input text file is 1-64, tree is 0-63

	//if(i==1)cout << "jj= " << jj << ", j = " << j << endl;
	//if(i==1)cout << "sfloats: " << sfloats[j] << ", baseline_channel: " << baseline_channel[int(smodule)][j] << endl;

	sfloats[j] -= baseline_channel[int(smodule)][j]; //ADC in top strip
	if(test_calib){ sfloats[j] *= gain_new[int(smodule)][j]/gainused[int(smodule)][j]; }

	if(sfloats[j] > cutnpe[int(smodule)][j]){

	  for(int kk=layersize; kk<=2*layersize-1;kk++){ //loop over bottom strips

            int k = channels_in[pmt_num][kk]-1;

	    //if(i == 1) cout << " kk= " << kk << ", k = " << k << endl;

	    sfloats[k] -= baseline_channel[int(smodule)][k];
	    if(test_calib){ sfloats[k] *= gain_new[int(smodule)][k]/gainused[int(smodule)][k]; }
	       		
	    if(sfloats[k] > cutnpe[int(smodule)][k]){
      	      //cout << "jj: "<<jj<<", j: " << j <<", kk: " << kk << ", k: " << k << ", overlap: " << fabs(kk-layersize+0.5-jj) << endl;  
	      //if(fabs(stripnum[j]-32+.5-stripnum[k])*5==2.5&&(sfloats[k]+sfloats[j]>max_adc)){ //pick out overlapping pair with maximum adc  
	      if(fabs(kk-layersize+0.5-jj) == 0.5 /*&& (sfloats[k]+sfloats[j]>max_adc)*/){  //jj-layersize-kk+0.5 should be +/-0.5 if the strips overlap
		//cout << "TopStrip: " << channels_in[pmt_num][jj]<<" BotStrip: " << channels_in[pmt_num][kk]<<endl;
		if( fabs(1-sfloats[j]/sfloats[k]) < fabs(1-adc_ratio_m)){ //Select the top/bottom ratio closest to 1
		  adc_ratio_m = sfloats[j]/sfloats[k];
		  //cout << "i: " << i << "\tjj: " << jj+1 << "\tkk: " << kk+1 << "\tadc_ratio_m: "<< adc_ratio_m << "\tsfloat(j/k): " << sfloats[j]/sfloats[k] << endl; 
		  stime_sec_high_m=stime_sec_high;
		  stime_sec_low_m=stime_sec_low;
		  stime_16ns_high_m=stime_16ns_high;
		  stime_16ns_low_m=stime_16ns_low;
		  smodule_m=smodule;
		  adc_bot_m = sfloats[k];
		  adc_top_m = sfloats[j];
		  max_adc = adc_bot_m + adc_top_m;
		  strip_bot_m = k;
		  strip_top_m = j;
		  index_bot_m = kk;
		  index_top_m = jj;
		}
	      }
	    }
	  }// end of loop over bottom strips
	}
      }//end of loop over top strips
     
      //if(2000<three_counter && three_counter < 2030 && max_adc>0){
      //  cout << "i: " << i << "\tsmodule: " << smodule_m << "\ttop: " << strip_top_m+1 << "\tbot: " << strip_bot_m+1 << "\tADC: " << max_adc << "\tt_sec: " << (uint32_t)(stime_sec_high_m*65536.+stime_sec_low_m) << "\tt_16ns: " << (uint32_t)(stime_16ns_high_m*65536.+stime_16ns_low_m) << endl;
      //}
      //if(max_adc>0) three_counter++;
 
      // create new ntuple with coincidence hits.
      if(max_adc && fabs(1-adc_ratio_m) <= ratio_cut ) {
	time_in_s = (int) (stime_sec_high_m*65536.+stime_sec_low_m);
	if(pmt_layer[1] == 1 && pmt_xy[1] == 'x'){
	  module_coinc->Fill(strip_bot_m,strip_top_m,adc_bot_m,adc_top_m,stime_sec_high_m ,stime_sec_low_m,stime_16ns_high_m,stime_16ns_low_m,smodule_m,adc_ratio_m);
	  maptimes.insert(make_pair(time_in_s,index));
	  index++;
	}
	else if(pmt_layer[1] == 2 && pmt_xy[1] == 'x'){
	  module_coinc3->Fill(strip_bot_m,strip_top_m,adc_bot_m,adc_top_m,stime_sec_high_m ,stime_sec_low_m,stime_16ns_high_m,stime_16ns_low_m,smodule_m,adc_ratio_m);
	  maptimes2.insert(make_pair(time_in_s,index2));
	  index2++;
	}
	else if(pmt_layer[1] == 1 && pmt_xy[1] == 'y'){
	  module_coinc2->Fill(strip_bot_m,strip_top_m,adc_bot_m,adc_top_m,stime_sec_high_m ,stime_sec_low_m,stime_16ns_high_m,stime_16ns_low_m,smodule_m,adc_ratio_m);
	}
	else if(pmt_layer[1] == 2 && pmt_xy[1] == 'y'){
	  module_coinc4->Fill(strip_bot_m,strip_top_m,adc_bot_m,adc_top_m,stime_sec_high_m ,stime_sec_low_m,stime_16ns_high_m,stime_16ns_low_m,smodule_m,adc_ratio_m);
	}
	//channel_events->Fill(index_bot_m,0);	
	//channel_events->Fill(index_top_m,0);	
	channel_events->Fill(strip_bot_m,0);	
	channel_events->Fill(strip_top_m,0);	
	ADC[pmtboardnumber[1]][(int)strip_top_m]->Fill(adc_top_m);
	ADC[pmtboardnumber[1]][(int)strip_bot_m]->Fill(adc_bot_m);
	//TESTING  
	//module_coinc->Fill(strip_bot_m,strip_top_m,adc_bot_m,adc_top_m,stime_sec_high_m ,stime_sec_low_m,stime_16ns_high_m,stime_16ns_low_m,smodule_m);
	//maptimes.insert(make_pair(time_in_s,index));
	//index++;
	blcount[0]++;	
      }
    }//end of module if statement for module 1 (3)


    if (smodule == pmtboardnumber[2]){  ///  only for pmt board number 7
      int pmt_num = 1;//number of vector row that corresponds to smodule 7
      int layersize = channels_in[pmt_num].size()/2;      
      adc_ratio_m2 = 999;
      for (int jj = 0; jj <= layersize-1; jj++){ //loop over top strips
	int j = channels_in[pmt_num][jj] - 1;
       	//if(i==1)cout << "jj= " << jj << ", j = " << j << endl;
	//if(i==1)cout << "sfloats: " << sfloats[j] << ", baseline_channel: " << baseline_channel[int(smodule)][j] << endl;
	sfloats[j] -= baseline_channel[int(smodule)][j];
	if(test_calib){ sfloats[j] *= gain_new[int(smodule)][j]/gainused[int(smodule)][j]; }

	if(sfloats[j]>cutnpe[int(smodule)][j]){

	  for(int kk=layersize; kk<=2*layersize-1;kk++){ //loop over bottom strips
            int k = channels_in[pmt_num][kk]-1;
	    sfloats[k] -= baseline_channel[int(smodule)][k];
	    if(test_calib){ sfloats[k] *= gain_new[int(smodule)][k]/gainused[int(smodule)][k]; }
	    if(sfloats[k]>cutnpe[int(smodule)][k]){
      	    //if(j==1)cout << "j: " << j << ", k: " << k << ", overlap: " << fabs(kk-layersize+0.5-jj) << endl;  
	    //if(j==1) cout << "j: " << j << ", k: " << k << ", stripnum overlap: " << fabs(stripnum[j]-32+.5-stripnum[k])*5 << endl << endl; 
	      //if(fabs(stripnum[j]-32+.5-stripnum[k])*5==2.5&&(sfloats[k]+sfloats[j]>max_adc)){ //pick out overlapping pair with maximum adc  
	      if(fabs(kk-layersize-0.5-jj) == 0.5 /*&& (sfloats[k]+sfloats[j]>max_adc2)*/){  //jj-layersize-kk+0.5 should be +/-0.5 if the strips overlap
		if( fabs(1-sfloats[j]/sfloats[k]) < fabs(1-adc_ratio_m2)){ //Select the top/bottom ratio closest to 1
		  adc_ratio_m2 = sfloats[j]/sfloats[k]; 
		  stime_sec_high_m2=stime_sec_high;
		  stime_sec_low_m2=stime_sec_low;
		  stime_16ns_high_m2=stime_16ns_high;
		  stime_16ns_low_m2=stime_16ns_low;
		  smodule_m2=smodule;
		  adc_bot_m2 = sfloats[k];
		  adc_top_m2 = sfloats[j];
		  max_adc2=adc_bot_m2+adc_top_m2;
		  strip_bot_m2 = k;
		  strip_top_m2 = j;
		  index_bot_m2 = kk;
		  index_top_m2 = jj;
		}
	      }
	    }
	  }// end of loop over bottom strips
	}
      }//end of loop over top strips
      //if(2000<seven_counter && seven_counter<2030 && max_adc2>0){
      //  cout << "i: " << i <<"\tsmodule: " << smodule_m2 << "\ttop: " << strip_top_m2+1 << "\tbot: " << strip_bot_m2+1 << "\tADC: " << max_adc2 << "\tt_sec: " << (uint32_t)(stime_sec_high_m2*65536.+stime_sec_low_m2) << "\tt_16ns: " << (uint32_t)(stime_16ns_high_m2*65536.+stime_16ns_low_m2) << endl;
      //}
      //if(max_adc2>0) seven_counter++;

      // create new ntuple with coincidence hits.
      if(max_adc2 && fabs(1-adc_ratio_m2) <= ratio_cut ) {
	time_in_s = (int) (stime_sec_high_m*65536.+stime_sec_low_m);
	if(pmt_layer[2] == 1 && pmt_xy[2] == 'x'){
	  module_coinc->Fill(strip_bot_m2,strip_top_m2,adc_bot_m2,adc_top_m2,stime_sec_high_m2 ,stime_sec_low_m2,stime_16ns_high_m2,stime_16ns_low_m2,smodule_m2,adc_ratio_m2);
	  maptimes.insert(make_pair(time_in_s,index));
	  index++;
	}
	else if(pmt_layer[2] == 2 && pmt_xy[2] == 'x'){
	  module_coinc3->Fill(strip_bot_m2,strip_top_m2,adc_bot_m2,adc_top_m2,stime_sec_high_m2 ,stime_sec_low_m2,stime_16ns_high_m2,stime_16ns_low_m2,smodule_m2,adc_ratio_m2);
	  maptimes2.insert(make_pair(time_in_s,index2));
	  index2++;
	}
	else if(pmt_layer[2] == 1 && pmt_xy[2] == 'y'){
	  module_coinc2->Fill(strip_bot_m2,strip_top_m2,adc_bot_m2,adc_top_m2,stime_sec_high_m2,stime_sec_low_m2,stime_16ns_high_m2,stime_16ns_low_m2,smodule_m2,adc_ratio_m2);
	}
	else if(pmt_layer[2] == 2 && pmt_xy[2] == 'y'){
	  module_coinc4->Fill(strip_bot_m2,strip_top_m2,adc_bot_m2,adc_top_m2,stime_sec_high_m2,stime_sec_low_m2,stime_16ns_high_m2,stime_16ns_low_m2,smodule_m2,adc_ratio_m2);
	}
	//channel_events->Fill(index_bot_m2,1);	
	//channel_events->Fill(index_top_m2,1);	
	channel_events->Fill(strip_bot_m2,1);	
	channel_events->Fill(strip_top_m2,1);	
	ADC[pmtboardnumber[2]][(int)strip_top_m]->Fill(adc_top_m);
	ADC[pmtboardnumber[2]][(int)strip_bot_m]->Fill(adc_bot_m);

	//TESTING
	//module_coinc2->Fill(strip_bot_m2,strip_top_m2,adc_bot_m2,adc_top_m2,stime_sec_high_m2,stime_sec_low_m2,stime_16ns_high_m2,stime_16ns_low_m2,smodule_m2);
	blcount[1]++;	
      }
    }//end of module if statement (7)

//Module 1 which is the third one
    if (smodule == pmtboardnumber[3]){  ///  only for pmt board number 1 apply gain corrections

      int pmt_num = 2;//number of vector row that corresponds to smodule 3
      int layersize = channels_in[pmt_num].size()/2;      
      adc_ratio_m3 = 999;
      for (int jj = 0; jj <= layersize-1; jj++){ //loop over top strips
        int j = channels_in[pmt_num][jj]-1; //input text file is 1-64, tree is 0-63
	//if(i==1)cout << "jj= " << jj << ", j = " << j << endl;
	//if(i==1)cout << "sfloats: " << sfloats[j] << ", baseline_channel: " << baseline_channel[int(smodule)][j] << endl;
//	if(j == 7 && sfloats[j] != 0){cout << "i: " << i << "\tsfloats[7]: " << sfloats[j] << endl;}

	sfloats[j] -= baseline_channel[int(smodule)][j]; //ADC in top strip
	if(test_calib){ sfloats[j] *= gain_new[int(smodule)][j]/gainused[int(smodule)][j]; }
	if(sfloats[j] > cutnpe[int(smodule)][j]){

	  for(int kk=layersize; kk<=2*layersize-1;kk++){ //loop over bottom strips

            int k = channels_in[pmt_num][kk]-1;
	    //if(i == 1) cout << " kk= " << kk << ", k = " << k << endl;
	    sfloats[k] -= baseline_channel[int(smodule)][k];
	    if(test_calib){ sfloats[k] *= gain_new[int(smodule)][k]/gainused[int(smodule)][k]; }
	    if(sfloats[k] > cutnpe[int(smodule)][k]){

	      //if(fabs(stripnum[j]-32+.5-stripnum[k])*5==2.5&&(sfloats[k]+sfloats[j]>max_adc)){ //pick out overlapping pair with maximum adc  
	      if(fabs(kk-layersize+0.5-jj) == 0.5 /*&& (sfloats[k]+sfloats[j]>max_adc3)*/){  //jj-layersize-kk+0.5 should be +/-0.5 if the strips overlap
	        //cout << "TopStrip = " << channels_in[pmt_num][jj]<<" BotStrip: " << channels_in[pmt_num][kk]<<", " << fabs(kk-layersize+0.5-jj) <<endl;
  		if( fabs(1-sfloats[j]/sfloats[k]) < fabs(1-adc_ratio_m3)){ //Select the top/bottom ratio closest to 1
		  adc_ratio_m3 = sfloats[j]/sfloats[k]; 
	          stime_sec_high_m3=stime_sec_high;
		  stime_sec_low_m3=stime_sec_low;
		  stime_16ns_high_m3=stime_16ns_high;
		  stime_16ns_low_m3=stime_16ns_low;
		  smodule_m3=smodule;
		  adc_bot_m3= sfloats[k];
		  adc_top_m3= sfloats[j];
		  max_adc3 = adc_bot_m3 + adc_top_m3;
		  strip_bot_m3 = k;
	 	  strip_top_m3= j;
	          index_bot_m3 = kk;
		  index_top_m3 = jj;
		}
	      }
	    }
	  }// end of loop over bottom strips
	}
      }//end of loop over top strips
      
      // create new ntuple with coincidence hits.
      if(max_adc3 && fabs(1-adc_ratio_m3) <= ratio_cut ) {
	time_in_s = (int) (stime_sec_high_m*65536.+stime_sec_low_m);
	if(pmt_layer[3] == 1 && pmt_xy[3] == 'x'){
	  module_coinc->Fill(strip_bot_m3,strip_top_m3,adc_bot_m3,adc_top_m3,stime_sec_high_m3,stime_sec_low_m3,stime_16ns_high_m3,stime_16ns_low_m3,smodule_m3,adc_ratio_m3);
	  maptimes.insert(make_pair(time_in_s,index));
	  index++;
	}
	else if(pmt_layer[3] == 2 && pmt_xy[3] == 'x'){
	  module_coinc3->Fill(strip_bot_m3,strip_top_m3,adc_bot_m3,adc_top_m3,stime_sec_high_m3,stime_sec_low_m3,stime_16ns_high_m3,stime_16ns_low_m3,smodule_m3,adc_ratio_m3);
	  maptimes2.insert(make_pair(time_in_s,index2));
	  index2++;
	}
	else if(pmt_layer[3] == 1 && pmt_xy[3] == 'y'){
	  module_coinc2->Fill(strip_bot_m3,strip_top_m3,adc_bot_m3,adc_top_m3,stime_sec_high_m3,stime_sec_low_m3,stime_16ns_high_m3,stime_16ns_low_m3,smodule_m3,adc_ratio_m3);
	}
	else if(pmt_layer[3] == 2 && pmt_xy[3] == 'y'){
	  module_coinc4->Fill(strip_bot_m3,strip_top_m3,adc_bot_m3,adc_top_m3,stime_sec_high_m3,stime_sec_low_m3,stime_16ns_high_m3,stime_16ns_low_m3,smodule_m3,adc_ratio_m3);
	}
	//channel_events->Fill(index_bot_m3,2);	
	//channel_events->Fill(index_top_m3,2);	
	channel_events->Fill(strip_bot_m3,2);	
	channel_events->Fill(strip_top_m3,2);	
	ADC[pmtboardnumber[3]][(int)strip_top_m]->Fill(adc_top_m);
	ADC[pmtboardnumber[3]][(int)strip_bot_m]->Fill(adc_bot_m);
	blcount[2]++;
	//TESTING
	//module_coinc3->Fill(strip_bot_m3,strip_top_m3,adc_bot_m3,adc_top_m3,stime_sec_high_m3,stime_sec_low_m3,stime_16ns_high_m3,stime_16ns_low_m3,smodule_m3);
	//maptimes2.insert(make_pair(time_in_s,index2));
	//index2++;
      }
    }//end of module if statement for module (1)

    //Start Module number 2 which is the fourth one
    if (smodule == pmtboardnumber[4]){  ///  only for pmt board number 2
      int pmt_num = 3;//number of vector row that corresponds to smodule 2
      int layersize = channels_in[pmt_num].size()/2;      
      adc_ratio_m4 = 999;
      for (int jj = 0; jj <= layersize-1; jj++){ //loop over top strips
        int j = channels_in[pmt_num][jj] - 1;
	sfloats[j] -= baseline_channel[int(smodule)][j];
	if(test_calib){ sfloats[j] *= gain_new[int(smodule)][j]/gainused[int(smodule)][j]; }
	if(sfloats[j]>cutnpe[int(smodule)][j]){

	  for(int kk=layersize; kk<=2*layersize-1;kk++){ //loop over bottom strips
            int k = channels_in[pmt_num][kk]-1;
	    sfloats[k] -= baseline_channel[int(smodule)][k];
	    if(test_calib){ sfloats[k] *= gain_new[int(smodule)][k]/gainused[int(smodule)][k]; }
	    if(sfloats[k]>cutnpe[int(smodule)][k]){
      	      cout << "jj: "<<jj<<", j: " << j <<", kk: " << kk << ", k: " << k << ", overlap: " << fabs(kk-layersize+0.5-jj) << endl;    
	      //if(fabs(stripnum[j]-32+.5-stripnum[k])*5==2.5&&(sfloats[k]+sfloats[j]>max_adc)){ //pick out overlapping pair with maximum adc  
	      if(fabs(kk-layersize-0.5-jj) == 0.5 /*&& (sfloats[k]+sfloats[j]>max_adc4)*/){  //jj-layersize-kk+0.5 should be +/-0.5 if the strips overlap
		if( fabs(1-sfloats[j]/sfloats[k]) < fabs(1-adc_ratio_m4)){ //Select the top/bottom ratio closest to 1
		  adc_ratio_m4 = sfloats[j]/sfloats[k]; 
		  stime_sec_high_m4=stime_sec_high;
		  stime_sec_low_m4=stime_sec_low;
		  stime_16ns_high_m4=stime_16ns_high;
		  stime_16ns_low_m4=stime_16ns_low;
		  smodule_m4=smodule;
		  adc_bot_m4 = sfloats[k];
		  adc_top_m4 = sfloats[j];
		  max_adc4=adc_bot_m4+adc_top_m4;
		  strip_bot_m4 = k;
		  strip_top_m4 = j;
	          index_bot_m4 = kk;
		  index_top_m4 = jj;
		}
	      }
	    }
	  }// end of loop over bottom strips
	}
      }//end of loop over top strips

      // create new ntuple with coincidence hits.
      if(max_adc4 && fabs(1-adc_ratio_m4) <= ratio_cut ) {
	time_in_s = (int) (stime_sec_high_m*65536.+stime_sec_low_m);
	if(pmt_layer[4] == 1 && pmt_xy[4] == 'x'){
	  module_coinc->Fill(strip_bot_m4,strip_top_m4,adc_bot_m4,adc_top_m4,stime_sec_high_m4,stime_sec_low_m4,stime_16ns_high_m4,stime_16ns_low_m4,smodule_m4,adc_ratio_m4);
	  maptimes.insert(make_pair(time_in_s,index));
	  index++;
	}
	else if(pmt_layer[4] == 2 && pmt_xy[4] == 'x'){
	  module_coinc3->Fill(strip_bot_m4,strip_top_m4,adc_bot_m4,adc_top_m4,stime_sec_high_m4,stime_sec_low_m4,stime_16ns_high_m4,stime_16ns_low_m4,smodule_m4,adc_ratio_m4);
	  maptimes2.insert(make_pair(time_in_s,index2));
	  index2++;
	}
	else if(pmt_layer[4] == 1 && pmt_xy[4] == 'y'){
	  module_coinc2->Fill(strip_bot_m4,strip_top_m4,adc_bot_m4,adc_top_m4,stime_sec_high_m4,stime_sec_low_m4,stime_16ns_high_m4,stime_16ns_low_m4,smodule_m4,adc_ratio_m4);
	}
	else if(pmt_layer[4] == 2 && pmt_xy[4] == 'y'){
	  module_coinc4->Fill(strip_bot_m4,strip_top_m4,adc_bot_m4,adc_top_m4,stime_sec_high_m4,stime_sec_low_m4,stime_16ns_high_m4,stime_16ns_low_m4,smodule_m4,adc_ratio_m4);
	}
	//channel_events->Fill(index_bot_m4,3);	
	//channel_events->Fill(index_top_m4,3);	
	channel_events->Fill(strip_bot_m4,3);	
	channel_events->Fill(strip_top_m4,3);	
	ADC[pmtboardnumber[4]][(int)strip_top_m]->Fill(adc_top_m);
	ADC[pmtboardnumber[4]][(int)strip_bot_m]->Fill(adc_bot_m);
	blcount[3]++;
	//TESTING
	//module_coinc4->Fill(strip_bot_m4,strip_top_m4,adc_bot_m4,adc_top_m4,stime_sec_high_m4,stime_sec_low_m4,stime_16ns_high_m4,stime_16ns_low_m4,smodule_m4);
	
      }
    }//end of module if statement (2)
  } //close for loop over schain



for(int i=0; i<numberofboardused; i++){
  cout << "Events in board " << pmtboardnumber[i] << ": " << blcount[i] << endl;
}
//  cout << "size of module (" <<pmtboardnumber[1]<< ") ntuple  = " << module_coinc->GetEntries() << endl;       // 19.000
//  cout << "size of module2 (" <<pmtboardnumber[2]<< ") ntuple  = " << module_coinc2->GetEntries() << endl;     //200.000
//  cout << "size of module3 (" <<pmtboardnumber[3]<< ") ntuple  = " << module_coinc3->GetEntries() << endl;       // 19.000
//  cout << "size of module4 (" <<pmtboardnumber[4]<< ") ntuple  = " << module_coinc4->GetEntries() << endl;     //200.000

////////////////////////////////////////////////////////
	

  //*** Four fold start ***
  //now get timing offsets from muon counters and module
  //start fourfold layer 1
/*  cout << "looping over module 1 ntuple..."<<endl;

  for (int j=0; j<module_coinc2->GetEntries();j++){

    module_coinc2->GetEntry(j);
    time_in_s = (int)(stime_sec_high_m2*65536.+stime_sec_low_m2);

    //if(j==2000){cout << "module 2: time: " << time_in_s << "\t16ns_high: " << (int)(stime_16ns_high_m2) << "\t16ns_low: " << (int)stime_16ns_low_m2 << endl;}

    typedef mymap_t::const_iterator I;
    for (int time=-1;time<=1;time++){
      std::pair<I,I> b = maptimes.equal_range(time_in_s+time);
      
      for (I i=b.first; i!=b.second; ++i){          //loop through all entries which match in seconds (+/-1)
		module_coinc->GetEntry((*i).second);
		
		//In general any pair of boards should have a constant offset.
		//We expect the difference in the high bits of the 16ns time to be within 10 (i.e. ~0.01 s)
		if(fabs(stime_16ns_high_m2-stime_16ns_high_m)<=10){  
		timediff = stime_16ns_low_m2-stime_16ns_low_m;
		//timediff = (stime_16ns_high_m2*65536 + stime_16ns_low_m2) - (stime_16ns_high_m*65536 + stime_16ns_low_m);

		//The time offset was calculted with only the low bits (0-65536 clock cycles)
		//The offset used will depend on which pair of PMTs are being compared
		timediff -= (65536-offsets[0]);
		
		while( timediff < 0 ) timediff += 65536;   
		//while( timediff > 65536) timediff -= 65536;

		//if(j==2000){cout << "  module 1: time: " << (uint32_t)(stime_sec_high_m*65536.+stime_sec_low_m) << "\t16ns_high: " << (int)(stime_16ns_high_m) << "\t16ns_low: " << stime_16ns_low_m << endl; }
	 	//if(j==2000)cout << "timediff = " << timediff << endl;

		  //We ask that the time difference be less than 4 clock cycles after accounting for the offset
		  if(fabs(timediff)<=TDIFF_CUT){
		    //if ( fabs(time_offset) > timediff ){
		      time_offset = timediff; 
		      bot_adc = adc_bot_m; //module 7
		      top_adc = adc_top_m; //module 7

		      bot_adc2 = adc_bot_m2; //module 9
		      top_adc2 = adc_top_m2; //module 9

		      topstrip = (int)strip_top_m;
		      botstrip = (int)strip_bot_m;

		      topstrip2 = (int)strip_top_m2;
		      botstrip2 = (int)strip_bot_m2;
		      //if(j==2000)cout << "4-fold found\n";
		    //}  
		  }
		}
      }
    }
    if (time_offset !=1e10){
      //If we have a counter hits in coincidence with module hits, fill histograms			
      //histogram of hits per counter
      timecoincidence_counter++;

      ADC[3][topstrip]->Fill(top_adc);
      ADC[3][botstrip]->Fill(bot_adc);

      ADC[7][topstrip2]->Fill(top_adc2);
      ADC[7][botstrip2]->Fill(bot_adc2);
      
    } //if on time offset
    time_offset =1e10;
  }  //end loop over j from 0 to counter->GEtEntries()

  cout << "Event in coincidence between 3/7 = " << timecoincidence_counter << endl;


//Start fourfold layer 2
cout << "looping over module 3 ntuple..."<<endl;

  for (int j=0; j<module_coinc4->GetEntries();j++){

    module_coinc4->GetEntry(j);
    time_in_s = (int)(stime_sec_high_m4*65536.+stime_sec_low_m4);

    typedef mymap_t::const_iterator I;
    for (int time=-1;time<=1;time++){
      std::pair<I,I> b = maptimes2.equal_range(time_in_s+time);
      
      for (I i=b.first; i!=b.second; ++i){          //loop through all entries which match in seconds (+/-1)
		module_coinc3->GetEntry((*i).second);
	   //stime_16ns_low_m -= offsets[0]; //subtract the offset from the top module
		//if ((stime_16ns_high_m4*65536.+stime_16ns_low_m4)<(stime_16ns_high_m3*65536.+stime_16ns_low_m3)){  
		if(fabs(stime_16ns_high_m4-stime_16ns_high_m3)<=10){ 
//		timediff = ((stime_16ns_high_m4*65536.+stime_16ns_low_m4-stime_16ns_high_m3*65536.-stime_16ns_low_m3));
		timediff = stime_16ns_low_m3 - stime_16ns_low_m4;
		timediff -= offsets[5];
		while( timediff < 0 ) timediff += 65536;	


	 	  //cout << "timediff = " << timediff << endl;

		  if(fabs(timediff)<=TDIFF_CUT){
			//if ( fabs(time_offset) > timediff ){
			  time_offset = timediff; 
			  bot_adc3 = adc_bot_m3; 
			  top_adc3 = adc_top_m3; 

			  bot_adc4 = adc_bot_m4; 
			  top_adc4 = adc_top_m4; 

			  topstrip3 = (int)strip_top_m3;
			  botstrip3 = (int)strip_bot_m3;

			  topstrip4 = (int)strip_top_m4;
			  botstrip4 = (int)strip_bot_m4;

			//}  
		  }
		}
		
      } //loop over b.first to b.second
    } //loop over time    
    if (time_offset !=1e10){
      //If we have a counter hits in coincidence with module hits, fill histograms			
      //histogram of hits per counter
      timecoincidence_counter2++;

      ADC[1][topstrip3]->Fill(top_adc3);
      ADC[1][botstrip3]->Fill(bot_adc3);

      ADC[2][topstrip4]->Fill(top_adc4);
      ADC[2][botstrip4]->Fill(bot_adc4);
      
    } //if on time offset



    time_offset =1e10;




//	if ((stime_16ns_high_m2*65536.+stime_16ns_low_m2)>(stime_16ns_high_m*65536.+stime_16ns_low_m)){  
//	  if(timediff<=20000){
//	    if (fabs(time_offset)>timediff){
//	      time_offset = timediff;
//	      bot_adc = adc_bot_m;  //module 7
//	      top_adc = adc_top_m;  //module 7
//
//	      bot_adc2 = adc_bot_m2; //module 9
//	      top_adc2 = adc_top_m2; //module 9
//
//	      topstrip = (int)strip_top_m;
//	      botstrip = (int)strip_bot_m;
//
//	      topstrip2 = (int)strip_top_m2;
//	      botstrip2 = (int)strip_bot_m2;
//
//	    }  
//	  }
//	}



  }  //end loop over j from 0 to counter->GEtEntries()
  cout << "Event in coincidence between 1/2 = " << timecoincidence_counter2 << endl;
*/
  gStyle->SetCanvasBorderMode( 0 );
  gStyle->SetCanvasColor( 0 );
  gStyle->SetPadColor( 0 );
  gStyle->SetPadBorderMode( 0 );
  gStyle->SetFrameBorderMode( 0 );
  gStyle->SetTitleColor( 0 );
  gStyle->SetTitleFillColor( 0 );
  gStyle->SetTitleBorderSize( 0 );
  gStyle->SetTitleX( 0.1 );
  gStyle->SetTitleY( 0.98 );
  gStyle->SetTitleFont( 22, "" );
  gStyle->SetTitleSize( 0.055, ""  );
  gStyle->SetStatColor( 0 );
  gStyle->SetStatFont( 22 );
  gStyle->SetStatBorderSize( 1 );
  gStyle->SetStatX( 0.90 );
  gStyle->SetStatY( 0.90 );
  gStyle->SetStatFontSize( 0.04 );
  gStyle->SetOptStat( 1110 );
  gStyle->SetTitleFont( 22, "XYZ"  );
  gStyle->SetTitleSize( 0.05, "XYZ"  );
  gStyle->SetTitleColor( kBlack, "XYZ"  );
  gStyle->SetTitleAlign(13);
  gStyle->SetLabelFont( 22, "XYZ"  );
  gStyle->SetLabelSize( 0.04, "XYZ"  );
  gStyle->SetOptStat( 0 );
  gStyle->SetOptFit( 0 );
  gStyle->SetPalette( 1 );

  const Int_t NRGBs = 5;
  const Int_t NCont = 255;

  Double_t stops[NRGBs] = { 0.00, 0.34, 0.61, 0.84, 1.00 };
  Double_t red[NRGBs]   = { 0.00, 0.00, 0.87, 1.00, 0.51 };
  Double_t green[NRGBs] = { 0.00, 0.81, 1.00, 0.20, 0.00 };
  Double_t blue[NRGBs]  = { 0.51, 1.00, 0.12, 0.00, 0.00 };

  TColor::CreateGradientColorTable(NRGBs, stops, red, green, blue, NCont);
  gStyle->SetNumberContours(NCont);
  gROOT->ForceStyle();
  channel_events->Draw("colz");
  channel_events->GetXaxis()->SetTitle("Channel index (compare to fcl file)");
  channel_events->GetYaxis()->SetTitle("PMT index (1-4)");
  c1->Print(Form("plots/channel_events_%i.png",subrunnumber));
  ofstream gain_file;
  if(!test_calib){gain_file.open("gain_file.txt");}

  for (int j=1; j<=numberofboardused; j++){ //to be replaced after query on mysql
    for (int i = 0; i < 64; i++){
      ADCv[pmtboardnumber[j]][i]=ADC[pmtboardnumber[j]][i]->GetMean();
      if(ADCv[pmtboardnumber[j]][i]>0){
        ADCvmean[pmtboardnumber[j]]->Fill(ADCv[pmtboardnumber[j]][i]);
	allADCv->Fill(ADCv[pmtboardnumber[j]][i]);
	ADC[pmtboardnumber[j]][i]->Draw();
        if(!test_calib){c1->Print(Form("adc_histos/adc_%d_%d.png",pmtboardnumber[j],i));}
        if(test_calib){c1->Print(Form("adc_histos_calib/adc_%d_%d.png",pmtboardnumber[j],i));}
	
        channel_adc->Fill(i,j-1,ADCv[pmtboardnumber[j]][i]);
      }
    }
    ADCvmean[pmtboardnumber[j]]->Draw();
    if(!test_calib){c1->Print(Form("adcvmean_%d.png",pmtboardnumber[j]));}
    if(test_calib){c1->Print(Form("adcvmean_calib_%d.png",pmtboardnumber[j]));}
  }
  channel_adc->Draw("colz");
  c1->Print(Form("plots/channel_adc_%i.png",subrunnumber));
  allADCv->Draw();
  if(!test_calib){c1->Print("alladcv.png");}
  if(test_calib){c1->Print("alladcv_calib.png");}

  for (int j=1; j<=4; j++){ 
    cout << ADCvmean[pmtboardnumber[j]]->GetMean() << endl;
    for (int i = 0; i < 64; i++){
      if(ADCv[pmtboardnumber[j]][i] > 0) {
        if(!test_calib){
          //gain_v[pmtboardnumber[j]][i] =  int(ADCvmean[pmtboardnumber[j]]->GetMean()/ADCv[pmtboardnumber[j]][i]*16);
	  gain_v[pmtboardnumber[j]][i] =  int(ADC_calib_target/ADCv[pmtboardnumber[j]][i]*gainused[pmtboardnumber[j]][i]);
	  if(gain_v[pmtboardnumber[j]][i] < 10){gain_v[pmtboardnumber[j]][i] = 10; }
	  if(gain_v[pmtboardnumber[j]][i] > 27){gain_v[pmtboardnumber[j]][i] = 27; }
	}
        else{
          gain_v[pmtboardnumber[j]][i] = gain_new[pmtboardnumber[j]][i];
        }
      }
      else {
        gain_v[pmtboardnumber[j]][i] = 0;
      } 
      //gain_file << Form("%d\t%d\t%d\n",pmtboardnumber[j],i,gain_v[pmtboardnumber[j]][i]);
      if(!test_calib){gain_file << pmtboardnumber[j] << " " << i << " " << gain_v[pmtboardnumber[j]][i] << endl;}
      if(gain_v[pmtboardnumber[j]][i] != 0){
	//printf("board %d, ch %d , gain %d\n",int(pmtboardnumber[j]),i,gain_v[pmtboardnumber[j]][i]);
      }
      gain_p[pmtboardnumber[j]]->Fill(gain_v[pmtboardnumber[j]][i]);
      allgain_p->Fill(gain_v[pmtboardnumber[j]][i]);
    }
    gain_p[pmtboardnumber[j]]->Draw();
    if(!test_calib){c1->Print(Form("gain_p_%d.png",pmtboardnumber[j]));}
    if(test_calib){c1->Print(Form("gain_p_calib_%d.png",pmtboardnumber[j]));}
  }
  allgain_p->Draw();
  if(!test_calib){c1->Print("allgain_p.png");}
  if(test_calib){c1->Print("allgain_p_calib.png");}
  
  time_t end_t;
  time( &end_t);

  cout << "Total runtime: " << difftime(end_t,start_t) << " seconds\n";

  exit(0);
  
}
 
