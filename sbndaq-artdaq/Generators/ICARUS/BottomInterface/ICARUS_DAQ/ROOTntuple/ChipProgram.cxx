#include <stdio.h>
#include <stdlib.h>
#include <TFile.h>
#include <TROOT.h>
#include <sys/stat.h>
#include <fstream>
#include <iostream>
#include <deque>
#include <vector>
#include <TNtuple.h>
#include <TH1.h>
#include <TH2.h>
#include <TString.h>

//using namespace std;
using std::cout;
using std::endl;
using std::ifstream;
using std::cerr;
using std::ios_base;

TNtuple *ntuple_sig;
//void decode(char *file, TH2D *histo[]);
void ChipProgram(TString sigfile, TString outfile);
//void decode_working(TNtuple *ntup, TString file);
void decode_working(TString file);
void got_word(UInt_t d);
void check_data();
Bool_t check_debug(UInt_t d);
void flush_extra();
void proc(Int_t line[]);

UInt_t words = 0;
Bool_t got_hi = kFALSE;
UInt_t time_hi=0;
Long64_t mytime=0;
std::deque<int> data;           // 11
std::deque<int> extra;          // leftovers
UInt_t word_index = 0;
UInt_t word_count[4] = {0, 0, 0, 0};
//TH2D *histogram[10];
struct stat fileinfo;
Int_t totalbytes,bytesleft, fsize;

//void decode(char *file, TH2D *histo[])
//void decode_working(TNtuple *ntup, TString file)
void decode_working(TString file)
{
  //char *file;
  //sprintf(file,filestring);
  char filedata[65535];//data buffer
  //for(Int_t i = 0; i<10; i++)
  //  histogram[i] = histo[i]; //Histogram

  ifstream myFile(file, ios_base::in | ios_base::binary);
  if(!myFile.is_open())
    return;
  
  if(stat(file, &fileinfo) == 0)//get file size
    //bytesleft = fileinfo.st_size;
    totalbytes = fileinfo.st_size;
  
  bytesleft=totalbytes;

  UInt_t word;           // holds 24-bit word being built, must be UInt_t 
  char exp = 0;        // expecting this type next
  //char byte[1];

  while(kTRUE)//loop over buffer packets
    {
      if(bytesleft < 65535)
	fsize = bytesleft;
      else
	{
	  fsize = 65535;
	  bytesleft -= 65535;
	}

      myFile.read(filedata, fsize);//read data of appropriate size

      for(Int_t bytedex = 0; bytedex < fsize; bytedex++)//loop over members in buffer
	{
	  //char payload = *bytes.byteme & 63; 
	  //char type = *bytes.byteme >> 6;
	  //printf("byte = %b\n",*byte);
	  char payload = filedata[bytedex] & 63; 
	  char type = (filedata[bytedex] >> 6) & 3;
	  //printf("payload = %b\n",payload);
	  //printf("type = %b\n",type);
	  if(type == 0) //not handling type very well.  
	    {
	      //cout << "type == 0" << endl;
	      exp = 1;
	      word = payload;
	    } 
	  else 
	    {
	      if(type == exp) 
		{
		  //cout << "type == exp" << endl;
		  word = (word << 6) | payload; //bitwise OR
		  if(++exp == 4) 
		    {
		      //cout << "++exp == 4" << endl;
		      exp = 0;
		      got_word(word); //24-bit word stored, process it
		    }
		} 
	      else
		{
		  //cout << "resetting..." << endl; 
		  exp = 0;
		}
	    }
	  //myFile.seekg(1, ios_base::cur);
	}
      if(fsize==bytesleft) //exit the loop after last read
	break;
    }
  //extra.push_back(data);
  flush_extra();
  myFile.close();
}

void got_word(UInt_t d) 
{
  //cout << "got_word called" << endl;
  words++;
  char type = (d >> 22) & 3;
  if(type == 1) 
    {                    // command word, not data
      /*
	UInt_t b1 = (d >> 16) & 63;
        UInt_t b2 = (d >> 8) & 255;
        UInt_t b3 = d & 255;
        flush_extra();
        printf(b1,b2,b3);
*/
    } 
  else if(type == 3) 
    {
      if(check_debug(d))
	return;
      //cout << "back here" << endl;
      data.push_back(d & 65535);//segfault this line
      //cout << "pushed" << endl;
      //printf("top of data = %b\n",data.front());
      check_data();
    } 
  else 
    {
      //printf "?,%06x\n", d; //another info type
    }
}

void check_data() 
{
  //cout << "check_data called" << endl;
  while(1) 
    {
      //cout << data.size() << endl;
      Bool_t got_packet = kFALSE;
      UInt_t got = data.size();
      //cout << data.size() << endl;
      if(!got)
	  break;
      if(data[0] == 0xffff) 
	{            // check header
	  //cout << "proper header" << endl;
	  if(!(got >= 2))
	    break;
	  //data.pop();
	  UInt_t len = data[1] & 255;
	  const UInt_t lenless = len-1;
	  if(len > 1) 
	    {
	      //cout << "proper len" << endl;
	      //cout << got << endl;
	      //cout << len+1 << endl;
	      if(!(got >= len + 1))
		break;
	      //cout << "big enough" << endl;
	      UInt_t par = 0;
	      Int_t packet[lenless];
	      for(UInt_t i = 1; i <= len; i++) 
		{ 
		  par ^= data[i];
		  //printf("par = %b\n",par);
		  if(i < len)
		    packet[i-1] = data[i];
		  //data.pop();
		}
	      //cout << par << endl;
	      if(!par) 
		{                 // check parity
		  //cout << "proper parity" << endl;
		  got_packet = kTRUE;
		  flush_extra();
		  //the "money" line
		  //cout << lenless << endl;
		  proc(packet);
		  //delete first few elements of data
		  data.erase(data.begin(),data.begin()+len+1); //(no longer)
		}
	    }
	}
      if(!got_packet)
	{
	  extra.push_back(data.front());
	  data.pop_front(); 
	}
    }
}

/*
  # c1 dac pmt
  # c2 dac
  # c5 XXXX  -skipped
  # c6 data_hi    - number of received data words
  # c6 data_lo
  # c6 lost_hi     - number of lost data words
  # c6 lost_lo
  # c8 timestamp_hi
  # c9 timestamp_lo
*/

Bool_t check_debug(UInt_t d)
{
  //cout << "check_debug called..." << endl;
  UInt_t a = (d >> 16) & 255;
  d = d & 65535;
  
  if(a == 0xc8) 
    {
      time_hi = d;
      got_hi = kTRUE;
      return 1;
    } 
  else if(a == 0xc9) 
    {
      if(got_hi) 
	{
	  //UInt_t time = (time_hi << 16) | d;
	  mytime = (time_hi << 16) | d;
	  got_hi = kFALSE;
	  flush_extra();
	  //print "t,$mytime\n";
	}
      return 1;
    } 
  else if(a == 0xc5) 
    {
      word_index = 0;
      return 1;
    } 
  else if(a == 0xc6) 
    { 
      word_count[word_index++] = d;
      if(word_index == 4) 
	{
	  UInt_t t = (word_count[0] << 16) + word_count[1];
	  UInt_t v = (word_count[2] << 16) + word_count[3];
	  UInt_t diff = t - v - words;
	  if(diff < 0) { diff += (Long64_t)1 << 32; }
	  flush_extra();
	  //print "n,$t,$v,$words,$diff\n";
	}
      return 1;
    } 
  else if(a == 0xc1) 
    {
      flush_extra();
      //print "dac,$d\n";
      return 1;
    } 
  else if(a == 0xc2) 
    {
      flush_extra();
      //print "delay,$d\n";
      return 1;
    } 
  else
    return 0;
}

void flush_extra() 
{
  if(!extra.empty()) 
      extra.clear();
}

//void proc(TNtuple *ntuple,Int_t line[]){
void proc(Int_t line[]){
  //this is a modification on decode.C, maintained by Matt Toups
  //cout << "ntuple called" << endl;
  ///char tt[1];
  ///Bool_t myflag = false;  // Flag to determine if timing information has been found
  ///const Int_t n = 150; //  Array length to hold decoded files
  const Int_t nvars = 71; //71 variables defined for ntuple
  Int_t mod_no, type, line_len, time_high, time_low, unix_high, unix_low, channel;
  //Long64_t time = 0; // unix time stamp
  //Int_t counter = 0;
  //TString x;
  //TString y=","; // Character that we need to parse the csv files with
  //vector<TString> z;
  Float_t line_data[nvars]; // Array to fill ntuple
  for(Int_t i = 0; i<nvars; i++)
    line_data[i] = 0;
  
  line_len = (line[0] & 0xff) - 1; // A variable related to the number of words in the packet
  mod_no = (line[0] >> 8) & 0x7f; // PMT board number
  type = line[0] >> 15; // 1 = ADC, 0 = trigger
  unix_high = line[1];  //top 16 bits of 16ns clock timer
  unix_low = line[2];  //low 16 bits of 16ns clock timer
  
  time_high = mytime/65536; // top 16 bits of unix time stamp
  time_low = mytime - (time_high*65536); //low 16 bits of unix time stamp
  
  // Non-hit info into data array (Float_t) so we can fill the ntuple later
  line_data[nvars-7]=mod_no;
  line_data[nvars-6]=type;
  line_data[nvars-5]=(line_len-3)/2; // This is the number of hits in the packet
  line_data[nvars-4]=time_high;
  line_data[nvars-3]=time_low;
  line_data[nvars-2]=unix_high;
  line_data[nvars-1]=unix_low;
  
  //packet reading.  Note: packets are NOT of a fixed length
  if(type == 1) {        //ADC PACKET
    line_len = (line_len - 3 ) / 2; // Now len becomes the number of hits in the packet
    for(Int_t m=0;m<=(line_len-1);m++) { //Loop over all hits in the packet
      channel = line[4+2*m]; // Channel number of the hit
      line_data[channel] = line[3+2*m]; // Enter hit info in appropriate array entry
    }
    ntuple_sig->Fill(line_data);
  }
  // Note: This trigger packet reading code has still not been tested!!!!!!
  else if(type == 0) { //TRIGGER PACKET
    line_len=0;
    for(Int_t w=0;w<=3;w++)        {
      Int_t tmp = line[3+w] + 0;
      for(Int_t q=0;q<=15;q++) {
	line_data[w*16+q] = tmp & 1;
	if(line_data[w*16+q]==1) { line_len++; }
	tmp >>=1;
	//cout << "trigger packet -- channel= "<< m*16+n << ",  data= "<< data[m*16+n]<< endl;
	//gets(t);
      }
    }
    line_data[nvars-5]=line_len;
    ntuple_sig->Fill(line_data); //Not really appropriate for trigger packets
  }
  else {
    cerr << "Could not detect packet type." << endl;
    return;
  }

  //packet reading.  Note: packets are NOT of a fixed length
  /*
  if(type == 1) 
    {        //ADC PACKET
      len = (len - 3) / 2; // Now len becomes the number of hits in the packet
      for(Int_t m=0;m<=(len-1);m++) { //Loop over all hits in the packet
	channel = line[4+2*m]; // Channel number of the hit
	data[channel] = line[3+2*m]; //Hit info into appropriate array entry
      }
      for(Int_t i = 0; i < 64; i++)
	if(data[i] > 0)
	  histogram[(Int_t)data[64]-1]->Fill(i+1,data[i]);
	  
    }
  // Note: This trigger packet reading code has still not been tested!!!!!!
  else if(type == 0) 
    { //TRIGGER PACKET
      
      for(Int_t m=0;m<=3;m++)        {
	Int_t tmp = line[3+m] + 0;
	for(Int_t n=0;n<=15;n++) {
	  data[m*16+n] = tmp & 1;
	  tmp >>=1;
	}
      }
      //ntup->Fill(data); //Not really appropriate for trigger packets 
      
    }
  else {
    cerr << "Could not detect packet type." << endl;
    return;
  }
  */
} 

//void ChipProgram(TString sigfile, TString basefile, TString outfile){
//void ChipProgram(TString sigfile, TString outfile){
//void ChipProgram(TString sigfile, TString outfile){
//int main(TString sigfile, TString outfile){
int main(int argc, char* argv[])
{
  if(argc < 3) {
    cerr << "Insufficient number of arguments. Usage:\n";
    cerr << "./ChipProgram <infilename> <outfilename>\n";
  }
  TString sigfile = argv[1];
  TString outfile = argv[2];
  Bool_t update = false;

  TFile *f = new TFile(outfile, "NEW");
  if(!f->IsOpen()){
    TFile *f = new TFile(outfile, "UPDATE");
    cout << "Updating existing ntuples..." << endl;
    //TNtuple * ntuple_base = (TNtuple*)f->Get("ntuple_base");
    //TNtuple * ntuple_sig = (TNtuple*)f->Get("ntuple_sig");
    ntuple_sig = (TNtuple*)f->Get("ntuple_sig");
    update = true;
  }

  // Create the appropriate Ntuple and fill it by calling the appropriate decoder function based on the format type of the data
  if(!update){
    cout << "Reading data..." << endl;
    ntuple_sig = new TNtuple("ntuple_sig","data from file","s1:s2:s3:s4:s5:s6:s7:s8:s9:s10:s11:s12:s13:s14:s15:s16:s17:s18:s19:s20:s21:s22:s23:s24:s25:s26:s27:s28:s29:s30:s31:s32:s33:s34:s35:s36:s37:s38:s39:s40:s41:s42:s43:s44:s45:s46:s47:s48:s49:s50:s51:s52:s53:s54:s55:s56:s57:s58:s59:s60:s61:s62:s63:s64:smodule:stype:slen:stime_sec_high:stime_sec_low:stime_16ns_high:stime_16ns_low");
    //TNtuple *ntuple_base = new TNtuple("ntuple_base","data from file","b1:b2:b3:b4:b5:b6:b7:b8:b9:b10:b11:b12:b13:b14:b15:b16:b17:b18:b19:b20:b21:b22:b23:b24:b25:b26:b27:b28:b29:b30:b31:b32:b33:b34:b35:b36:b37:b38:b39:b40:b41:b42:b43:b44:b45:b46:b47:b48:b49:b50:b51:b52:b53:b54:b55:b56:b57:b58:b59:b60:b61:b62:b63:b64:bmodule:btype:blen:btime_sec_high:btime_sec_low:btime_16ns_high:btime_16ns_low");

    //decode_working(ntuple_base,basefile); //fills ntuple_base with csv basefile
    //decode_working(ntuple_sig,sigfile); //fills ntuple_sig with csv sigfile
    decode_working(sigfile); //fills ntuple_sig with csv sigfile
  }
  else if(update) {
    cout << "Reading data and appending..." << endl;
    //decode_working(ntuple_sig,sigfile); //decoder(ntuple_base,basefile);
    decode_working(sigfile); //decoder(ntuple_base,basefile);
  }

  ntuple_sig->Write("ntuple_sig",TObject::kWriteDelete);
  //ntuple_base->Write("ntuple_base",TObject::kWriteDelete);
  cout << "Finished filling ntuples." << endl;
  if(!update) {
    delete ntuple_sig;
  }
  f->Close();
}
