
#ifndef CRT_h
#define CRT_h

#include <stdio.h>
#include <cstring>
#include <vector>
#include <string>

//using namespace std;
using std::cout;
using std::endl;
using std::string;
using std::vector;
using std::ifstream;
using std::to_string;

namespace Bottom {

void reset();

void initializeboard(string define_runnumber, int trigger_num, int pmtini, int pmtfin, string online_path);

void process_signal(int usb);

void loadconfig(string mode_local, int usb_board, int pmt_board, string filename);
int scanFiles(string inputDirectory);
void starttakedata(int pmtini, int pmtfin);
void stoptakedata(int pmtini, int pmtfin, string online_path);
void getpmtdata(int usbboard, int pmtnumber);
void SplitString(string s, vector<string> &v, string c);
int eventbuilder(string DataPath, int totalusb, int totalpmt, string online_path);
int getnumpmt();
}
#endif /* CRT_h */
