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
//#include <TSQLServer.h>
//#include <TSQLResult.h>
//#include <TSQLRow.h>
#include <ctype.h>   
#include <iostream>
#include <string>
#include "fstream"
#include "sstream"
#include <vector>
#include <sstream>
#include <dirent.h>
//using System;        // for Char


void ch2strip(int s2c[64], bool debug=false)
{        
/*
This function provide a mapping between Maroc2 channel number and strip number
*/

  Int_t i,j;
  for(Int_t m =0; m<64; m++) {
    i=(Int_t)(m/8);
    j=m-8*(Int_t)(m/8);
    s2c[m]=8*(7-i)+2*(7-j)-7*(Int_t)((7-j)/4)+1;
    //if(debug) cout << "s2c[" << m << "]: " << s2c[m] << endl;
    if(debug) cout << 65-s2c[m] << " ";
  }
  if(debug) cout << endl;
}

void strip2ch(int c2s[65], bool debug=false)
{        
/*
This function provide a mapping between Maroc2 channel number and strip number
*/

  Int_t i,j;
  int s2c[64];
  for(Int_t m =0; m<64; m++) {
    i=(Int_t)(m/8);
    j=m-8*(Int_t)(m/8);
    s2c[m]=8*(7-i)+2*(7-j)-7*(Int_t)((7-j)/4)+1;
	c2s[s2c[m]] = 64-m;
    if(debug) cout << "c2s[" << m << "]: " << c2s[m] << endl;
  }
}

void counterpos(float z[64])
{        
/*
This function provide position mapping of coincidence counter channels in cm
*/

for (int k=0;k<32;k++){
    z[k]=-1;
  }
  
  
  z[0]=9;
  z[1]=9;
  z[2]=45;
  z[3]=45;
  z[4]=92;
  z[12]=92;
  z[5]=149;
  z[13]=149;
  z[7]=207;
  z[6]=207;
  z[15]=262;
  z[14]=262;
  z[23]=310;
  z[22]=310;
  z[31]=353;
  z[30]=353;

}

void paddle(float counter_num[64], float channel_num[16])
{        
  //determining counter number from channel
  counter_num[0]=0;
  counter_num[1]=1;
  counter_num[2]=2;
  counter_num[3]=3;
  counter_num[4]=4;
  counter_num[12]=5;
  counter_num[5]=6;
  counter_num[13]=7;
  counter_num[7]=8;
  counter_num[6]=9;
  counter_num[15]=10;
  counter_num[14]=11;
  counter_num[23]=12;
  counter_num[22]=13;
  counter_num[31]=14;
  counter_num[30]=15;

  //determining channel number from counter number
  
  channel_num[0]=0;
  channel_num[1]=1;
  channel_num[2]=2;
  channel_num[3]=3;
  channel_num[4]=4;
  channel_num[5]=12;
  channel_num[6]=5;
  channel_num[7]=13;
  channel_num[8]=7;
  channel_num[9]=6;
  channel_num[10]=15;
  channel_num[11]=14;
  channel_num[12]=23;
  channel_num[13]=22;
  channel_num[14]=31;
  channel_num[15]=30;

}

void lenghtpigtail(float fiberlength[32])
{
//length of extra fiber pigtails

  fiberlength[0]= 105;
  fiberlength[1]= 101;
  fiberlength[2]= 93;
  fiberlength[3]= 89;
  fiberlength[4]= 84;
  fiberlength[5]= 80;
  fiberlength[6]= 74;
  fiberlength[7]= 69;
  fiberlength[8]= 64;
  fiberlength[9]= 59;
  fiberlength[10]= 55;
  fiberlength[11]= 51;
  fiberlength[12]= 48;
  fiberlength[13]= 45;
  fiberlength[14]= 44;
  fiberlength[15]= 43;
  fiberlength[16]= 44;
  fiberlength[17]= 47;
  fiberlength[18]= 49;
  fiberlength[19]= 52;
  fiberlength[20]= 56;
  fiberlength[21]= 60;
  fiberlength[22]= 65;
  fiberlength[23]= 67;
  fiberlength[24]= 73;
  fiberlength[25]= 79;
  fiberlength[26]= 83;
  fiberlength[27]= 86;
  fiberlength[28]= 93;
  fiberlength[29]= 100;
  fiberlength[30]= 105;
  fiberlength[31]= 108;
}

void usb2pmt(int usbnum, int smodnum, int pmtboardnumber[100]){
  if(smodnum != 1 && smodnum != 2){
    cout << "There are two super-modules per USB.\n";
    cout << "Use '1' or '2' to indicate which of them you want to calibrate.\n";
    return;
  }

  if(usbnum == 35){ //Robeson setup
    if(smodnum == 1){
      pmtboardnumber[1] = 3;
      pmtboardnumber[2] = 7;
      pmtboardnumber[3] = 1;
      pmtboardnumber[4] = 2;
      return;
    }
    else if(smodnum == 2){
      cout << "USB-" << usbnum << " does not have a second super-module.\n";
      return;
    }
  }


//Use this template to add the pmt configuration of each usb
/*if(usbnum == ){
    if(smodnum == 1){
      pmtboardnumber[1] = ;
      pmtboardnumber[2] = ;
      pmtboardnumber[3] = ;
      pmtboardnumber[4] = ;
      return;
    }
    else if(smodnum == 2){
      pmtboardnumber[1] = ;
      pmtboardnumber[2] = ;
      pmtboardnumber[3] = ;
      pmtboardnumber[4] = ;
      return;      
    }
  }*/
}



float trunc_mean(TH1F* hist)		//code to do the truncated mean
{
  float truncmeanlastm=0;
  float truncmeanm = hist->GetMean();
  while(fabs(truncmeanm-truncmeanlastm)>.1){ 
    truncmeanlastm=truncmeanm;
    hist->GetXaxis()->SetRangeUser(truncmeanm*.2,truncmeanm*2);
    truncmeanm = hist->GetMean();
  }
  return truncmeanm;
}


/*void getruninfo(char lab[10], const char* date, int subrunnumber, float *ADC2PE, int *DACt, int *PMT_board_serial, int *HV, bool debug = true)
{
        

//This function retrieves the ADC2PE and the DAC threshold used in the run
//- Input: labname (folder name), date (yyyymmdd), subrunnumber - 
//- Output currently: ADC2PE and DAC threhsold
//- it is also available the cut in terms of PE - 
//- Function will generate warning if somenthing will go wrong -

	
	char sql[1000];
	char sql0[1000];
	const char* servername = "mysql://riverside.nevis.columbia.edu:3306/doublechooz_ov_far"; //specify the mysql address
	const char* username   = "dcdaq";														 // username
	const char* userpass   = "doublechooz";													 // password
	const char* temp;
        const char* temp2;

	int nrows;
	int nfields;
	
	double cutinpe;
	double p0,p1;
    //char s[1];
    
    for (int loop = 0; lab[loop] !=0; loop++)
        lab[loop] = tolower(lab[loop]);

   // open connection to MySQL server on localhost
   
   TSQLServer *db = TSQLServer::Connect(servername,username,userpass);
   
   TSQLResult *res;
   TSQLRow *row;

   if (debug) printf("MYSQL info: version=%s\n", db->ServerInfo());

   
   //first query over runsummary_table table
		sprintf(sql0, "SELECT HV, DAC_threshold, PMT_Serial from runsummary_%s WHERE Runnumber = 'Run_%s_%d' AND PMT_Serial !='none'",lab,date,subrunnumber);

        res = db->Query(sql0);

		nrows = res->GetRowCount();
        if (nrows > 1)printf("\nWarning Got %d rows in result\n", nrows);

		nfields = res->GetFieldCount();
        if (nfields !=3) printf("\nWarning Got %d fields in result\n", nfields);
		
		row = res->Next();

		*HV = atoi(row->GetField(0));
		*DACt = atoi(row->GetField(1));

		temp = row->GetField(2);

        string mystring = string(temp);
		string temp1 = mystring.substr(2,5);

        temp2 = temp1.c_str(); 

		*PMT_board_serial = atoi(temp2);
		
		if (debug) printf("PMT Serial number=%d--HV used=%d--DAC_threshold=%d\n", *PMT_board_serial, *HV, *DACt);

   //second query over pmt_characterization_summary table

   sprintf(sql, "SELECT ADC2PE,p0,p1 from pmt_characterization_summary WHERE PMT_Serial = 'PA%d' AND HV = '%d'",*PMT_board_serial,*HV);


   res = db->Query(sql);

   nrows = res->GetRowCount();
   if (nrows > 1)printf("\nWarning Got %d rows in result\n", nrows);

	
	row = res->Next();

	*ADC2PE = atof(row->GetField(0));
	p0 = atof(row->GetField(1));
	p1 = atof(row->GetField(2));
	if (debug) printf("--ADC2PE=%f--p0=%f--p1=%f\n", *ADC2PE, p0, p1);

   delete row;

   delete res;
   
   delete db;

   cutinpe = (*DACt-p0)/(*ADC2PE*p1);		// this will give the number of pe used for the cut

   printf("DATA for this run was taken with %f pe threshold cut\n",cutinpe);

}
*/
//Functions for reading in the channels:
  int get_num_pmt(string filename) {

	ifstream file;
	file.open(filename);

	if (!file.is_open()) {
		cout << "Error: File could not be opened.";
		return -1;
	}

	string temp;

	getline(file, temp);

	int i = 0;
	for (; i < temp.length(); i++) { 

		if (isdigit(temp[i])) {
			break;
		}

	}

	temp = temp.substr(i, temp.length() - i);

	int no_pmts = 0;

	no_pmts = atoi(temp.c_str());

	if (no_pmts <= 0) {
		cout << "Error: Number of pmts can not be less than or equal to 0.";
		return -1;
	}

	file.close();

	return no_pmts;

}

int get_num_implemented_channels(string filename) {

	ifstream file;
	file.open(filename);

	if (!file.is_open()) {
		cout << "Error: File could not be opened.";
		return -1;
	}

	string temp;

	getline(file, temp);
	getline(file, temp);

	int i = 0;
	for (; i < temp.length(); i++) {

		if (isdigit(temp[i])) {
			break;
		}

	}

	temp = temp.substr(i, temp.length() - i);

	int no_channels = 0;

	no_channels = atoi(temp.c_str());

	if (no_channels <= 0 || no_channels > 64 || no_channels%2 != 0) {
		cout << "Error: Number of implemented channels can not be less than or equal to 0 nor greater than 64. Also can not be an odd number." << endl;
		return -1;
	}

	file.close();

	return no_channels;
}

vector<vector<int>> get_channels_on_pmt(string filename, int num_pmts, int implemented_channels) {
	ifstream file;
	file.open(filename);

	string temp;
	getline(file, temp);
	getline(file, temp);
	getline(file, temp);
	getline(file, temp);

	vector<vector<int>> v;
	v.resize(num_pmts);

	int tempint=0;
	for(int j=0; j<num_pmts; j++){
		v[j].resize(implemented_channels);
		for(int k=0; k<implemented_channels; k++){
			file >> tempint;
			//cout << "j=" << j << ", k=" << k << ", tempint=" << tempint << endl;
			v[j][k] = tempint;			
		}
	}
	file.close();
	return v;

}

void get_pmt_xy_layer(string filename, int num_pmt, char pmt_xy[100], int pmt_layer[100]){

  ifstream file;
  file.open(filename);

  char temp_c;
  int temp_i;
  string temp_s;

  for(int i=0;i<num_pmt+5;i++){getline(file, temp_s);} //ignore the first n+5 lines of the file

  for(int i=1;i<=num_pmt;i++){
    file >> temp_c;
    pmt_xy[i] = temp_c;
  }
  for(int i=1;i<=num_pmt;i++){
    file >> temp_i;
    pmt_layer[i] = temp_i;
  } 
return;
}

//Get Left Channel deleted
//Get Right Channel deleted




//# End of Axel Addition
//Takes the TChain of events, finds and prints the timing offsets between pmts
/*void do_offsets(TChain schain){

  int pmt_no[4] = {3,7,1,2};
  const int numpairs = 6;

  TH1D *h_off[numpairs]; 
  TH1D *h_off_high[numpairs]; //for testing
  int offcounter=0;
  
  for(int i=0; i<3; i++){
    for(int j=i; j<3; j++){
      h_off[offcounter] = new TH1D(Form("h_off_%d_%d",pmt_no[i],pmt_no[j+1]),Form("Offset between PMT-%d and PMT-%d",pmt_no[i],pmt_no[j+1]),100000,0.0,100000.0);
      h_off_high[offcounter] = new TH1D(Form("h_off_high_%d_%d",pmt_no[i],pmt_no[j+1]),Form("High offset between PMT-%d and PMT-%d",pmt_no[i],pmt_no[j+1]),100000,0.0,100000.0);
      cout << "Pair "<< offcounter << ": PMT-" << pmt_no[i] << " and PMT-" << pmt_no[j+1] << endl;
      offcounter++;
    }
  }

  vector< Float_t > pack;
  vector< vector< vector<Float_t> > > p_mod; // of the form mod 2, 1, 7, 3
  p_mod.resize(4);

  bool check = 0;

  int num_entries = schain.GetEntries();

  for(int i=0; i<num_entries; i++){
    schain.GetEntry(i);

    pack.clear();
    pack.push_back(stime_sec_low);
    pack.push_back(stime_sec_high);
    pack.push_back(stime_16ns_low);
    pack.push_back(stime_16ns_high);

    if(slen >= 3 && slen <= 8){

      if(i%10000 == 9999){
        check = 1;
      }
      if(check == 0){  
        for(int m=0; m<4; m++){ //loop over the pmts
          if(smodule == pmt_no[m]){
	    p_mod[m].push_back(pack); //Add the time packet to the corresponding vector
	  }
	}
      }

      if(check == 1){ //Only doing this loop every 10k entries
        int counter = 0; //for counting which pair we're looking at
	for(int first_pmt=0; first_pmt<3; first_pmt++){
	  int first_size = p_mod[first_pmt].size();  //Number of time packets for the first pmt in the pair (2,1,7)
	  for(int second_pmt = first_pmt+1; second_pmt<4; second_pmt++){
	    second_size = p_mod[second_pmt].size(); //Number of time packets for the second pmt in the pair (1 7 3, 7 3, 3)

	    for(int ii = 0; ii<first_size; ii++){
	      for(int jj = 0; jj<second_size; jj++){ //Looping over all pairs of timing packets
		if(p_mod[first_pmt][ii].size() == p_mod[second_pmt][jj].size()){ //i.e. if both packets have all 4 entries
		  if(p_mod[first_pmt][ii].at(0) == p_mod[second_pmt][jj].at(0) && p_mod[first_pmt][ii].at(1) == p_mod[second_pmt][jj].at(1)){
		    //i.e. if the stime_sec_low and stime_sec_high are the same
		    h_off[counter]->Fill(p_mod[first_pmt][ii].at(2)-p_mod[second_pmt][jj].at(2));		    
		    h_off_high[counter]->Fill(p_mod[first_pmt][ii].at(3)-p_mod[second_pmt][jj].at(3)); //for testing		    
		  }
		}
	      }
	    }

	  counter++;
	  }
	}
	for(int k=0; k<4; k++){p_mod[k].clear();} //make way for the next 10k entries
        check = 0;
      }

    }
  } //end of loop over all entries

  int offsets[numpairs];
  offcounter = 0;

  TCanvas *c1 = new TCanvas( "c1", "" );  

  for(int i=0; i<3; i++){
    for(int j=i; j<3; j++){
      h_off[offcounter]->Draw("");
      c1->Print("offset_histos.pdf","pdf");
      h_off_high[offcounter]->Draw("");
      c1->Print("offset_histos_high.pdf","pdf");
      
      offsets[offcounter] = (int)h_off[offcounter]->GetBinLowEdge(h_off[offcounter]->GetMaximumBin());
      
      cout << "Offset between PMT-" << pmt_no[i] << " and PMT-" << pmt_no[j+1] << " is " << offsets[offcounter] << endl;

      offcounter++;
    }
  }  
}*/

void scanFiles(vector<string> &fileList, string inputDirectory){
    inputDirectory = inputDirectory.append("/");

    DIR *p_dir;
    const char* str = inputDirectory.c_str();
    
    p_dir = opendir(str);
    if( p_dir == NULL)
    {
        cout << "can't open :" << inputDirectory << endl;
    }   
    
    struct dirent *p_dirent;
    
    long tmp,max;
    max = 0;
    
    while ( (p_dirent = readdir(p_dir)))
    {
        string tmpFileName = p_dirent->d_name;
        if( tmpFileName.find("root") == -1) //Find only the root files
        {
            continue;
        }   
        else
        {
            //cout << "tmpFileName: " <<  tmpFileName << endl;
            if(tmpFileName[0] == 'b'){continue;} //This will be the baseline
            else{fileList.push_back(tmpFileName);} 
        }   
    }   
    closedir(p_dir);
    return;
}
//FCL file reader for bottom CRT backend and gain file.
vector <vector<string>> fcl_read(string filename)
{
    vector <vector<string>> output; 
	ifstream file (filename);
    int lineNum = 1;   //Line tracker
    bool keepReading = true; //Keeps track of when to stop reading.
    string line; 
    if (file.is_open()){
        while(getline(file,line)){
            if (line[line.length()-1] == ']' && line[line.length()-2] == ']'){ keepReading = false; /*cout<<"set to false at line "<<lineNum<<'\n';*/}
            if (line[0] == ']'){ keepReading = false; /*cout<<"set to false at line "<<lineNum<<'\n';*/}
            if (line[0] == '#'){ keepReading = false; /*cout<<"set to false at line "<<lineNum<<'\n';*/}
            if (line[0] == '[' && line.length()>2){ keepReading = true;  /*cout<<"set to true at line "<<lineNum<<'\n'*/;}
                if(keepReading){                        
                    vector<string> v;
                    stringstream ss(line);

                    while(ss.good()){
                        string sub;
                        getline(ss, sub, ',');
                        if(!sub.empty()){
                        v.push_back(sub);
                        }                        
                    }
                    if(v[0].length()>0){
                        v[0] = v[0].substr(1,v[0].length()-1);
                    }
                    if(v[v.size()-1].length()>0){
                        v[v.size()-1] = v[v.size()-1].substr(0,v[v.size()-1].length()-1);
                    }
                    //v is a vector where each element is an element of each row starting at usb# and ending at the last gain.
                    output.push_back(v);       
                    //cout<<"Pushed line: "<<lineNum<<'\n';                    
                } 
        lineNum++;
        }
        //Print out vector for debug:
		/*
        for (unsigned int i = 0; i < output.size(); i++){
          cout << "Line: "<<i+1 << "\n";
          for (unsigned int j = 0; j < output[i].size(); j++){
            cout<<"Value "<<j+1<<" :"<<output[i][j]<<"\n";
          }
        }
		*/
        return output;
		cout<<"Read FCL file succesfully."<<'\n';
        file.close();
        keepReading = false;
    }
    else{
        
        cout << "Unable to open file";
        //Returns empty 2d vector
        vector<string> v;
        v.push_back("");
        output.push_back(v);
        return output;
        
    }
        vector<string> v;
        v.push_back("");
        output.push_back(v);
        return output;
}

///////UNCOMMENT LATER//////////////////////
//Updates gain constants in fcl file for backend.
/*int update_gain(string filename,string gainfile)//filename of fcl file, filename of gain file
{
  //string filename = "crt_daq.fcl";
  //string gainfile = "gainfile.txt";

  
    string gains[65][65];
	//Gains are in a 2d array where the first dimension specifies the pmt number and the second houses its corresponding 64 gains
	//Reading gains from gain.txt file.
	ifstream file (gainfile);
    string line; 
	unsigned int col;
	string pmtNum;
	string chanNum;				
	string sub;
    if (file.is_open()){
		cout<<"Gain file: "<<gainfile<<" read in succesfully."<<'\n';
		while(getline(file,line)){
		stringstream ss(line);
		col = 1;
            while(ss.good()){
                getline(ss, sub, ' ');						
                if(!sub.empty()){
            	    if (col == 1){				
						pmtNum = sub;						
						col++;
					}
					else if (col == 2){
						chanNum = sub;
						col++;
					}
					else if (col == 3){						
						gains[stoi(pmtNum)][stoi(chanNum)] = sub;																
					}
                } 
				                    
            }

		}
	}
	//Print out gain file contents for debugging.
	
	//for (int i = 0; i<=64;i++){
	//	for (int j=0; j<=64;j++){
	//		if (!gains[i][j].empty()){
	//		cout << "Module Number: "<< i<< " Channel Number: "<<j<<" Gain: "<<gains[i][j]<<'\n';
	//		}
	//		
	//	}
	//}
	
	
	cout<< "Closing gain file."<<'\n';
	file.close();


	/////Done Reading Gain file//////
	/////Read and rewrite fcl file//////


    //Open the original fcl file
	vector <vector<string>> output = fcl_read(filename);//Read in fcl file values.
	ofstream fileout;	
	//Fill in new gains	
	
	int pmt_num;
    unsigned int firstGainCol = 11; //Hardcoded number for column number in which the first Gain number is found. (0 indexed)
    unsigned int pmtNumColumn = 2;//Hardcoded number for column in which the pmt board number is stored.
	for (unsigned int i = 0; i < output.size(); i++){		
		//cout<<"Gain values in row "<<i+1<<'\n';
		
		for (unsigned int j = firstGainCol; j < output[i].size(); j++){
			pmt_num = stoi(output[i][pmtNumColumn]);//Get the pmt number for which gain files we are looking for. 
			//cout<<"Pmt #: "<<pmt_num<<'\n';
			//cout<<"Original: "<<output[i][j];
			if (!gains[pmt_num][j-firstGainCol].empty()){
				output[i].at(j) = gains[pmt_num][j-firstGainCol];//replace gain values obtained from fcl file with the new ones.
			}
			//cout<<" New: "<<output[i][j]<<'\n';
		}
			
   		
	}	
	
	//Open file to write to, will overwrite original fcl file.
	//string temp = "temp_file.fcl";
	time_t t = time();
	string temp = filename+"_"+ctime(t);
	fileout.open(temp);
	//When file is open, add in all comments then add in all lines with new gain values.
	if (fileout.is_open()){
		cout<<"Opened fcl file:" << filename<< " to write to."<<'\n';
		fileout << "#USB_serial, PMT_Serial, board_number, HV, DAC_threshold, use_maroc2gain, gate, pipedelay, trigger_mode, force_trig, pmtboard_u, gain1, gain2, gain3, gain4, gain5, gain6, gain7, gain8, gain9, gain10, gain11, gain12, gain13, gain14, gain15, gain16, gain17, gain18, gain19, gain20, gain21, gain22, gain23, gain24, gain25, gain26, gain27, gain28, gain29, gain30, gain31, gain32, gain33, gain34, gain35, gain36, gain37, gain38, gain39, gain40, gain41, gain42, gain43, gain44, gain45, gain46, gain47, gain48, gain49, gain50, gain51, gain52, gain53, gain54, gain55, gain56, gain57, gain58, gain59, gain60, gain61, gain62, gain63, gain64" << '\n';//First line
		fileout << "#" << '\n';//Second line
		fileout << "#" << '\n';//Third Line
		fileout << "channel_configuration:" << '\n';//Fourth Line
		fileout << "[" << '\n';//Fifth Line open bracket for values

        //Loops through all values obtained from reading fcl file then adds them to each row, including new gain constants.
		
		for (unsigned int i = 0; i < output.size(); i++){ //Loop through each row of values from fcl file
          fileout << "[";//Start a row of values
          for (unsigned int j = 0; j < output[i].size(); j++){//Loop through each column of values of each row of fcl file.									
			fileout << output[i][j]<<','; //For all but final value in row								
          }		
		  fileout << "]";	
		  if (i<output.size()-1){
			fileout << "," <<'\n';
		  }
		  else{
			fileout << '\n';
		  }
        }
		
		
		fileout << "]" << '\n';//Last Line	
		cout<<"Closing updated fcl file."<<'\n';	
		fileout.close();
	}
	else{
	cout << "Unable to open file";	
	fileout.close();
	}
	
return 0;
}*/

