#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <ctype.h>
#include <vector>
#include <cmath>
#include <sstream>
#include <regex>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <string>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include "usbreadout.h"
#include "decode.h"
#include "CRT.h"

#include <tracemf.h>

#define TRACE_NAME "BottomCRTBackend_CRT_cc"


#define foreach(item, array) \
    for(int keep = 1, \
            count = 0,\
            size = sizeof (array) / sizeof *(array); \
        keep && count != size; \
        keep = !keep, count++) \
      for(item = (array) + count; keep; keep = !keep)

#define MAX 80 // max number of elements in the pmt board array
#define buf_size 1024

using std::cout;
using std::endl;
using std::string;
using std::vector;
using std::ifstream;
using std::istringstream;
using std::stringstream;
using std::to_string;

namespace Bottom {

int pmt;
static int totalpmt;
int usbhowmanyboards[2][10]={0};
int usbhowmanybox[2][10]={0};
int usbhowmanyboxcount = 0;
int boxtousb[128];
int boxtoboard[128];
//int pmt_local;
//int usb_local;
int pmt_usb_conv[100000];
int pmttousb[10];
int pmttoboard[10];
int pmtnumbers[10];
int structure[128]={-10};
int gateonoff;
int DACt;
int trigger_mode;
int fclload = 0;//mysql load flag set to 0 to start. 
int gain[65];
//static int pmtserialnumber;
int force_trig;
//int HVsetting;
//string gate;
int filelength = 20;
int run_length;
int usb_box;
int totalbox;
int usblocal[128];
//int boardnumber;
string pmtdata[128][128][11];
//string module;
int hdelay;
int gaindata[128][128][64];
int gapkey;
//int pipedelay;
string choice = "";
char run_number[16];
string usemaroc2gainconstantsmb;
//string comments;

int Year;
int Mon;
int Day;
int Hour;
int Min;
int Sec;


string mode;
string Time_Data_Path;

int top=-1;
int pmt_board_array[MAX];
int stack[MAX];


void push(int value)
{
	
	if(top==MAX-1)
	{}
	else
	{
		top=top+1;
		pmt_board_array[top] = value;
	}
}

void reset()
{

  for(int i=0; i<MAX;i++){
    pmt_board_array[i] = -1;
  }
}

void tarry(float sec)
{
  usleep( sec*1e6 );
}


void getpmtdata(int,int);
void SplitString(string s, vector<string> &v, string c);
int scanFiles(string inputDirectory);

////////////////////////////////////////////////////////////////////////////////
//Reads fcl file returns vector of values for each line. 
//Argument 1: Filename for .fcl file to be read.
vector<vector<string>> fcl_read(string filename)
{
    vector <vector<string>> output; 
    ifstream file(filename);

    int lineNum = 1;   //Line tracker
    bool keepReading = true; //Keeps track of when to stop reading.
    string line; 
    if (file.is_open()){
        while(getline(file,line)){
            if (line.length() < 2) { continue; } 
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
                } 
        lineNum++;
        }
        return output;
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
///////////////////////////////////////////////////////////////////////////////
void loadconfig(string mode_local, int usb_board, int pmt_board, int triggerbox,string filename){
    int usbhowmanyboardscount = 0;

    //Default mode for mode_local paramter.
    if(mode_local == "")
        mode = "fcl";
    else
        mode = mode_local;
    
    totalbox = 0;
    totalpmt= 0;
	
        if(mode.compare("debug") == 0){
        if(!usb_board || !pmt_board)
        {
            cout << "error cannot initialize need USB board and PMT board ! \n";
        }
        else{
            if(!triggerbox){
                cout << "Board " << pmt_board << " on USB " << usb_board << " parameters loaded \n";
                pmtdata[usb_board][pmt_board][0] = "none";     //pmt serial number
                pmtdata[usb_board][pmt_board][1] = to_string(pmt);  //board number
                pmtdata[usb_board][pmt_board][2] = to_string(0);  //HV
                string DebugDAC = "999";
                if(pmt_board == 1)
                    pmtdata[usb_board][pmt_board][3] = "900";        //DAC
                else if(pmt_board == 11)
                    pmtdata[usb_board][pmt_board][3] = "900";        //DAC
                else if(pmt_board == 21)
                    pmtdata[usb_board][pmt_board][3] = "900";        //DAC
                else if(pmt_board == 31)
                    pmtdata[usb_board][pmt_board][3] = "900";        //DAC
                else if(pmt_board == 41)
                    pmtdata[usb_board][pmt_board][3] = "900";        //DAC
                else if(pmt_board == 3)
                    pmtdata[usb_board][pmt_board][3] = "932";        //DAC
                else if(pmt_board == 14)
                    pmtdata[usb_board][pmt_board][3] = "932";        //DAC
                else
                    pmtdata[usb_board][pmt_board][3] = DebugDAC;   //DAC
                pmtdata[usb_board][pmt_board][4] = "no";   //use m2 gains
                pmtdata[usb_board][pmt_board][5] = "off";  //gate (on,off,...)
                if(pmt_board <= 40){
                    pmtdata[usb_board][pmt_board][6] = to_string(20);  //pipedelay
                }
                else{
                    pmtdata[usb_board][pmt_board][6] = to_string(21);  //pipedelay
                }
                pmtdata[usb_board][pmt_board][7] = to_string(0);    //op87
                pmtdata[usb_board][pmt_board][8] = to_string(0);    //op75
                pmtdata[usb_board][pmt_board][9] = "debug mode";    //comments
                pmtdata[usb_board][pmt_board][10] = to_string(0);   //module number
                totalpmt++;
                pmttousb[totalpmt] = usb_board;
                pmttoboard[totalpmt] = pmt_board;
                cout << "totalpmt = " << totalpmt << "\n";
                // should see what to do in initializeboard
                structure[usb_board] = 0;
                usbhowmanyboards[1][usbhowmanyboardscount] = usb_board;
                usbhowmanyboards[0][usbhowmanyboardscount]++;
                usbhowmanyboardscount++;
            }
            else{
                pmtdata[usb_board][pmt_board][0] = "none";     //pmt serial number
                pmtdata[usb_board][pmt_board][1] = to_string(pmt);  //board number
                pmtdata[usb_board][pmt_board][2] = "-999";  //HV
                pmtdata[usb_board][pmt_board][3] = "932";        //DAC
                pmtdata[usb_board][pmt_board][4] = "no";   //use m2 gains
                pmtdata[usb_board][pmt_board][5] = "off";  //gate (on,off,...)
                if(pmt_board <= 40){
                    pmtdata[usb_board][pmt_board][6] = to_string(20);  //pipedelay
                }
                else{
                    pmtdata[usb_board][pmt_board][6] = to_string(21);  //pipedelay
                }
                pmtdata[usb_board][pmt_board][7] = to_string(0);    //op87
                pmtdata[usb_board][pmt_board][8] = to_string(0);    //op75
                pmtdata[usb_board][pmt_board][9] = "debug mode";    //comments
                pmtdata[usb_board][pmt_board][10] = to_string(0);   //module number
                totalbox++;
                boxtousb[totalbox] = usb_board;
                boxtoboard[totalbox] = pmt_board;
                // should see what to do in initializeboard
                structure[usb_board] = 0;
            }
        }
    }
	else if(mode.compare("fcl") == 0){ //read from FiCHL file if mode = fcl        
        vector<vector<string>> output_fcl = fcl_read(filename);
        totalpmt = 0;
        totalbox = 0;
        usb_box = 0;
        int usbtemp;
        int pmttemp;
        for (unsigned int i=0; i< output_fcl.size(); i++){
            TLOG(TLVL_DEBUG) << "Attempting to parse fcl parameter ["<<i<<"][0] = \""<<output_fcl[i][0]<<"\"";
            usbtemp = std::stoi(output_fcl[i][0]);        
            TLOG(TLVL_DEBUG) << "Attempting to parse fcl parameter ["<<i<<"][1] = \""<<output_fcl[i][0]<<"\"";
            pmttemp = std::stoi(output_fcl[i][2]);
	    pmtdata[usbtemp][pmttemp][0] = output_fcl[i][1];//
	    pmtdata[usbtemp][pmttemp][1] = output_fcl[i][2];//
	    pmtdata[usbtemp][pmttemp][2] = output_fcl[i][3];//
	    pmtdata[usbtemp][pmttemp][3] = output_fcl[i][4];//
	    pmtdata[usbtemp][pmttemp][4] = output_fcl[i][5];//
	    pmtdata[usbtemp][pmttemp][5] = output_fcl[i][6];//
	    pmtdata[usbtemp][pmttemp][6] = output_fcl[i][7];//
	    pmtdata[usbtemp][pmttemp][7] = output_fcl[i][9];//
	    pmtdata[usbtemp][pmttemp][8] = output_fcl[i][8];//
	    pmtdata[usbtemp][pmttemp][9] = "fcl_mode";//
	    pmtdata[usbtemp][pmttemp][10] = output_fcl[i][10];//            
            if(pmtdata[usbtemp][pmttemp][2] == "-999"){
                for(int index = 0; index<64; index++){
                gaindata[usbtemp][pmttemp][index] = 16;
                }
                totalbox++;
                boxtousb[totalbox] = usbtemp;     //usb number
                boxtoboard[totalbox] = pmttemp;   //board number
                structure[usbtemp] = 0;
                usbhowmanybox[0][usbhowmanyboxcount] = usbtemp;
                usbhowmanybox[1][usbhowmanyboxcount]++;
                usbhowmanyboxcount++;
                usb_box = usbtemp;                //unique usb to have boxes connected
            }
            else{
                for (int j=0;j<64;j++){
                  TLOG(TLVL_DEBUG) << "Attempting to parse fcl parameter ["<<i<<"]["<<(j+11)<<"] = \""<<output_fcl[i][0]<<"\"";
                gaindata[usbtemp][pmttemp][j] = std::stoi(output_fcl[i][j+11]);
                }
                totalpmt++;				
                pmttousb[totalpmt] = usbtemp;
                pmttoboard[totalpmt] = pmttemp;
                structure[usbtemp] = 0;
                usbhowmanyboards[1][usbhowmanyboardscount] = usbtemp;
                usbhowmanyboards[0][usbhowmanyboardscount]++;
                pmtnumbers[totalpmt] = pmttemp;
                usbhowmanyboardscount++;                
            }
        }
	fclload = 1;	
	} 
}

int getnumpmt(){
  return totalpmt;
}

//////////////////////////////////////////////////////////////////////////////
int scanFiles(string inputDirectory){
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
        if( tmpFileName == "." || tmpFileName == "..")
        {
            continue;
        }
        else
        {
            tmpFileName.erase(0,4);
            if(tmpFileName[0] == '0'){
                tmp = stol(tmpFileName);
                if(tmp > max)
                {
                    max = tmp;
                }
            }
        }
    }
    closedir(p_dir);
    return max+1;
}

////////////////////////////////////////////////////////////////////////
void starttakedata(int pmtini, int pmtfin, int boxini, int boxfin)
{
    
    if(!pmtini && !pmtfin)                          // both not defined
    {
        pmtini = 1;
        pmtfin = totalpmt;
    }
    
    if(mode.compare("debug")==0)
    {
        int totalboard = pmtfin - pmtini + 1;
        printf("%d \t %d \t\n",totalpmt, totalboard);
        if(totalpmt != totalboard)
        {
            printf("Problem with initialization at starttakedata \n");
        }
    }
    else
    {
        pmtini = 1;
        pmtfin = totalpmt;
    }
    
    if (!boxini && !boxfin)                           // both not defined
    {
        boxini = 1;
        boxfin = totalbox;
    }
    
    if(mode.compare("debug")==0)
    {
        int totalfans = boxfin - boxini + 1;
        printf("%d\t %d\t \n", totalfans, totalbox);
        if(totalbox != totalfans)
        {
            printf("Problem with initialization of fan-in modules at starttakedata \n");
        }
    }
    else
    {
        boxini = 1;
        boxfin = totalbox;
    }
   

    int pmt1;
    int usb_local;
    int pmt_local;
    
    for(pmt1 = pmtini; pmt1<=pmtfin; pmt1++)
    {
        if(!pmttousb[pmt1] || !pmttoboard[pmt1])
        {
            if(pmttoboard[pmt1] != 0)
                printf("usb_local or pmt_local not defined.\n");
        }

        usb_local = pmttousb[pmt1];
        pmt_local = pmttoboard[pmt1];
        getpmtdata(usb_local, pmt_local);
        com_usb(usb_local, pmt_local, 254, 0);
        
	sleep(0.1);
    }
    
    int box1;
    for(box1 = boxini; box1<=boxfin; box1++)
    {
        if(!boxtousb[box1] || !boxtoboard[box1])
        {
            printf("usb_local or pmt_local not defined.\n");
        }
    }
    
   for(int pmt1 = pmtini; pmt1 <= pmtfin; pmt1++){
        usb_local = pmttousb[pmt1];
        pmt_local = pmttoboard[pmt1];
        if(structure[usb_local] == -1)                // only trigger box!!
        {
            set_inhibit_usb(usb_local, 0);            // -2; release inhibit for baseline
            					      // -1; inhibit writing data
            					      // 0; release inhibit for writing data
            structure[usb_local] = 1;
            printf("File open for writing for USB: %d\n",usb_local);
	    sleep(0.1);
        }
    }
    sleep(1.);

    time_t t1 = time(0);   //get time now
    struct tm * now = localtime( & t1 );
    
    Sec = now->tm_sec;
    Hour = now->tm_hour;
    Min = now->tm_min;
    Day = now->tm_mday;
    Mon = now->tm_mon;
    Year = now->tm_year;
    
    printf("..... Taking data .....\n");
    
    
}



////////////////////////////////////////////////////////////////

void stoptakedata( int pmtini, int pmtfin, int boxini, int boxfin, string online_path)
{
    if(!pmtini && !pmtfin)                           // both not defined
    {
        pmtini = 1;
        pmtfin = totalpmt;
    }
    
    if(mode.compare("debug")==0)
    {
        int totalboard = pmtfin - pmtini + 1;
        if(totalpmt != totalboard)
        {
            printf("Problem with initialization at stoptakedata \n");
        }
    }
    else
    {
        pmtini = 1;
        pmtfin = totalpmt;
    }
    
    
    if(!boxini && !boxfin)                            // both not defined
    {
        boxini = 1;
        boxfin = totalbox;
    }
    
    if(mode.compare("debug")==0)
    {
        int totalfans = boxfin - boxini + 1;
        if(totalbox != totalfans)
        {
            printf("Problem with initialization at stoptakedata \n");
        }
    }
    else{
        boxini = 1;
        boxfin = totalbox;
    }
    
    time_t t2 = time(0);   //get time now
    struct tm * now1 = localtime( & t2 );
    
    int newSec = now1->tm_sec;
    int newHour = now1->tm_hour;
    int newMin = now1->tm_min;
    int newDay = now1->tm_mday;
    
    int elapsed_time;
    elapsed_time = (newDay - Day)*24*3600 + (newHour - Hour)*3600 + (newMin - Min)*60 + (newSec - Sec);
    
    printf("%d sec...", elapsed_time);
    
    int pmt1;
    int usb_local;
    int pmt_local;
    
    for(pmt1 = pmtini; pmt1<=pmtfin; pmt1++)
    {
        if(!pmttousb[pmt1] || !pmttoboard[pmt1])
        {
            if(pmttoboard[pmt1] != 0)
                printf("usb_local or pmt_local not defined.\n");
        }
        usb_local = pmttousb[pmt1];
        pmt_local = pmttoboard[pmt1];
        getpmtdata(usb_local, pmt_local);
        com_usb(usb_local, pmt_local, 255, 0);                     // disable trigger
        com_usb(usb_local, pmt_local, 73, 0b00000);
        sleep(0.2);                                               // give it some time
    }
    
    sleep(5.0);
    printf("shutting down ");
    
    for(int m = 0;m<=10; m++)
    {
        printf(".");
        sleep(0.3);
    }
    printf("\n");
    
  for(int pmt1 = pmtini; pmt1 <= pmtfin; pmt1++){
        usb_local = pmttousb[pmt1];
        pmt_local = pmttoboard[pmt1]; 
        if(structure[usb_local] == 1 || structure[usb_local] == 0)
        {
            set_inhibit_usb(usb_local, -1);                       
            // -2; release inhibit for baseline
            // -1; inhibit writing data
            // 0; release inhibit for writing data
            structure[usb_local] = -1;
        }
    }
    
    sleep(1);

    long runtmp = scanFiles(online_path + "/runs1/DATA");
    
        sprintf(run_number,"%.7ld",runtmp - 1); //ScanFiles returns 1+ the current max run#
}

///////////////////////////////////////////////////////////////////////////

void getpmtdata(int usbboard, int pmtnumber)
{
    int pmtserialnumber = atoi(pmtdata[usbboard][pmtnumber][0].c_str());
    int boardnumber = atoi(pmtdata[usbboard][pmtnumber][1].c_str());
    int HVsetting = atoi(pmtdata[usbboard][pmtnumber][2].c_str());
    DACt = atoi(pmtdata[usbboard][pmtnumber][3].c_str());
    usemaroc2gainconstantsmb = pmtdata[usbboard][pmtnumber][4];
    string gate = pmtdata[usbboard][pmtnumber][5];
    int pipedelay = atoi(pmtdata[usbboard][pmtnumber][6].c_str());
    force_trig = atoi(pmtdata[usbboard][pmtnumber][7].c_str());
    trigger_mode = atoi(pmtdata[usbboard][pmtnumber][8].c_str());
    string comments = pmtdata[usbboard][pmtnumber][9];
    string module = pmtdata[usbboard][pmtnumber][10];
    
    if(gate.compare("on") == 0)
    {
      gateonoff = 0b11010; ////0bxyzab - x=gate on/off, y=trigger on, z=force readout of 64 channels,
      // a=adc info, b=hit info only
      
    }
    else if(gate.compare("off") == 0)
    {
        gateonoff = 0b01010;
    }
    else if(gate.compare("trigger") == 0)
    {
        gateonoff = 0b01011;   //highest bit -> en_gate,en_input,en_all,en_adc,en_hit
    }
    else if(gate.compare("triggera") == 0)
    {
        gateonoff = 0b01111;
    }
    else if(gate.compare("alladc") == 0)
    {
        gateonoff = 0b11110;
    }
    else if(gate.compare("alladcoff") == 0)
    {
        gateonoff = 0b01110;
    }
    else if(gate.compare("test") == 0)
    {
        gateonoff = 0b00110;  //0b00110; //0b01010
    }
    else if(gate.compare("testalladc") == 0)
    {
      gateonoff = 0b01110;  ///decimal=14
    }
    else if(gate.compare("allhit") == 0)
    {
        gateonoff = 0b01001;
    }
    ///Load in from fcl file into gain or set to default gain.
    for(int index = 0; index<64; index++)
    {
        if(fclload)
        {
            gain[index+1] = gaindata[usbboard][pmtnumber][index];
        }
        else
        {
            gain[index+1] = 16;
        }
    }

}

////////////////////////////////////////////////////////////////////////////////////////// 
int scanCheckrateFiles(vector<string> &fileList, string inputDirectory){
    inputDirectory = inputDirectory.append("/");
    
    DIR *p_dir;
    const char* str = inputDirectory.c_str();
    
    p_dir = opendir(str);
    if( p_dir == NULL)
    {
        cout << "can't open :" << inputDirectory << endl;
    }
    
    struct dirent *p_dirent;
    
    while ( ( p_dirent = readdir(p_dir) ))
    {
        string tmpFileName = p_dirent->d_name;
        if( tmpFileName == "." || tmpFileName == "..")
        {
            continue;
        }
        else
        {
            fileList.push_back(tmpFileName);
        }
    }
    closedir(p_dir);
    return fileList.size();
}

/////////////////////////////////////////////////////////////////////

void SplitString(string s, vector<string> &v, string c)
{
    size_t pos1, pos2;
    pos2 = s.find(c);
    pos1 = 0;
    while(string::npos != pos2)
    {
        v.push_back(s.substr(pos1,pos2-pos1));
        
        pos1 = pos2 + c.size();
        pos2 = s.find(c,pos1);
    }
    if(pos1 != s.length())
        v.push_back(s.substr(pos1));
}

/////////////////////////////////////////////////////////////////////////////////////////////////////


void initializeboard(string define_runnumber, int trigger_num, int pmtini, int pmtfin, string online_path){

    int disk_num = 1;
    char DataPath[256] = "/DATA/";
    string ScratchLocal = online_path + "/runs"+to_string(disk_num)+"/DATA/";
    int pipedelay = 20;
    int structure_t0[128] = {-10};

    if(!pmtini && !pmtfin){
        pmtini = 1;
        pmtfin = totalpmt;
    }
    if(!mode.compare("debug")){
        int totalboard = pmtfin - pmtini + 1;
        if(totalpmt != totalboard){
            printf("totalpmt: %d\t totalboard: %d\n",totalpmt,totalboard);
            printf("Problem with initialization \n");
        }
    }
    int pmt1;
    int usbbase[20];
    int usbread;
 
    for(int i=0;i<pmtfin;i++){
        usbread = usbhowmanyboards[1][i];         //usbhowmanyboard[0] saves board number, usbhowmanyboard[1] saves usb number
        if(structure_t0[usbread]==0) {
	  set_inhibit_usb(usbread,-1);
	  structure_t0[usbread] = 1;
	}
    }
    
    sleep(1);
    
    //now the new folder and the new data path will be passed to the DAQ
    
    printf("DataPath=%s, Disk=%d\n",DataPath,disk_num);
    
    set_data_path(DataPath);
    //check_diskspace;
    set_data_disk(disk_num);  //setting $ScratchLocal
    
    
    //defining the run number
    int file_run;
    int max = 0;
    int files_run[20];
    
    time_t t1 = time(0);   //get time now
    struct tm * now = localtime( & t1 );
    
    Sec = now->tm_sec;
    Hour = now->tm_hour;
    Min = now->tm_min;
    Day = now->tm_mday;
    Mon = now->tm_mon;
    Year = now->tm_year;
    
    string date;
    
    date = to_string(Year) + to_string(Mon) + to_string(Day);
    
    long runtmp = scanFiles(online_path + "/runs1/DATA");
    // data_path = online_path+"/runs";
    
    if(define_runnumber.compare("auto") == 0){
        sprintf(run_number,"%.7ld",runtmp);
    }
    
    set_run_number(run_number);
    
    printf("pmtini = %d, pmtfin = %d\n", pmtini, pmtfin);
    
    printf("run number = %s\n",run_number);


    printf("Baseline data taking .");
    
    int t = 0;

    int usb_local;
    int pmt_local;
    
    for (pmt1 = pmtini; pmt1<=pmtfin; pmt1++){
        if(!(usb_local = pmttousb[pmt1] || !(pmt_local = pmttoboard[pmt1]))){
            printf("usb_local or pmt_local not defined.\n");
        }
        usb_local = pmttousb[pmt1];
        pmt_local = pmttoboard[pmt1];
        getpmtdata(usb_local, pmt_local);
        usb_usb(usb_local, 0 ,1);                       // turn auto token ON
        com_usb(usb_local, pmt_local, 110, 1);          // turn off lef off/on the PMT's board = 1/0
        com_usb(usb_local, pmt_local, 109, 1);          // vdd_fsb on
        com_usb(usb_local, pmt_local, 73, 0b00000);     // set up pmt module
        com_usb(usb_local, pmt_local, 255, 0);          // clear buffers
        com_usb(usb_local, pmt_local, 84, 255);         
        com_usb(usb_local, pmt_local, 74, 0b0100000);   // default gain
        com_usb(usb_local, pmt_local, 70, 0);           // load default
        com_usb(usb_local, pmt_local, 67, 0b000010);       // statea
        com_usb(usb_local, pmt_local, 68, 0b000000);       // stateb
        com_usb(usb_local, pmt_local, 69, 0b000000);       // statec
        com_usb(usb_local, pmt_local, 71, 0);           // rst_g to maroc
        com_usb(usb_local, pmt_local, 72, 0);           // write config was done twice
        com_usb(usb_local, pmt_local, 72, 0);           
        com_usb(usb_local, pmt_local, 73, 0b00110);           
        com_usb(usb_local, pmt_local, 80, 5);           // hold delay is variable. Has been fixed to 5 here
        dac_usb(usb_local, pmt_local, 1000);            // threshold value
        com_usb(usb_local, pmt_local, 85, 0);   // no pipe delay
        com_usb(usb_local, pmt_local, 75, 0b00010000);  // set up trigger mode for module
        com_usb(usb_local, pmt_local, 86, 0);           // edge strip mode is off
        com_usb(usb_local, pmt_local, 87, 0b00);           // no force trigger
        com_usb(usb_local, pmt_local, 74, 0b0100000);   // default gain for the 64 channels
        
        com_usb(usb_local, pmt_local, 109, 0);          // vdd_fsb off
        com_usb(usb_local, pmt_local, 254, 0);          // enable trigger
        sleep(1.0);
	for ( int i = 0; i < 10 ; i++){
            com_usb(usb_local, pmt_local, 81, 0);       // avoid first packets
	    printf(".");
        }
        usbbase[t] = usb_local;
        t++;
	sleep(0.2);
    }

    sleep(2.0); //looose previous packets
    
    for(int j = 0; j<t; j++){
        usb_local = usbbase[j];
        if(structure[usb_local]==0){
            set_inhibit_usb(usb_local, -3);              // -3; created file structure
            sleep(0.5);
            set_inhibit_usb(usb_local, -2);              // -2; release inhibit for baseline
            						 // -1; inhibit writing data
            						 // 0; release inhibit for writing data
            cout << "Baseline data taking, releasing inhibit for USB " << usb_local << endl;
	    structure[usb_local] = -2;
	    sleep(0.5);
        }
    }

    for(pmt1 = pmtini; pmt1 <= pmtfin; pmt1++){
        usb_local = pmttousb[pmt1];
        pmt_local = pmttoboard[pmt1];
        com_usb(usb_local, pmt_local, 254, 0);            // enable trigger
	for( int i = 1; i <= trigger_num; i++){
            com_usb(usb_local, pmt_local, 81, 0);         // take baseline hits - 500
	}
        com_usb(usb_local, pmt_local, 73, 0b00000);
    }
 
    sleep(2.0);                                             // wait late packets
    
    time_t t2 = time(0);   //get time now
    struct tm * now1 = localtime( & t2 );
    
    int newSec = now1->tm_sec;
    int newHour = now1->tm_hour;
    int newMin = now1->tm_min;
    int newDay = now1->tm_mday;
    
    int elapsed_time;
    elapsed_time = (newDay - Day)*24*3600 + (newHour - Hour)*3600 + (newMin - Min)*60 + (newSec - Sec);

    
    printf(": %d sec\n",elapsed_time);
    
    //now let's initialize everything for the data taking
    
    printf("Initializing .\n");
    
    string dir2 = ScratchLocal + "Run_" + run_number;
    string summary = "off";                                 // summary file default = off
 
    for (pmt1 = pmtini; pmt1 <= pmtfin; pmt1++){
        
        usb_local = pmttousb[pmt1];
        pmt_local = pmttoboard[pmt1];
        
        printf("Loading PMT: %d\n", pmt_local);
        
        getpmtdata(usb_local,pmt_local);
        com_usb(usb_local, pmt_local, 110, 1);             // turn off the three led on the PMT's board = 1
        com_usb(usb_local, pmt_local, 109, 1);             // vdd_fsb on
        com_usb(usb_local, pmt_local, 73, 0b00000);        // set up pmt module
        com_usb(usb_local, pmt_local, 255, 0);             // clear buffers
        com_usb(usb_local, pmt_local, 84, 255);            // buffer size limit
        com_usb(usb_local, pmt_local, 74, 0b0100000);      // default gain
        com_usb(usb_local, pmt_local, 70, 0);              // load default
        dac_usb(usb_local, pmt_local, DACt);               // threshold value
        com_usb(usb_local, pmt_local, 67, 0b000010);       // statea
        com_usb(usb_local, pmt_local, 68, 0b000000);       // stateb
        com_usb(usb_local, pmt_local, 69, 0b000000);       // statec
        com_usb(usb_local, pmt_local, 71, 0);              // rst_g to maroc
        com_usb(usb_local, pmt_local, 72, 0);              // write config
        com_usb(usb_local, pmt_local, 72, 0);              // write config
      
        com_usb(usb_local, pmt_local, 73, gateonoff);        //gateonoff);      // gate
        
        com_usb(usb_local, pmt_local, 75, trigger_mode);     // trigger mode (gateonoff = 0b01011; trigger_mode = 0b0110 0000);
        com_usb(usb_local, pmt_local, 80, 5);              // hold delay is variable. Has been fixed to 5 here
        com_usb(usb_local, pmt_local, 85, pipedelay);      // set up pipe delay
        com_usb(usb_local, pmt_local, 86, 0);              // edge strip mode
        com_usb(usb_local, pmt_local, 87, force_trig);     // force readout -> 01: 1msec, 10: 16msec, 11: 256msec
        com_usb(usb_local, pmt_local, 74, 0b0100000);      // default gain
        
        sleep(0.5);
        
        if(usemaroc2gainconstantsmb.compare("no")==0){
            com_usb(usb_local, pmt_local, 74, 0b0100000);  // default gain
        }
        else if(usemaroc2gainconstantsmb.compare("yes")==0 && fclload == 0){
            printf("Error cannot load mysql gain constants from MYSQL\n");
            com_usb(usb_local, pmt_local, 74, 0b0100000);  // default gain
        }
        else if(usemaroc2gainconstantsmb.compare("yes")==0 && fclload == 1){
            for(int index = 1 ; index < 65; index++){
                com_usb(usb_local, pmt_local, index - 1, 2*gain[index]);  // applying gain constants from MySQL
		}
        }
        
        sleep(1.0);

        
	com_usb(usb_local, pmt_local, 109, 0);              // vdd_fsb off
        
        // here create a first summary file or append to an existing one
     
        if(summary.compare("on")==0){
            string summaryfile = dir2 + "/summary.txt";
            string summaryfile1 = dir2 + "/summary_new.txt";
            
            ifstream efile(summaryfile.c_str());
            
            if(efile.good()){
                
                FILE * pFile2;
                pFile2 = fopen(summaryfile.c_str(),"a+");
                if(pFile2 == NULL){
                    printf("Can not open summaryfile1\n");
                    fclose(pFile2);
                }
                fprintf(pFile2,"\n");
                //fprintf(pFile2,"%d \t %d \t %d \t %d \t %d \t %s \t %d \t %d \t %d \t %d \t %s \t %s \t", pmt_local, pmtserialnumber, pmt_local,HVsetting, DACt, gate.c_str(), trigger_mode, pipedelay, filelength, run_length, usemaroc2gainconstantsmb.c_str(), comments.c_str());
                if(usemaroc2gainconstantsmb.compare("yes")==0){
                    for(int index = 1; index < 65; index++){
                        fprintf(pFile2, "%d \t", gain[index]);
                    }
                }
                efile.close();
                fclose(pFile2);
            }
            else{
                efile.close();
                FILE * pFile1;
                pFile1 = fopen(summaryfile.c_str(),"w");
                if(pFile1 == NULL){
                    printf("Can not open summaryfile\n");
                    perror("Failed: ");
                    fclose(pFile1);
                }
                //fprintf(pFile1,"%d \t %d \t %d \t %d \t %d \t %s \t %d \t %d \t %d \t %d \t %s \t %s \t", pmt_local, pmtserialnumber, pmt_local,HVsetting, DACt, gate.c_str(), trigger_mode, pipedelay, filelength, run_length, usemaroc2gainconstantsmb.c_str(), comments.c_str());
                if(usemaroc2gainconstantsmb.compare("yes")==0){
                    for(int index = 1; index < 65; index++){
                        fprintf(pFile1, "%d \t", gain[index]);
                    }
                }
                fclose(pFile1);
            }
            
        } // end for summary on or off
        
        
    } //end loop over pmt
    
    printf("finished initializing \n");

    //string cmdq = "ipcs -q";    
    //for(int e = 0; e<=10; e++){
    //    sleep(1.0);
    //    printf(".\n");
    //system(cmdq.c_str());
    //}

    for( int j = 0; j<t; j++){
        usb_local = usbbase[j];
        if(structure[usb_local] == -2){
            set_inhibit_usb(usb_local, -1);
            // -1; inhibit writing data for baseline
            // -1; inhibit writing data
            // 0; release inhibit for writing data
            structure[usb_local] = -1;
            usblocal[j] = usb_local;
	    sleep(0.01);
        }
    }

    sleep(1.0);
    
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int eventbuilder(string DataPath, int pmtini, int pmtfin, string online_path)
{


    if(DataPath=="auto"){
        int disk_num = 1;
	DataPath = online_path + "/runs1/DATA/Run_" + run_number;
	string mkdir_cmd0 = "mkdir -m a=rwx -p " + DataPath;
        system(mkdir_cmd0.c_str());
	string rem = "rm -f " + online_path + "/runs1/DataFolder";
	system(rem.c_str());

        string symb = "ln -sf " + DataPath + " " + online_path + "/runs1/DataFolder";
        system(symb.c_str());
    }
   
    if(!pmtini && !pmtfin){
        pmtini = 1;
        pmtfin = totalpmt;
    }
 
     FILE * OUT;
     string cmd1 = DataPath + "/baselines.dat"; //define the dir
     OUT = fopen(cmd1.c_str(),"w");
     if(OUT == NULL){
         printf("Can not open file - eventbuilder\n");
         fclose(OUT);
     }
     else //baselines.dat is open
     {
      
     //1>usbnum*100+mod  ->total pmt identifier
     //2>pmt channel
     //3> -> data : ADC counts 
     
     for(int pmt1 = pmtini; pmt1<=pmtfin; pmt1++) //loop over PMTs
     {
	if(!pmttousb[pmt1] || !pmttoboard[pmt1])
        {
            if(pmttoboard[pmt1] != 0)
                printf("usb_local or pmt_local not defined.\n");
        }

	string file = DataPath + "/binary/baseline_" + to_string(pmttousb[pmt1]);
        unsigned int otime = 0;
        unsigned long delta = 0;
        
        std::stringstream ss;
        auto old_buf = std::cout.rdbuf(ss.rdbuf());
        decode::dodecode(file.c_str());
        std::cout.rdbuf(old_buf);
	  
	  int hist[64][2048] = {0};

      	  string s;
      	  char buf[buf_size];//buf_size
          if(!ss)
	  {
	    cout << "Can not open file\n";
	  }
          else
	  { //get characters from stream
            //1. pointer to an array of chars where the string read is copied
            // 2. maximum number of characters copied to the stream
            // 3. stream, pointer to a file object that identifies an input stream

           while (std::getline(ss,s))
	      {
	      vector<string> line;
	      SplitString(s,line,","); //split string by comma 
	      if(strcmp(line[0].c_str(),"p") == 0)
                {  
		  //conver string into integer tmp
		  int tmp = atoi(line[1].c_str());
		  //left shift 
		  int mod = (tmp >> 8) & 127;  //pmt board number
                    int len = (tmp & 255) - 4;
                    unsigned int time = (atoi(line[2].c_str()) << 16) + atoi(line[3].c_str());
                    if(time<otime)
                        delta++;
                    otime = time;
                    time = (delta << 32) + time;
                    int type = tmp >> 15;
                    if(type)                //adc packet
                    {
                        len >>= 1;
                        for(int i = 0; i<=(len-1); i++)
                        {
			  //for each usb
			  //keep track of pmt number = mod
			  //pmt channel
			  //data
			  int data = atof(line[3 + 2 * i + 1].c_str());  //
			  int channel = atoi(line[4 + 2 * i + 1].c_str());  //Channels from 0-63

			  if(mod==pmttoboard[pmt1])
			  {
			    hist[channel][data]++;
			  }
			  
			}
                    }
                }
            }
        }
   
      //assume if the size in each subarray is same 

	      double min=0;
	      for(int i=0; i<=63; i++){
		//for each pmt loop over channels
		double n=0;
		int x=0;
		double dev=0; 
		double avg=0;	
		double f;

		for (int j=0; j<2048; j++) 
	        {
		    if(hist[i][j])
		    {
			x = j;
			f = hist[i][j];
			avg += f * x;
			n += f;
		    }
		}
		if(n) //fix the calculation of dev here
		{
		    avg = avg / n;
		    int t = 0;
		    for(t = 0;t<2048; t++){  //
		      if(hist[i][t])
		      {  
			  x = t;   //x is the channel number
			  f = hist[i][t];
			  dev += f * pow((x-avg),2);
		      }
		    }
		    dev = sqrt(dev/n);
		}
		fprintf(OUT,"%d,%d,%d,%f,%f,%f\n",pmttousb[pmt1],pmttoboard[pmt1],i,avg,dev,n);
		min = n;
	      }//loop over all the pm channels

		cout << "PMT " << pmttousb[pmt1] << "-" << pmttoboard[pmt1] << ":\t baseline hits: " << min << "\n";
	  
     }//end of loop over all PMTs (for loop pmt1++)
   }//end of else (putting things in baselines.dat)
fclose(OUT);

return 0;
    
}
}
