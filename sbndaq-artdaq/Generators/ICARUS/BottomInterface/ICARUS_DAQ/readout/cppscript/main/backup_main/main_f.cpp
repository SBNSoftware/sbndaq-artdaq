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
#include <mysql/mysql.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <dirent.h>

#define buf_size 1024
#define MSG_BUF_SIZE 32
#define MSG_BUF_SIZE_2 5

using namespace std;

int Msg_Base = 10000;
long MQs[128][128];
double DBH;
int Error=0;      // 0=no error; 1=error
int MQ_size;
char buf[buf_size];

struct mymsg{
    long     mtype;
    unsigned char     mtext[MSG_BUF_SIZE];
};

struct mymsg1{
    long     mtype;
    unsigned char     mtext[MSG_BUF_SIZE_2];
};

void handle(int sig){
    printf("mq timeout");
    Error = 1;
    return;
}


//find out which message queues exist
void find_mqs(){
  
    string cmd = "ipcs -q";
    FILE *myfile = popen(cmd.c_str(), "r");
    if(!myfile){
        cout << "Error to open\n";
        pclose(myfile);
    }
    else
    {
        int i = 0;
        string line;
        while(fgets(buf,buf_size,myfile) != NULL)
        {
            line = buf;
           // regex repPattern("(0[xX][0-9A-Fa-f]+)(\\s+)");
          //  smatch match;
          //  regex_match(line,match,repPattern);
          //  cout << "match_size = " << match.size() << "\n";
            if(line[1] == 'x'){
         //   if(regex_match(line,match,repPattern)){
                int tmp = (int)strtol(line.c_str(), NULL, 0);
                if(tmp >= Msg_Base){
                    int msg;
                    msg = msgget(tmp,0666 | IPC_CREAT);
                  //  msg = shmget(tmp, 4*sizeof(int), 0666 | IPC_CREAT);
                   // cout << "mq1 = " << tmp << ", mq0 = " << msg << "\n";
                    MQs[0][i]=msg;
                    MQs[1][i]=tmp;
                    i++;
                    MQ_size = i;
                }
            }
        }
        pclose(myfile);
    }
}

//mq_send $msg_id,$data
void mq_send(int msg, char data[]){
    int mq0;
    int mq1;
    struct mymsg msgp;
    memset(msgp.mtext,0,MSG_BUF_SIZE);
    MQ_size = 0;
    if(MQs[0][0]==0)
        find_mqs();
    while(1){
        try{
            signal(SIGALRM,handle);
            alarm(1);
            try{
                msgp.mtype = msg;
                strcpy((char *)msgp.mtext,data);
               // sprintf (msgp.mtext,"%d",data[0]);
               // msgp.mtext[0] = data[0];
               // int a = *((int *)msgp.mtext);
               // printf("disk = %d \n",a);
               // cout << "msgp = " << *((int *)msgp.mtext) << "\n";
                for(int i = 0; i<=MQ_size;i++){
                    msgsnd(MQs[0][i],(void*)&msgp,MSG_BUF_SIZE,IPC_NOWAIT | MSG_NOERROR);
                //    cout << "mq1 = " << MQs[1][i] << ",mq0 = " << MQs[0][i] << ",msg = " << msgp.mtype << ",data = " << *((int *)msgp.mtext) << ", " << msgp.mtext << "\n";
                }
            }
            catch(...){}
            alarm(0);
        }
        catch(...){}
        alarm(0);
        if(Error == 0)
            return;                             //success
        else
            break;                              //reraise other exceptions
        printf("Killing MQ mq1\n");
        system("ipcrm -Q mq1");                 //kill blocking mq
        find_mqs();                               //remove it from MQs
    }
}


//mq_send_str $msg_id,$data(string)
void mq_send_str(int msg, char data[]){
    int mq0;
    int mq1;
   // struct msgbuf msgp = (struct msgbuf*)malloc(sizeof(struct msgbuf) + 100);
    struct mymsg msgp;
    memset(msgp.mtext,0,MSG_BUF_SIZE);
    MQ_size = 0;
    if(MQs[0][0]==0)
        find_mqs();
    while(1){
        try{
            signal(SIGALRM,handle);
            alarm(1);
            try{
                msgp.mtype = msg;
                strcpy((char *)msgp.mtext, data);
                for(int i =0;i<=MQ_size;i++){
                    msgsnd(MQs[0][i],(void*)&msgp,MSG_BUF_SIZE,IPC_NOWAIT|MSG_NOERROR);    
                //    cout << "mq1 = " << MQs[1][i] << ",mq0 = " << MQs[0][i] << ",msg = " << msgp.mtype << ",data = " << msgp.mtext << "\n";
                }
            }
            catch(...){}
            alarm(0);
        }
        catch(...){}
        alarm(0);
        if(Error == 0)
            return;                             //success
        else
            break;                              //reraise other exceptions
        printf("Killing MQ mq1\n");
        system("ipcrm -Q mq1");                 //kill blocking mq
        find_mqs();                               //remove it from MQs
    }
}


//mq_send_usb $msg_id, $usb_num,$data;
void mq_send_usb (long msg, int usb_num, unsigned char data[]){
    int mq0;
    int mq1;
    struct mymsg1 msgp;
   // memset(msgp.mtext,0,4);
    memset(msgp.mtext,'\0',MSG_BUF_SIZE_2);
   // struct msgbu* msgp = (struct msgbuf*)malloc(sizeof(struct msgbuf) + sizeof(data));
    MQ_size = 0;
    if(MQs[0][0]==0)
        find_mqs();
    usb_num += Msg_Base;
    while(1){
        try{
            signal(SIGALRM,handle);
            alarm(1);
            try{
                msgp.mtype = msg;
               // strcpy((char *)msgp.mtext,(char *)data);
               // int msgsz = sizeof(msgp) - sizeof(long);
               for(int t =0;t<4;t++)
                   msgp.mtext[t] = data[t];
               // int tmp1 = *((int *)msgp.mtext);
               // printf ("tmp = %d\n", tmp1);
                for(int i=0; i<=MQ_size; i++){
                  //  cout << "mq1 = " << MQs[1][i] << ",mq0 = " << MQs[0][i] << ",msg = " << msgp.mtype << ",data = " << msgp.mtext << ",mdata= " << (unsigned int *) msgp.mtext <<"\n";
              //    printf("mdata = %d\n",(unsigned int *)msgp.mtext);
                   // for(int j = 0;j<4;j++)    printf("%d",msgp.mtext[j]);
                   // printf("\n");
                    mq1 = MQs[1][i];
                    if(mq1==usb_num){
                       // msgsnd(MQs[0][i],(void*)&msgp,msgsz, 0);
                       int rc = msgsnd(MQs[0][i],(void*)&msgp,4, IPC_NOWAIT | MSG_NOERROR); 
                       if(rc == -1){
                           perror("msgsnd");
                           exit(1);
                       }
                    }
                }
            }
            catch(...){}
            alarm(0);
        }
        catch(...){}
        alarm(0);
        if(Error == 0)
            return;                              //success
        else
            break;                               //raise other exceptions
        printf("Killing MQ mq1\n");
        system("ipcrm -Q mq1");                  //kill blocking mq
        find_mqs();                                //remove it from MQs
        
    }
}

void send_out(int usbdata){
  //  string data = to_string((unsigned int)usbdata);
    char data1[MSG_BUF_SIZE];
    memset(data1,'\0',MSG_BUF_SIZE);
  //  strcpy(data1,data.c_str());
  //  data1[0] = usbdata;
    memcpy(data1,&usbdata,sizeof(int));
    mq_send(1,data1);
}

void send_out_usb(int usb, unsigned int usbdata){
   // string data = to_string((unsigned long)usbdata);
    unsigned char data1[4];
    memset(data1,'\0',4);
   // strcpy(data1,data.c_str());
   // data1[0] = usbdata;
   // cout << "usbdata = " << usbdata << "\n";
  //  memcpy(data1,&usbdata,sizeof(int));
    data1[0] = usbdata & 0xff;
    data1[1] = (usbdata>>8) & 0xff;
    data1[2] = (usbdata>>16) & 0xff;
    data1[3] = (usbdata>>24) & 0xff;
    mq_send_usb(1,usb,data1);
    
}

void set_data_disk(int disk_num){
    char disknum1[MSG_BUF_SIZE];
    memset(disknum1,'\0',MSG_BUF_SIZE);
   // sprintf(disknum1,"%d",disk_num);
  //  char disknum1 = '0' + disk_num;
    disknum1[0] = disk_num;
  //  cout << "disk_num = " << *((int *)disknum1) << "," << disknum1 <<  "\n";
    mq_send(6,disknum1);
}

void set_run_number(char run_number[]){
    mq_send_str(5,run_number);
}

void set_data_path(char data_path[]){
   mq_send_str(7,data_path);
}

void com_usb(int usb, int pmt, int b2, int b3){
    unsigned int b1 = 4194304+((pmt & 63) * 256 + b2) *256 + b3;
 //   cout << "b2 = " << b2 << ", b3 = " << b3 << ", b1 = " << b1 << "\n";
    send_out_usb(usb, b1);
}

void set_inhibit_usb(int usb, int usbdata){
  //  string data = to_string((unsigned long)usbdata);
    unsigned char data1[MSG_BUF_SIZE_2];
    memset(data1,'\0',MSG_BUF_SIZE_2);
  //  strcpy(data1,data.c_str());
    memcpy(data1,&usbdata,sizeof(int));
    mq_send_usb(4,usb,data1);
}

void usb_usb(int usb, int adr, int data){
    int b1 = (adr << 16) + (data & 65535);
    send_out_usb(usb,b1);
}

void usb(int adr, int data){
    int b1 = (adr << 16) + (data & 65535);
    send_out(b1);
}

void dac_usb(int usb, int pmt, int data){
    com_usb(usb,pmt,64,(data & 255));
    data >>= 8;
    data = (1UL << (data & 31)) - 1;
    com_usb(usb,pmt,65,(data & 255));
    com_usb(usb,pmt,66,((data >> 8) & 255));
}


int pmt;
int totalpmt = 0;
int totalboard;
int usbhowmanyboards[2][10]={0};
int usbhowmanybox[2][10]={0};
int usbhowmanyboxcount = 0;
int usbhowmanyboardscount = 0;
int boxtousb[128];
int boxtoboard[128];
int pmt_local;
int usb_local;
int pmttousb[10];
int pmttoboard[10];
int structure[128];
int gateonoff;
int DACt;
int trigger_mode;
int mysqlload = 0;
int gain[64];
int pmtserialnumber;
int HVsetting;
string gate;
int filelength = 20;
int run_length;
int usb_box;
int totalbox;
int usblocal[128];
int boardnumber;
string pmtdata[128][128][11];
string module;
int pipedelay;
int gaindata[128][128][64];
int gapkey;
int hdelay;
string choice = "";
char run_number[16];

string DebugDAC;
string usemaroc2gainconstantsmb;
string comments;

int Year;
int Mon;
int Day;
int Hour;
int Min;
int Sec;


string mode;
string Time_Data_Path;

string database = "doublechooz_ov_far";
string DBhostname = "localhost";
string user = "dcndaq";
string password = "137316neutron";

void getpmtdata(int,int);
void SplitString(string s, vector<string> &v, string c);
void signal(string dir, string file, string baselines, int pmt_board, string homedir);
int scanFiles(string inputDirectory);


void initializeboard(string define_runnumber, int trigger_num, int pmtini, int pmtfin){
    int disk_num = 1;
    char DataPath[128] = "/OVDAQ/DATA/";
    string DataFolder;
    DataFolder = "/e/h.0/localdev/readout/data" + to_string(disk_num) + "/OVDAQ/DATA/";
    pipedelay = 20;
    
    if(!pmtini || !pmtfin){
        pmtini = 1;
        pmtfin = totalpmt;
    }
    if(!mode.compare("debug")){
        totalboard = pmtfin - pmtini + 1;
        if(totalpmt != totalboard){
            printf("totalpmt: %d\t totalboard: %d\n",totalpmt,totalboard);
            printf("Problem with initialization \n");
        }
    }
    
    int pmt1;
    int usbbase[20];
    int usbread;
    
    int i;
    for(i=0;usbhowmanyboards[0][i];i++){
        usbread = usbhowmanyboards[1][i];         //usbhowmanyboard[0] saves board number, usbhowmanyboard[1] saves usb number
        set_inhibit_usb(usbread,-1);
    }
    
    for(i=0;usbhowmanybox[0][i];i++){
        usbread = usbhowmanybox[1][i];
        set_inhibit_usb(usbread,-1);
    }
    
    sleep(2);
    
    //now the new folder and the new data path will be passed to the DAQ
    
    printf("DataPath=%s, Disk=%d\n",DataPath,disk_num);
    
    set_data_path(DataPath);
    //check_diskspace;
    set_data_disk(disk_num);  //setting $DataFolder
    
    
    //defining the run number
    int file_run;
    int max = 0;
    int files_run[20];
    
    time_t t1 = time(0);   //get time now
    struct tm * now = localtime( & t1 );
    
    int Sec = now->tm_sec;
    int Hour = now->tm_hour;
    int Min = now->tm_min;
    int Day = now->tm_mday;
    int Mon = now->tm_mon;
    int Year = now->tm_year;
    
    string date;
    
   // run_number = to_string(Year) + to_string(Mon) + to_string(Day) + to_string(Hour) + to_string(Min) + to_string(Sec);
    date = to_string(Year) + to_string(Mon) + to_string(Day);
    
    long runtmp = scanFiles("/e/h.0/localdev/readout/data1/OVDAQ/DATA");
  
    if(define_runnumber.compare("auto") == 0){
        sprintf(run_number,"%0.7d",runtmp);
    }

    set_run_number(run_number);
    
    printf("pmtini = %d, pmtfin = %d\n", pmtini, pmtfin);
    
    printf("Bseline data taking .");
    
    int cout = 0;   
 
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
        com_usb(usb_local, pmt_local, 74, 0b0100000);   // default gain
        com_usb(usb_local, pmt_local, 70, 0);           // load default
        dac_usb(usb_local, pmt_local, 1000);            // threshold value
        com_usb(usb_local, pmt_local, 71, 0);           // rst_g to maroc
        com_usb(usb_local, pmt_local, 72, 0);           // write config was done twice
        com_usb(usb_local, pmt_local, 73, 0b00110);     // set up pme module
        com_usb(usb_local, pmt_local, 87, 0);           // no force trigger
        com_usb(usb_local, pmt_local, 75, 0b00010000);  // set up trigger mode for module
        com_usb(usb_local, pmt_local, 109, 0);          // vdd_fsb off
        com_usb(usb_local, pmt_local, 254, 0);          // enable trigger
        for ( int i = 0; i < 10 ; i++){
            com_usb(usb_local, pmt_local, 81, 0);       // avoid first packets
        }
        com_usb(usb_local, pmt_local, 255, 0);          // disable trigger
        usbbase[cout] = usb_local;
        cout++;
    }
    sleep(2);
    
    for(int j = 0; usbbase[j]; j++){
        usb_local = usbbase[j];
        if(structure[usb_local]==0){
            set_inhibit_usb(usb_local, -3);              // -3; created file structure
            sleep(0.5);
            set_inhibit_usb(usb_local, -2);              // -2; release inhibit for baseline
            // -1; inhibit writing data
            // 0; release inhibit for writing data
            structure[usb_local] = -2;
        }
    }
    
    for(pmt1 = pmtini; pmt1 <= pmtfin; pmt1++){
        usb_local = pmttousb[pmt1];
        pmt_local = pmttoboard[pmt1];
        usb_usb(usb_local, 0, 1);                         // turn auto token ON
        com_usb(usb_local, pmt_local, 254, 0);            // enable trigger
        for( int i = 1; i <= trigger_num; i++){
            com_usb(usb_local, pmt_local, 81, 0);         // test trigger
        }
        com_usb(usb_local, pmt_local, 255, 0);            // disable trigger
        sleep(0.01);                                      // give it some time
    }
    sleep(3);                                             // wait late packets
    
    for( int j = 0; usbbase[j]; j++){
        usb_local = usbbase[j];
        if(structure[usb_local] == -2){
            set_inhibit_usb(usb_local, -1);       // -2; release inhibit for baseline
            // -1; inhibit writing data
            // 0; release inhibit for writing data
            structure[usb_local] = -1;
            usblocal[j] = usb_local;
        }
    }
    
    time_t t2 = time(0);   //get time now
    struct tm * now1 = localtime( & t2 );
    
    int newSec = now1->tm_sec;
    int newHour = now1->tm_hour;
    int newMin = now1->tm_min;
    int newDay = now1->tm_mday;
    int newMon = now1->tm_mon;
    int newYear = now1->tm_year;
    
    int elapsed_time;
    elapsed_time = (newDay - Day)*24*3600 + (newHour - Hour)*3600 + (newMin - Min)*60 + (newSec - Sec);
    
    for(int e = 0; e<=20; e++){
        sleep(0.1);
        printf(".");
    }
    
    printf(": %d sec\n",elapsed_time);
    
    sleep(3);
    
    
    //now let's initialize everything for the data taking
    
    printf("Initializing .\n");
    
    string dir2 = DataFolder + "/Run_" + run_number;
    string summary = "on";                                 // summary file default = off
    
    for (pmt1 = pmtini; pmt1 <= pmtfin; pmt1++){
        
        usb_local = pmttousb[pmt1];
        pmt_local = pmttoboard[pmt1];
        
        printf("Loading PMT: %d\n", pmt_local);
        
        getpmtdata(usb_local,pmt_local);
        
        usb_usb(usb_local, 0, 1);                          // auto token on
        com_usb(usb_local, pmt_local, 110, 1);             // turn off the three led on the PMT's board = 1
        com_usb(usb_local, pmt_local, 109, 1);             // vdd_fsb on
        com_usb(usb_local, pmt_local, 73, 0b00000);        // set up pmt module
        //        com_usb(usb_local, pmt_local, 255, 0);             // clear buffers
        //        com_usb(usb_local, pmt_local, 84, 255);            // buffer size limit
        com_usb(usb_local, pmt_local, 74, 0b0100000);      // default gain
        com_usb(usb_local, pmt_local, 70, 0);              // load default
        dac_usb(usb_local, pmt_local, DACt);               // threshold value
        
        com_usb(usb_local, pmt_local, 67, 0b000010);       // statea
        com_usb(usb_local, pmt_local, 68, 0b000000);       // stateb
        com_usb(usb_local, pmt_local, 69, 0b000000);       // statec
        
        com_usb(usb_local, pmt_local, 71, 0);              // rst_g to maroc
        com_usb(usb_local, pmt_local, 72, 0);              // write config
        com_usb(usb_local, pmt_local, 72, 0);              // write config
        //        com_usb(usb_local, pmt_local, 73, 0b01010);        // gate
        //        com_usb(usb_local, pmt_local, 75, 0b00010000);     // trigger mode( gateonoff = 0b01011, trigger_mode = 0b01010000)
        
        com_usb(usb_local, pmt_local, 80, 5);              // hold delay is variable. Has been fixed to 5 here
        /////////
        //        dac_usb(usb_local, pmt_local, DACt);               // threshold value
        com_usb(usb_local, pmt_local, 73, gateonoff);      // gate
        com_usb(usb_local, pmt_local, 85, 0b0000);         // set up pipe delay
        
        com_usb(usb_local, pmt_local, 75, trigger_mode);   // trigger mode (gateonoff = 0b01011; trigger_mode = 0b01010000);
        com_usb(usb_local, pmt_local, 86, 0);              // edge strip mode
        com_usb(usb_local, pmt_local, 87, 0b01);           // force readout -> 01: 1msec, 10: 16msec, 11: 256msec
       
        if(usemaroc2gainconstantsmb.compare("no")==0){
            com_usb(usb_local, pmt_local, 74, 0b0100000);  // default gain
        }
        else if(usemaroc2gainconstantsmb.compare("yes")==0 && mysqlload == 0){
            printf("Error cannot load mysql gain constants from MYSQL\n");
            com_usb(usb_local, pmt_local, 74, 0b0100000);  // default gain
        }
        else if(usemaroc2gainconstantsmb.compare("yes")==0 && mysqlload == 1){
            for(int index = 1 ; index < 65; index++){
                com_usb(usb_local, pmt_local, index - 1, 2*gain[index]);  // applying gain constants from MySQL
            }
        }
        
        sleep(0.5);
        
        com_usb(usb_local, pmt_local, 109, 0);              // vdd_fsb off
        
        com_usb(usb_local, pmt_local, 255, 0);
        
        //        com_usb(usb_local, pmt_local, 254, 1);              // trigger on
        
        
        // here create a first summary file or append to an existing one
        
        if(summary.compare("on")==0){
            
            string summaryfile = dir2 + "/summary.txt";
            string summaryfile1 = dir2 + "/summary_new.txt";
            
            ifstream efile(summaryfile.c_str());
            
            if(efile.good()){
                
                FILE * pFile2;
                pFile2 = fopen(summaryfile1.c_str(),"w");
                if(pFile2 == NULL){
                    printf("Can not open summaryfile1\n");
                    fclose(pFile2);
                }
                fprintf(pFile2,"\n");
                fprintf(pFile2,"%d \t %d \t %d \t %d \t %d \t %s \t %d \t %d \t %d \t %d \t %s \t %s \t", pmt_local, pmtserialnumber, pmt_local,HVsetting, DACt, gate.c_str(), trigger_mode, pipedelay, filelength, run_length, usemaroc2gainconstantsmb.c_str(), comments.c_str());
                if(usemaroc2gainconstantsmb.compare("yes")==0){
                    for(int index = 1; index < 65; index++){
                        fprintf(pFile2, "%d \t", gain[index]);
                    }
                }
                efile.close();
                fclose(pFile2);
                string move = "mv" + summaryfile1 + summaryfile;
                system(move.c_str());
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
                fprintf(pFile1,"%d \t %d \t %d \t %d \t %d \t %s \t %d \t %d \t %d \t %d \t %s \t %s \t", pmt_local, pmtserialnumber, pmt_local,HVsetting, DACt, gate.c_str(), trigger_mode, pipedelay, filelength, run_length, usemaroc2gainconstantsmb.c_str(), comments.c_str());
                if(usemaroc2gainconstantsmb.compare("yes")==0){
                    for(int index = 1; index < 65; index++){
                        fprintf(pFile1, "%d \t", gain[index]);
                    }
                }
                fclose(pFile1);
            }
            
        } // end for summary on or off
        
        
    } //end loop over pmt
    
    sleep(0.5);
    printf("finished initializing \n");
    
    
}

///////////////////////////////////////////////////////////////////////////////
void loadconfig(string mode_local, int usb_board, int pmt_board, int triggerbox){
    if(mode_local == "")
        mode = "mysql";
    else
        mode = mode_local;
    
    totalbox = 0;
   
    //code for retrieving info from mysql database
    
    if(mode.compare("mysql") == 0){
        totalpmt = 0;
        totalbox = 0;
        usb_box = 0;
        
        try
        {
            MYSQL mysql;
            MYSQL_RES *result;
            MYSQL_ROW sql_row;
            int res;
            
            
            mysql_init(&mysql);          //initial mysql structure
            
            if(mysql_real_connect(&mysql, DBhostname.c_str(), user.c_str(), password.c_str(), database.c_str(),0,NULL,0))
            {
                printf("Connected to MySQL database\n");
                string sqlstr = "SELECT USB_serial, PMT_Serial, board_number, HV, DAC_threshold, use_maroc2gain, gate, hdelay, trigger_mode, gain1, gain2, gain3, gain4, gain5, gain6, gain7, gain8, gain9, gain10, gain11, gain12, gain13, gain14, gain15, gain16, gain17, gain18, gain19, gain20, gain21, gain22, gain23, gain24, gain25, gain26, gain27, gain28, gain29, gain30, gain31, gain32, gain33, gain34, gain35, gain36, gain37, gain38, gain39, gain40, gain41, gain42, gain43, gain44, gain45, gain46, gain47, gain48, gain49, gain50, gain51, gain52, gain53, gain54, gain55, gain56, gain57, gain58, gain59, gain60, gain61, gain62, gain63, gain64 FROM online";
                res = mysql_query(&mysql,sqlstr.c_str());
                if(!res)
                {
                    result = mysql_store_result(&mysql);
                    if(result)
                    {
                        int num_rows;
                        num_rows = mysql_affected_rows(&mysql);
                        while(sql_row = mysql_fetch_row(result))
                        {
                            int usbtemp = atoi(sql_row[0]);
                            int pmttemp = atoi(sql_row[2]);
                            pmtdata[usbtemp][pmttemp][0] = sql_row[1];     //pmt serial number
                            pmtdata[usbtemp][pmttemp][1] = sql_row[2];     //pmt board number (not unique)
                            pmtdata[usbtemp][pmttemp][2] = sql_row[3];     //HV setting
                            pmtdata[usbtemp][pmttemp][3] = sql_row[4];     //DAC threshold
                            pmtdata[usbtemp][pmttemp][4] = sql_row[5];     //Use Maroc2 gain constants
                            pmtdata[usbtemp][pmttemp][5] = sql_row[6];     //op73
                            pmtdata[usbtemp][pmttemp][6] = sql_row[7];     //op85
                            pmtdata[usbtemp][pmttemp][7] = sql_row[8];     //op87
                            pmtdata[usbtemp][pmttemp][8] = sql_row[9];     //op75
                            pmtdata[usbtemp][pmttemp][9] = "mysql mode";   // comments
                            pmtdata[usbtemp][pmttemp][10] = sql_row[10];   //module number
                            if(pmtdata[usbtemp][pmttemp][2] == "-999"){
                                for(int index = 0; index<64; index++)
                                {
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
                                for(int index = 0; index<64; index++)
                                {
                                    gaindata[usbtemp][pmttemp][index] = atoi(sql_row[11+index]);
                                }
                                totalpmt++;
                                pmttousb[totalpmt] = usbtemp;
                                pmttoboard[totalpmt] = pmttemp;
                                structure[usbtemp] = 0;
                                usbhowmanyboards[1][usbhowmanyboardscount] = usbtemp;
                                usbhowmanyboards[0][usbhowmanyboardscount]++;
                                usbhowmanyboardscount++;
                            }
                        }
                    }
                }
                else
                {
                printf("query sql failed!\n");
                }
            }
            else
            {
                printf("Connection failed!\n");
            }
            if(result != NULL)
                mysql_free_result(result);
            mysql_close(&mysql);
        }
        catch(...){}
        
    }
    else if(mode.compare("debug") == 0){
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
    
    while ( p_dirent = readdir(p_dir))
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

///////////////////////////////////////////////////////////////////////////////

void test_system_time(int usb, int pmt){
    int disk = 1;
    char DataPath[128] = "CRTTime/DATA/";
    string DataFolder = "../data" + to_string(disk) + "/" + DataPath;
    
    set_data_path(DataPath);
    //check_diskspace;
 
    set_data_disk(disk);
    
    loadconfig("debug", usb, pmt,0);
    
    int usbread;
    int usb_local;
    int box_local;
    int pmt_local;
    
    // defining the run number for  this data taking
    
    int file_run;
    int max = 0;
    int files_run[20];
    
    time_t t1 = time(0);   //get time now
    struct tm * now = localtime( & t1 );
    
    int Sec = now->tm_sec;
    int Hour = now->tm_hour;
    int Min = now->tm_min;
    int Day = now->tm_mday;
    int Mon = now->tm_mon;
    int Year = now->tm_year;
    
    char run1[128];
    string date;
    
    //run1 = to_string(2018) + to_string(Mon) + to_string(Day) + to_string(Hour) + to_string(Min) + to_string(Sec);
    date = to_string(Year) + to_string(Mon) + to_string(Day);
    
    long runtmp = scanFiles("/e/h.0/localdev/readout/data1/CRTTime/DATA");
    sprintf(run1,"%0.7d",runtmp);
    set_run_number(run1);                     // define the run_number and pass it to the DAQ
    
    Time_Data_Path = DataFolder + "/Run" + run1 + "/binary";
    
    printf("Initializing time check for the CRT .");
   
    for (int pmt1 = 1; pmt1<=totalpmt; pmt1++){
        
        if(!(usb_local = pmttousb[pmt1] || !(pmt_local = pmttoboard[pmt1]))){
            printf("usb_local or pmt_local not defined.\n");
        }
   
        usb_local = pmttousb[pmt1];
        pmt_local = pmttoboard[pmt1];
        getpmtdata(usb_local, pmt_local);
        usb_usb(usb_local, 0 ,1);                       // turn auto token ON
        com_usb(usb_local, pmt_local, 110, 0);          // turn off lef off/on the PMT's board = 1/0
        com_usb(usb_local, pmt_local, 109, 1);          // vdd_fsb on
        com_usb(usb_local, pmt_local, 73, 0b00000);     // set up pmt module
        com_usb(usb_local, pmt_local, 255, 0);          // clear buffers
        com_usb(usb_local, pmt_local, 84, 255);         // buffer size limit
        com_usb(usb_local, pmt_local, 74, 0b0100000);   // default gain
        com_usb(usb_local, pmt_local, 70, 0);           // load default
        com_usb(usb_local, pmt_local, 67, 0b000010);       // statea
        com_usb(usb_local, pmt_local, 68, 0b000000);       // stateb
        com_usb(usb_local, pmt_local, 69, 0b000000);       // statec
        com_usb(usb_local, pmt_local, 71, 0);              // rst_g to maroc
        com_usb(usb_local, pmt_local, 72, 0);              // write config
        com_usb(usb_local, pmt_local, 80, 5);              // hold delay is variable. Has been fixed to 5 here
        dac_usb(usb_local, pmt_local, 1000);            // threshold value
        com_usb(usb_local, pmt_local, 73, 0b01110);     // gate alladc gate on -> 0b11110 ;   alladc gate off -> 0b01110
        com_usb(usb_local, pmt_local, 85, 5);           // set up pipe delay
        com_usb(usb_local, pmt_local, 75, 0b01100000);  // trigger mode
        com_usb(usb_local, pmt_local, 86, 0);           // edge strip mode is off
        com_usb(usb_local, pmt_local, 87, 01);          // force readout -> 01: 1msec, 10: 16msec, 11: 256msec
        
        com_usb(usb_local, pmt_local, 74, 0b0100000);   //default gain for the 64 channels
        
        sleep(0.01);
        
        printf(".");
        
        com_usb(usb_local, pmt_local, 109, 0);          // vdd_fsb off
        
        com_usb(usb_local, pmt_local, 254, 1);          // enbale trigger
        
        sleep(0.01);
        
    } //end loop over pmt
    
    printf("\n");
    
    //initializing data taking for the trigger boxes
    
    usb_usb(usb_box, 0, 1);                             // turn on auto token
    
    for( int i = 1; i <= totalbox; i++){
        if(!(usb_local = pmttousb[i] || !(pmt_local = pmttoboard[i]))){
            printf("usb_local or pmt_local not defined.\n");
        }
        com_usb(usb_local, box_local, 252, 0b1001);      // control flags=
        // (flag == 1### --> test)
        // (flag == 0000 fan in/out)
        // (flag == 0001 X&Y trigger)
        // (flag == 0010 edge strip mode)
        
        com_usb(usb_local, box_local, 254, 0);           // enable trigger
        sleep(0.5);
    }
    
    int i;
    for(i=0; usbhowmanyboards[0][i];i++){
        usbread = usbhowmanyboards[1][i];         //usbhowmanyboards[0] saves board number, usbhowmanyboard[1] saves usb number
        set_inhibit_usb(usbread,-3);                      // -3; create file structure
    }
    
    for(i=0;usbhowmanybox[0][i];i++){
        usbread = usbhowmanybox[1][i];
        set_inhibit_usb(usbread,-3);                      // -3; create file structure
    }
    
    
    sleep(1);
    
    printf("start data taking ....\n");
    
    for(i=0; usbhowmanyboards[0][i];i++){
        usbread = usbhowmanyboards[1][i];         //usbhowmanyboards[0] saves board number, usbhowmanyboard[1] saves usb number
        set_inhibit_usb(usbread,0);                      // start taking data
    }
    
    for(i=0;usbhowmanybox[0][i];i++){
        usbread = usbhowmanybox[1][i];
        set_inhibit_usb(usbread,0);                      // start taking data
    }
    
    
    //data taking in progress
}

bool JudgeNum(string str, int iTmp){
    bool bNum = true;
    string::size_type szSize = str.size();
    for(int i=0; i<szSize; ++i){
        char ch = str.at(i);
        if((ch < '0') || (ch > '9')){
            bNum = false;
            break;
        }
    }
    if(bNum){
        istringstream iss(str);
        iss >> iTmp;
    }
    return bNum;
}


void check_system_time()
{
    int filesize;
    int readfilesize[2][20];                           //readfilesize[0] save filesize, readfilesize[1] save usb number
    int usbread;
    
    int i = 0;
    int j = 0;
    int diff;
    int a,b,c;
    int iTmp = 0;
    
    string cmd = "ls -ltr --time-sytle=long-iso " + Time_Data_Path + "|";
    ifstream myfile(cmd.c_str());
    if(myfile.is_open())
    {
        string line;
        vector<int> iVec;
        while(getline(myfile,line))
        {
            if(JudgeNum(line, iTmp))
            {
                iVec.push_back(iTmp);
            }
        }
        a = iVec.at(0);
        b = iVec.at(1);
        c = iVec.at(2);
        cout << a << " " << b << "_" << c;
        filesize = a;
        readfilesize[0][j] = filesize;
        readfilesize[1][j] = c;
        if(readfilesize[1][j] != usb_box)
        {
            readfilesize[0][j] += filesize;
            i++;
        }
        j++;
    }
    
    myfile.close();
    
    
    int maxdim = 0;
    int mindim = 1000000000;
    
    for (i=0; readfilesize[1][i] && usbhowmanyboards[1][i]; i++)
    {
        if(readfilesize[1][i] != usbhowmanyboards[1][i])
        {
            printf("Problem in the number of USB found .%d. in path %s expected .%d. \n", readfilesize[1][i], Time_Data_Path.c_str(), usbhowmanyboards[1][i]);
        }
    }
    
    for(i=0; readfilesize[1][i]; i++)
    {
        usbread = readfilesize[1][i];
        if(usbread != usb_box)
        {
            readfilesize[0][i] /= usbhowmanyboards[0][i];           //this normalize the file dimension by the pmt boards
            if(readfilesize[0][i] > maxdim)
            {
                maxdim = readfilesize[0][i];
            }
            if(readfilesize[0][i] < mindim)
            {
                mindim = readfilesize[0][i];
            }
        }
    }
    
    diff = maxdim - mindim;
    
    if((maxdim == 0) || (mindim == 0))
    {
        printf("Got a problem Number of packet is ZEOR!\n");
        //    gaibu_msg(MERROR, "Got a problem Number of packet is ZEOR!");
    }
    
    if(diff > 300)
    {
        printf("We got a problem with the timing info DeltaDim(Max-Min) between files=%d \n", diff);
        //     syslog('LOG_ERR',"We got a problem with the timing info DeltaDim(Max-Min) between files=%d \n", diff);
        //     sendmail("OVRC","ERROR","We got a problem with the timing info DeltaDim(Max-Min) between files=%d \n", diff);
    }
}



////////////////////////////////////////////////////////////////////////
void starttakedata(int pmtini, int pmtfin, int boxini, int boxfin)
{
    if(!pmtini || !pmtfin)                          // both not defined
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
    
    if (!boxini || !boxfin)                           // both not defined
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
    for(pmt1 = pmtini; pmt1<=pmtfin; pmt1++)
    {
        if(!pmttousb[pmt1] || !pmttoboard[pmt1])
        {
            printf("usb_local or pmt_local not defined.\n");
        }
        usb_local = pmttousb[pmt1];
        pmt_local = pmttoboard[pmt1];
        getpmtdata(usb_local, pmt_local);
        printf("usb_local=%d and pmt_local=%d.\n",usb_local,pmt_local);
        com_usb(usb_local, pmt_local, 254, 0);
        
        sleep(0.05);
    }
    
    com_usb(33,63,254,0);
    
    sleep(2);
    
    int box1;
    for(box1 = boxini; box1<=boxfin; box1++)
    {
        if(!boxtousb[box1] || !boxtoboard[box1])
        {
            printf("usb_local or pmt_local not defined.\n");
        }
    }
    
    for(int i = 0; usblocal[i]!=0; i++){
        usb_local = usblocal[i];
        if(structure[usb_local] == -1)                // only trigger box!!
        {
            set_inhibit_usb(usb_local, 0);            // -2; release inhibit for baseline
            // -1; inhibit writing data
            // 0; release inhibit for writing data
            structure[usb_local] = 1;
            printf("File open for writing \n");
        }
    }
    
    //usb(usb_local,0,1);
    usb_usb(usb_local,0,1);
    
    sleep(0.5);
    
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

void stoptakedata( int pmtini, int pmtfin, int boxini, int boxfin)
{
    if(!pmtini || !pmtfin)                           // both not defined
    {
        pmtini = 1;
        pmtfin = totalpmt;
    }
    
    if(mode.compare("debug")==0)
    {
        totalboard = pmtfin - pmtini + 1;
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
    
    
    if(!boxini || !boxfin)                            // both not defined
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
    int newMon = now1->tm_mon;
    int newYear = now1->tm_year;
    
    int elapsed_time;
    elapsed_time = (newDay - Day)*24*3600 + (newHour - Hour)*3600 + (newMin - Min)*60 + (newSec - Sec);
    
    printf("%d sec...", elapsed_time);
    
    int pmt1;
    for(pmt1 = pmtini; pmt1<=pmtfin; pmt1++)
    {
        if(!pmttousb[pmt1] || !pmttoboard[pmt1])
        {
            printf("usb_local or pmt_local not defined.\n");
        }
        com_usb(usb_local, pmt_local, 255, 0);                     // disable trigger
        com_usb(usb_local, pmt_local, 73, 0b00000);
        com_usb(usb_local, 0, 255, 0);
        sleep(0.05);                                               // give it some time
    }
    
    sleep(1);
    
    printf("shutting down ");
    
    for(int m = 0;m<=10; m++)
    {
        printf(".");
        sleep(0.3);
    }
    printf("\n");
    
    for(int i = 0; usblocal[i]; i++)
    {
        usb_local = usblocal[i];
        if(structure[usb_local] == 1)
        {
            set_inhibit_usb(usb_local, -1);                       // -2; release inhibit for baseline
            // -1; inhibit writing data
            // 0; release inhibit for writing data
            structure[usb_local] = -1;
        }
    }
    
    sleep(1);
}




///////////////////////////////////////////////////////////////////////////

void getpmtdata(int usbboard, int pmtnumber)
{
    pmtserialnumber = atoi(pmtdata[usbboard][pmtnumber][0].c_str());
    boardnumber = atoi(pmtdata[usbboard][pmtnumber][1].c_str());
    HVsetting = atoi(pmtdata[usbboard][pmtnumber][2].c_str());
    DACt = atoi(pmtdata[usbboard][pmtnumber][3].c_str());
    usemaroc2gainconstantsmb = pmtdata[usbboard][pmtnumber][4];
    gate = pmtdata[usbboard][pmtnumber][5];
    pipedelay = atoi(pmtdata[usbboard][pmtnumber][6].c_str());
    //  force_trig = pmtdata[usbboard][pmtnumber][7];
    trigger_mode = atoi(pmtdata[usbboard][pmtnumber][7].c_str());
    comments = pmtdata[usbboard][pmtnumber][8];
    module = pmtdata[usbboard][pmtnumber][10];
    
    if(gate.compare("on") == 0)
    {
        gateonoff = 0b11010;
    }
    else if(gate.compare("off") == 0)
    {
        gateonoff = 0b01010;
        trigger_mode = 0b00010000;
    }
    else if(gate.compare("trigger") == 0)
    {
        gateonoff = 0b01011;
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
        gateonoff = 0b01010;
        trigger_mode = 0b01100000;
    }
    else if(gate.compare("allhit") == 0)
    {
        gateonoff = 0b01001;
    }
    
   // printf("pmt serial number = %d\n", pmtserialnumber);
   // printf("board number = %d\n", boardnumber);
   // printf("HV setting = %d\n", HVsetting);
   // printf("DAC threshold in use = %d\n", DACt);
   // printf("use of maroc2 gain constants = %s\n", usemaroc2gainconstantsmb.c_str());
   // printf("gate = %s\n", gate.c_str());
   // printf("gateonoff = %d\n", gateonoff);
   // printf("pipedelay = %d\n", pipedelay);
   // printf("trigger_mode = %d\n", trigger_mode);
   // printf("comments = %s\n", comments.c_str());
   // printf("module = %s\n", module.c_str());
    
    for(int index = 0; index<64; index++)
    {
        if(mysqlload)
        {
            gain[index+1] = gaindata[usbboard][pmtnumber][index];
        }
        else
        {
            gain[index+1] = 16;
        }
       // printf("%d\n",gain[index+1]);
    }
    
}


///////////////////////////////////////////////////////////////////
//void sendmail()

///////////////////////////////////////////////////////////////////

//void gaibu_msg()



///////////////////////////////////////////////////////////////////
/*
 void getFiles( string path, vector<string> &files)
 {
 long hFile = 0;
 struct _finddata_t fileinfo;
 string p;
 if((hFile = _findfirst(p.assign(path).append("\\*").c_str(),&fileinfo)) != -1)
 {
 do
 {
 if((fileinfo.attrib & _A_SUBDIR))
 {
 if(strcmp(fileinfo.name,".") != 0 && strcmp(fileinfo.name, "..") != 0)
 getFiles( p.assign(path).append("\\").append(fileinfo.name), files);
 }
 else
 {
 files.push_back(p.assign(path).append("\\").append(fileinfo.name));
 }
 }while(_findnext(hFile, &fileinfo) == 0);
 _findclose(hFile);
 }
 }
 
 
 
 void check_rate(string folderlocation, int tot_pmt, int tot_usb, int boxusb, int switchonoff)
 {
 string location_baseline;
 string files[20];
 string tempfile;
 int hnumber = 0;
 int gnumber = 0;
 int hit_pmt[100][2];                  // hit_pmt[][0] for keys, hit_pmt[][1] for value
 int cnt_pmt[100];
 int gap_pmt[100][3];                  // gap_pmt[][0] for keys, gap_pmt[][1] for gapkey, gap_pmt[][2] for value
 int diff, tdiff;
 int keymax = 0;
 int keymin = 1000000;
 int key;
 int usbnum;
 int found_pmt_in_time = 0;
 int module_number;
 int filenum = 0;
 vector<string> tempfiles;
 
 if(!switchonoff)
 {
 //get all the files under the directory
 getFiles(folderlocation.c_str(), tempfiles);
 
 int size = tempfiles.size();
 for(int i = 0; i < size; i++)
 {
 cout << tempfiles[i].c_str() << endl;
 files[i] = tempfiles[i];
 filenum++;
 }
 }
 else
 {
 //get one file under the directory
 getFiles(folderlocation.c_str(), tempfiles);
 cout << tempfiles[0].c_str() << endl;
 location_baseline = tempfiles[0];
 tot_usb = tot_usb - 1;    //box do not get baseline file
 files[0] = location_baseline;
 filenum++;
 }
 
 string cmd = "decode";
 if(filenum < tot_usb - 1)
 printf("No files found \n");
 for(int i = 0; files[i]!=""; i++)
 {
 tempfile = files[i];
 regex repPattern("(_(/d+)$");
 smatch match;
 if(regex_match(tempfile,match,repPattern))
 {
 usbnum = atoi(match.str(1).c_str());
 }
 if(usbnum == boxusb)
 continue;
 string cmd1 = cmd + tempfile;
 ifstream IN(cmd1.c_str());
 string s;
 if(!IN.good()){
 printf("Can not open file\n");
 IN.close();
 }
 else
 {
 getline(IN,s);
 while(getline(IN,s))
 {
 //   if(isspace(s))
 //       continue;
 vector<string> line;
 SplitString(s,line,",");
 if(line[0].compare("p") == 0)
 {
 for(int j=0;line[j] != "";j++)
 line[j] = line[j+1];
 int tmp = atoi(line[0].c_str());
 int mod = (tmp >> 8) & 127;
 mod += 100 * usbnum;
 hit_pmt[hnumber][0] = mod;
 hit_pmt[hnumber][1]++;
 hnumber++;
 if(!switchonoff)
 {
 for(int j=0;line[j] != "";j++)
 line[j] = line[j+1];
 int tim = (atoi(line[0].c_str()) >> 8) & 255;    // This assumes 256ms internal clocked trigger
 int tdiff;
 if(cnt_pmt(mod))
 {
 tdiff = tim - cnt_pmt[mod];
 if(tdiff > 1)
 {
 gap_pmt[gnumber][0] = mod;
 gap_pmt[gnumber][1] = tdiff;
 gap_pmt[gnumber][2]++;
 gnumber++;
 }
 }
 cnt_pmt[mod] = tim;
 }
 }
 }
 IN.close();
 }
 }
 
 // check which is the missing pmt
 for(int i = 1; i<= tot_pmt;i++)
 {
 found_pmt_in_time = 0;
 for(int j = 0; hit_pmt[j][0]; j++)
 {
 module_number = pmttoboard[i] + 100*pmttousb[i];
 if(hit_pmt[j][0] == module_number)
 {
 found_pmt_in_time = 1;
 }
 }
 if(found_pmt_in_time == 0)
 {
 if(!switchonoff)
 {
 cout << "Error: PMT Board " << pmttoboard[i] << " is missing from the timing check \n";
 }
 else
 {
 cout << "Error: PMT Board " << pmttoboard[i] << " is missing from the baseline file \n";
 string cmd = "Error: PMT Board" + to_string(pmttoboard[i]) + " is missing from the baseline file \n";
 string cmd1 = "PMT Board " + to_string(pmttoboard[i]) + " is missing from the baseline file";
 // gaibu_msg(MERROR,cmd);
 // sendmail("OVRC","CRITICAL",cmd1);
 }
 }
 }
 
 int tmptotpmts = hit_pmt[0][0];
 if(tmptotpmts != tot_pmt)
 {
 cout << "Error: Number of pmts found: " << tmptotpmts << "\tNumber of pmts expected in MySQL: " << tot_pmt << "\n";
 string cmd = "Error: Number of pmts found:" + to_string(tmptotpmts) + ", Number of pmts expected in MySQL:" + to_string(tot_pmt);
 string cmd1 = "Number of pmts found  " + to_string(tmptotpmts) + " expected " + to_string(tot_pmt);
 //  gaibu_msg(MERROR,cmd);
 //  sendmail("OVRC","ERROR",cmd1);
 }
 
 //now find the min and max value for the hit
 for(int j = 0; hit_pmt[j][0]; j++)
 {
 key = hit_pmt[j][0];
 if(hit_pmt[j][1] > keymax)
 {
 keymax = hit_pmt[j][1];
 }
 if(hit_pmt[j][1] < keymin)
 {
 keymin = hit_pmt[j][1];
 }
 }
 
 diff = keymax - keymin;
 
 if(keymax == 0 || keymin == 0)
 {
 cout << "Got a Problem Number of packet is ZEOR\n";
 }
 
 if(diff > 5)
 {
 if(!switchonoff)
 {
 cout << "Error the differrence between the trigger packet is " << diff << "\n";
 cmd ="Error the differrence between the trigger packet is " + to_string(diff);
 //   gaibu_msg(MNOTICE,cmd);
 }
 else
 {
 cmd ="Error the differrence between baseline packet is " + to_string(diff);
 cout << cmd << "\n";
 //    gaibu_msg(MNOTICE,cmd);
 }
 }
 
 if(!switchonoff)
 {
 for(int i = 0; hit_pmt[i][0]; i++)
 {
 key = hit_pmt[i][0];
 int tot_missed = 0;
 for(int j = 0; gap_pmt[j][0];j++)
 {
 gapkey = gap_pmt[j][1];
 tot_missed += gapkey * gap_pmt[j][2];
 }
 if(tot_missed > 1 && hit_pmt[i][1])
 {
 cout << "PMT " << key << " missing fraction of triggers: " << tot_missed << "/" << hit_pmt[i][1] << "\n";
 cmd = "PMT " + to_string(key) + " missing fraction of triggers: " + to_string(tot_missed) + "/" + to_string(hit_pmt[i][1]);
 //    gaibu_msg(MNOTICE,cmd);
 }
 }
 }
 
 if(switchonoff && keymin < (switchonoff - 5))
 {
 cmd = "Error minimum number of baseline packets found is " + to_string(keymin);
 cout << cmd << "\n";
 //  gaibu_msg(MERROR,cmd);
 }
 }
 
 */

////////////////////////////////////////////////////////////////////

void setpmtdata(int usbboard, int pmtnumber)
{
pmtserialinput:
    printf("Enter PMT Serial Number (for example:4662);");
    cin >> pmtserialnumber;                       // read number input from cin
    if(!pmtserialnumber)
        goto pmtserialinput;
    
    boardnumber = pmtserialnumber;
    
    if(!pmtserialnumber)
    {
        HVsetting = 0;
        usemaroc2gainconstantsmb = "no";
    }
    else
    {
    hvsettinginput:
        printf("Enter the High Voltage setting: ");
        cin >> HVsetting;
        if(!HVsetting)
            goto hvsettinginput;
        
    marocuse:
        printf("Use maroc2 gain constants to take data? (yes or no) ");
        cin >> usemaroc2gainconstantsmb;
        if(usemaroc2gainconstantsmb == "")
            goto marocuse;
    }
    
DACinput:
    printf("Enter the DAC threshold: ");
    cin >> DACt;
    if(!DACt)
        goto DACinput;
    
gate:
    printf("Which gate (op73)? (on or off or alladc or alladcoff) ");
    cin >> gate;
    if(gate == "")
        goto gate;
    
    trigger_mode = 1;
    
    /*
     trigger:
     printf("Which trigger (op75)? (0,1,2) ");
     cin >> trigger_mode;
     if(!trigger_mode)
     goto trigger;
     */
    
hdelay:
    printf("Which hdelay? (push enter will set the default value of 5) ");
    cin >> hdelay;
    if(!hdelay)
        hdelay = 5;
    printf("hdelay: %d \n", hdelay);
    
    if(choice.compare("manual") == 0)
    {
        printf("Any comments \n");
        cin >> comments;
    }
    
    pmtdata[usbboard][pmtnumber][0] = to_string(pmtserialnumber);
    pmtdata[usbboard][pmtnumber][1] = to_string(boardnumber);
    pmtdata[usbboard][pmtnumber][2] = to_string(HVsetting);
    pmtdata[usbboard][pmtnumber][3] = to_string(DACt);
    pmtdata[usbboard][pmtnumber][4] = usemaroc2gainconstantsmb;
    pmtdata[usbboard][pmtnumber][5] = gate;
    pmtdata[usbboard][pmtnumber][6] = to_string(hdelay);
    pmtdata[usbboard][pmtnumber][7] = to_string(trigger_mode);
    pmtdata[usbboard][pmtnumber][8] = comments;
    
    printf("\n");
    printf("\n");
}


//////////////////////////////////////////////////////////////////

void loadpmtdata_auto(int usb, int pmt, int pmtserialnumber, string mysql_table)
{
    totalbox = 0;
    totalpmt = 0;
    usb_box = 0;
    
    if(pmt == 0)
    {
        pmtdata[usb][pmt][0] = "none";
        pmtdata[usb][pmt][1] = to_string(pmt);
        pmtdata[usb][pmt][2] = to_string(0);
        pmtdata[usb][pmt][3] = to_string(1000);
        pmtdata[usb][pmt][4] = "no";
        pmtdata[usb][pmt][5] = "testhit";
        pmtdata[usb][pmt][6] = to_string(5);
        pmtdata[usb][pmt][7] = to_string(0b00000110);
        pmtdata[usb][pmt][8] = "test with default configuration";
        
        mode = "debug";
        
        goto goend;
    }
    
    if(pmtserialnumber)
    {
        int foundcount = 0;
        string choicefirst = "";
        string choicesecond;
        int DACt1;
        string gate1;
        string maroc2;
        int board1;
        
        mode = "mysql";
        
        try
        {
            MYSQL mysql;
            MYSQL_RES *result;
            MYSQL_ROW sql_row;
            int res;
            
            
            mysql_init(&mysql);          //initial mysql structure
            
            if(mysql_real_connect(&mysql, DBhostname.c_str(), user.c_str(), password.c_str(), database.c_str(),0,NULL,0))
            {
                printf("Connected to MySQL database\n");
                string sqlstr = "SELECT USB_serial, PMT_Serial, board_number, HV, DAC_threshold, use_maroc2gain, gate, hdelay, trigger_mode, gain1, gain2, gain3, gain4, gain5, gain6, gain7, gain8, gain9, gain10, gain11, gain12, gain13, gain14, gain15, gain16, gain17, gain18, gain19, gain20, gain21, gain22, gain23, gain24, gain25, gain26, gain27, gain28, gain29, gain30, gain31, gain32, gain33, gain34, gain35, gain36, gain37, gain38, gain39, gain40, gain41, gain42, gain43, gain44, gain45, gain46, gain47, gain48, gain49, gain50, gain51, gain52, gain53, gain54, gain55, gain56, gain57, gain58, gain59, gain60, gain61, gain62, gain63, gain64 FROM" + mysql_table;
                res = mysql_query(&mysql,sqlstr.c_str());
                if(!res)
                {
                    result = mysql_store_result(&mysql);
                    if(result)
                    {
                        while(sql_row = mysql_fetch_row(result))
                        {
                            if(atoi(sql_row[1]) == pmtserialnumber)     //check for matching pmt_serial number and board_address
                            {
                                pmtdata[usb][pmt][0] = sql_row[1];
                                pmtdata[usb][pmt][1] = sql_row[2];
                                pmtdata[usb][pmt][2] = sql_row[3];
                                pmtdata[usb][pmt][3] = sql_row[4];
                                pmtdata[usb][pmt][4] = sql_row[5];
                                pmtdata[usb][pmt][5] = sql_row[6];
                                pmtdata[usb][pmt][6] = sql_row[7];
                                pmtdata[usb][pmt][7] = sql_row[8];
                                
                                for(int index = 0; index<64; index++)
                                {
                                    gaindata[usb][pmt][index] = atoi(sql_row[9+index]);
                                }
                                pmttousb[pmt] = usb;
                                pmttoboard[pmt] = pmt;
                                structure[usb] = 0;
                                
                                printf("Found info for: PMT_serial = %s, board_number = %d \n\n",sql_row[1],pmt);
                                printf("Please check that your setup has the following setting:\n");
                                printf("USB address = %d\n", usb);
                                printf("PMT serial number=%s\n",pmtdata[usb][pmt][0].c_str());
                                printf("Board number=%s\n",pmtdata[usb][pmt][1].c_str());
                                printf("HV setting=%s\n",pmtdata[usb][pmt][2].c_str());
                                printf("DAC threshold = %s\n",pmtdata[usb][pmt][3].c_str());
                                printf("Use of maroc2 gain constants = %s\n",pmtdata[usb][pmt][4].c_str());
                                printf("Gate = %s\n",pmtdata[usb][pmt][5].c_str());
                                printf("hdelay = %s\n",pmtdata[usb][pmt][6].c_str());
                                printf("trigger_mode = %s\n",pmtdata[usb][pmt][7].c_str());
                                DACt1 = atoi(pmtdata[usb][pmt][3].c_str());
                                gate1 = pmtdata[usb][pmt][5];
                                maroc2 = pmtdata[usb][pmt][4];
                                foundcount = 1;
                                mysqlload = 1;
                                
                                board1 = atoi(pmtdata[usb][pmt][1].c_str());
                            }
                            
                        }
                    }
                }
                else
                {
                    printf("query sql failed!\n");
                }
            }
            else
            {
                printf("Connection failed!\n");
            }
            if(result != NULL)
                mysql_free_result(result);
            mysql_close(&mysql);
        }
        catch(...){}
        
        
        if(foundcount == 0)
        {
            printf("\t\t !!!!!! Warning !!!!!!\n");
            printf("Info for your PMT <-> board not present in the mysql database or do not match.\n\n");
            printf("For PMT = %d the board address in the MySQL database is not %d\n",pmtserialnumber,pmt);
            mysqlload = 0;
        restartchoice:
            printf("If you want to quit just type quit and press enter, if you want to continue and put the run info manually type manual\n");
            cin >> choice;
            if((choice.compare("quit") != 0) && (choice.compare("manual") != 0))
                goto restartchoice;
            if(choice.compare("quit") == 0)
                return;
            if(choice.compare("manual") == 0)
            {
                setpmtdata(usb,pmt);
                if(pmtdata[usb][pmt][4].compare("yes") == 0)
                {
                    printf("I will use if founded the MARCOC2 gain constants stored in this machine\n");
                }
                goto goend;
            }
        }
        else
        {
            if(choicefirst.compare("change") == 0)
            {
                choice = "manual";
                setpmtdata(usb,pmt);
                if(pmtdata[usb][pmt][4].compare("yes") == 0)
                {
                    printf("Please type which gain constants you would like to use (mysql or local)\n");
                    cin >> choicesecond;
                    if(choicesecond.compare("local") == 0)
                    {
                        mysqlload = 0;
                    }
                }
            }
        }
        printf("\n\n");
    }
    else
    {
        printf("Loading setting as default for pmt board %d\n",pmt);
        
        pmtdata[usb][pmt][0] = "none";
        pmtdata[usb][pmt][1] = to_string(pmt);
        pmtdata[usb][pmt][2] = to_string(0);
        pmtdata[usb][pmt][3] = to_string(1000);
        pmtdata[usb][pmt][4] = "no";
        pmtdata[usb][pmt][5] = "off";
        pmtdata[usb][pmt][6] = to_string(5);
        pmtdata[usb][pmt][7] = to_string(0b00000110);
        pmtdata[usb][pmt][8] = "test with sync configuration";
        
    }
    
goend:
    if(!pmtserialnumber && (pmt != 1))
    {
        printf("Please enter comments for this run (max 50 characters), press enter when done\n");
        cin >> comments;
        pmtdata[usb][pmt][8] = comments;
    }
    pmtdata[usb][pmt][8] = comments;
    printf("\n");
    printf("\n");
    
}


//////////////////////////////////////////////////////////////////////

void loadpmtdata(int usb, int pmt, int SWITCH, string gate_override)
{
    if(!SWITCH)
        mode = "mysql";
    else
        mode = "debug";
    
    if(!SWITCH)
    {
    pmtserialnumber:
        printf("Enter pmt serial number without PA (example 4673) for board %d:\n",pmt);
        cin >> pmtserialnumber;
        if(!pmtserialnumber)
        {
            goto pmtserialnumber;
        }
        printf("Re-enter pmt serial number without PA (example 4673) for board %d:\n",pmt);
        int tempserial;
        cin >> tempserial;
        if(tempserial != pmtserialnumber)
        {
            printf("Serial numbers did not match. Please try again.\n");
            goto pmtserialnumber;
        }
        
        //code for retrieving info from mysql database
        totalbox = 0;
        totalpmt = 0;
        usb_box = 0;
        
        int foundcount = 0;
        string choicefirst = "";
        string choicesecond;
        int DACt1;
        string gate1;
        string maroc2;
        int board1;
        
        try
        {
            MYSQL mysql;
            MYSQL_RES *result;
            MYSQL_ROW sql_row;
            int res;
            
            
            mysql_init(&mysql);          //initial mysql structure
            
            if(mysql_real_connect(&mysql, DBhostname.c_str(), user.c_str(), password.c_str(), database.c_str(),0,NULL,0))
            {
                printf("Connected to MySQL database\n");
                string sqlstr = "SELECT USB_serial, PMT_Serial, board_number, HV, DAC_threshold, use_maroc2gain, gate, hdelay, trigger_mode, gain1, gain2, gain3, gain4, gain5, gain6, gain7, gain8, gain9, gain10, gain11, gain12, gain13, gain14, gain15, gain16, gain17, gain18, gain19, gain20, gain21, gain22, gain23, gain24, gain25, gain26, gain27, gain28, gain29, gain30, gain31, gain32, gain33, gain34, gain35, gain36, gain37, gain38, gain39, gain40, gain41, gain42, gain43, gain44, gain45, gain46, gain47, gain48, gain49, gain50, gain51, gain52, gain53, gain54, gain55, gain56, gain57, gain58, gain59, gain60, gain61, gain62, gain63, gain64 FROM onlinenew";
                res = mysql_query(&mysql,sqlstr.c_str());
                if(!res)
                {
                    result = mysql_store_result(&mysql);
                    if(result)
                    {
                        int num_rows;
                        num_rows = mysql_affected_rows(&mysql);
                        while(sql_row = mysql_fetch_row(result))
                        {
                            if(atoi(sql_row[1]) == pmtserialnumber)     //check for matching pmt_serial number and board_address
                            {
                                pmtdata[usb][pmt][0] = sql_row[1];
                                pmtdata[usb][pmt][1] = sql_row[2];
                                pmtdata[usb][pmt][2] = sql_row[3];
                                pmtdata[usb][pmt][3] = sql_row[4];
                                pmtdata[usb][pmt][4] = sql_row[5];
                                pmtdata[usb][pmt][5] = sql_row[6];
                                pmtdata[usb][pmt][6] = sql_row[7];
                                pmtdata[usb][pmt][7] = sql_row[8];
                    
                                totalpmt++;            
                                pmttousb[totalpmt] = usb;
                                pmttoboard[totalpmt] = pmt;
                                structure[usb] = 0;
                                usbhowmanyboards[1][usbhowmanyboardscount] = usb;
                                usbhowmanyboards[0][usbhowmanyboardscount]++;
                                usbhowmanyboardscount++;
                                
                                for(int index = 0; index<64; index++)
                                {
                                    gaindata[usb][pmt][index] = atoi(sql_row[9+index]);
                                }
                                printf("Found info for: PMT_serial = %s, board_number = %d \n\n",sql_row[1],pmt);
                                printf("Please check that your setup has the following setting:\n");
                                printf("USB address = %d\n", usb);
                                printf("PMT serial number=%s\n",pmtdata[usb][pmt][0].c_str());
                                printf("Board number=%s\n",pmtdata[usb][pmt][1].c_str());
                                printf("HV setting=%s\n",pmtdata[usb][pmt][2].c_str());
                                printf("DAC threshold = %s\n",pmtdata[usb][pmt][3].c_str());
                                printf("Use of maroc2 gain constants = %s\n",pmtdata[usb][pmt][4].c_str());
                                printf("Gate = %s\n",pmtdata[usb][pmt][5].c_str());
                                printf("hdelay = %s\n",pmtdata[usb][pmt][6].c_str());
                                printf("trigger_mode = %s\n",pmtdata[usb][pmt][7].c_str());
                                DACt1 = atoi(pmtdata[usb][pmt][3].c_str());
                                gate1 = pmtdata[usb][pmt][5];
                                maroc2 = pmtdata[usb][pmt][4];
                                foundcount = 1;
                                mysqlload = 1;
                                
                                printf("Please enter if you want to take data with that settings. If instead you would like to change any settings type change.\n");
                                char temp[100];
                                cin >> temp;
                                choicefirst = temp;
                                board1 = atoi(pmtdata[usb][pmt][1].c_str());
                            }
                            
                        }
                    }
                }
                else
                {
                    printf("query sql failed!\n");
                }
            }
            else
            {
                printf("Connection failed!\n");
            }
            if(result != NULL)
                mysql_free_result(result);
            mysql_close(&mysql);
        }
        catch(...){}
        
        
        
        if(foundcount == 0)
        {
            printf("\t\t !!!!!! Warning !!!!!!\n");
            printf("Info for your PMT <-> board not present in the mysql database or do not match.\n\n");
            printf("For PMT = %d the board address in the MySQL database is not %d\n",pmtserialnumber,pmt);
            mysqlload = 0;
        restartchoice:
            printf("If you want to quit just type quit and press enter, if you want to continue and put the run info manually type manual\n");
            cin >> choice;
            if((choice.compare("quit") != 0) && (choice.compare("manual") != 0))
                goto restartchoice;
            if(choice.compare("quit") == 0)
                return;
            if(choice.compare("manual") == 0)
            {
                setpmtdata(usb,pmt);
                if(pmtdata[usb][pmt][4].compare("yes") == 0)
                {
                    printf("I will use if founded the MARCOC2 gain constants stored in this machine\n");
                }
                goto goend;
            }
        }
        else
        {
            if(choicefirst.compare("change") == 0)
            {
                choice = "manual";
                setpmtdata(usb,pmt);
                if(pmtdata[usb][pmt][4].compare("yes") == 0)
                {
                    printf("Please type which gain constants you would like to use (mysql or local)\n");
                    cin >> choicesecond;
                    if(choicesecond.compare("local") == 0)
                    {
                        mysqlload = 0;
                    }
                }
            }
        }
        printf("\n\n");
    }
    else
    {
        printf("Loading setting as default for pmt board %d\n",pmt);
        
        pmtdata[usb][pmt][0] = "none";
        pmtdata[usb][pmt][1] = to_string(pmt);
        pmtdata[usb][pmt][2] = to_string(0);
        pmtdata[usb][pmt][3] = to_string(1000);
        pmtdata[usb][pmt][4] = "no";
        pmtdata[usb][pmt][5] = "off";
        pmtdata[usb][pmt][6] = to_string(5);
        pmtdata[usb][pmt][7] = to_string(0b00000110);
        pmtdata[usb][pmt][8] = "test with sync configuration";
        
        totalpmt++;
        pmttousb[totalpmt] = usb;                    //usb number
        pmttoboard[totalpmt] = pmt;                  //board number
        structure[usb] = 0;
        
    }
    
goend:
    if(!SWITCH)
    {
        printf("Please enter comments for this run (max 50 characters), press enter when done\n");
        cin >> comments;
        pmtdata[usb][pmt][8] = comments;
    }
    pmtdata[usb][pmt][8] = comments;
    printf("\n");
    printf("\n");
    
    if(gate_override != "")
    {
        printf("%s\n",gate_override.c_str());
        if(!(gate_override.compare("off") == 0) || (gate_override.compare("alladcoff") == 0) || (gate_override.compare("on") == 0) || (gate_override.compare("trigger") == 0))
        {
            printf("Gate override option %s not defined. Available options: off, on, alladcoff, or trigger\n",gate_override.c_str());
            exit(1);
        }
        pmtdata[usb][pmt][5] = gate_override;
        printf("Warning! Gate override initiated by user! Now using gate = %s\n",gate_override.c_str());
        
    }
    
}
////////////////////////////////////////////////////////////////////////////////
int scanSignalFiles(vector<string> &fileList, string inputDirectory){
    inputDirectory = inputDirectory.append("/");

    DIR *p_dir;
    const char* str = inputDirectory.c_str();

    p_dir = opendir(str);
    if( p_dir == NULL)
    {
        cout << "can't open :" << inputDirectory << endl;
    }
 
    struct dirent *p_dirent;
    
    while ( p_dirent = readdir(p_dir))
    {
        string tmpFileName = p_dirent->d_name;
        if( tmpFileName == "." || tmpFileName == "..")
        {
            continue;
        }
        else
        {
            if(tmpFileName[0] == '1'){
                fileList.push_back(tmpFileName);
            }
        }
    }
    closedir(p_dir);
    return fileList.size();
}

///////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////////

void generatecsv(int usb, int pmt)
{
    getpmtdata(usb, pmt);
   
    string runtmp(run_number); 
    string full_path = "/e/h.0/localdev/readout/data1/OVDAQ/DATA/Run_" + runtmp;
   
    string full_path_temp = full_path;
    string homedir = "/e/h.0/localdev/readout";
    
    string baselines = full_path + "/binary/baselines.dat";
    string filename = full_path + "/binary/signal";
    
    int newrunlength = 1;
    vector<string> fileList;
    int filesize;
    filesize = scanSignalFiles(fileList,full_path + "/binary/");
    string cmd = "/e/h.0/localdev/readout/cppscript/decode/decode "; 
    for(int i = 0;i<filesize; i++){
        string cmd1;
        cmd1 = cmd + fileList[i] + " >> " + filename + ".dec";
        system(cmd1.c_str());
    }    

    string temp1 = filename + ".dec";
    
    signal(full_path, temp1, baselines, pmt, homedir);
}


//////////////////////////////////////////////////////////////////////////////////

void plotdatamb(int usb, int pmtmb)
{
    getpmtdata(usb, pmtmb);
    
    printf("Plotting data for USB=%d and PMT=%d\n",usb,pmtmb);
    
    string runtmp(run_number);
    string full_path = "/e/h.0/localdev/readout/data1/OVDAQ/DATA/Run_" + runtmp;
    string cmd = "/e/h.0/localdev/readout/cppscript/histogram/histogram ";   
 
    //make histograms
    string cmd1 = cmd + full_path + " summary.csv " + to_string(1) + " Pulse heights" + " >> log_" + to_string(pmtserialnumber) + "_" + runtmp + ".txt"; 
    string cmd2 = cmd + full_path + " summary.csv " + to_string(2) + " Sigma" + " >> log_" + to_string(pmtserialnumber) + "_" + runtmp + ".txt"; 
    string cmd3 = cmd + full_path + " summary.csv " + to_string(3) + " Hits by channel" + " >> log_" + to_string(pmtserialnumber) + "_" + runtmp + ".txt"; 
    string cmd5 = cmd + full_path + " summary.csv " + to_string(5) + " Photoelectrons" + " >> log_" + to_string(pmtserialnumber) + "_" + runtmp + ".txt"; 
    string cmd6 = cmd + full_path + " summary.csv " + to_string(6) + " Gains" + " >> log_" + to_string(pmtserialnumber) + "_" + runtmp + ".txt"; 
    string cmd7 = cmd + full_path + " summary.csv " + to_string(7) + " Rate" + " >> log_" + to_string(pmtserialnumber) + "_" + runtmp + ".txt"; 
    string cmd8 = cmd + full_path + " summary.csv " + to_string(8) + " Gain Constants" + " >> log_" + to_string(pmtserialnumber) + "_" + runtmp + ".txt"; 
    string cmd9 = cmd + full_path + " summary.csv " + to_string(9) + " Normalized Hits by channel" + " >> log_" + to_string(pmtserialnumber) + "_" + runtmp + ".txt"; 
    system(cmd1.c_str());
    system(cmd2.c_str());
    system(cmd3.c_str());
    system(cmd5.c_str());
    system(cmd6.c_str());
    system(cmd7.c_str());
    system(cmd8.c_str());
    system(cmd9.c_str());
    system(cmd1.c_str());

}


/////////////////////////////////////////////////////////////////////////////////

void signal(string dir, string file, string baselines, int pmt_board, string homedir)
{
    int nbins = 100;                    //number of bins in histograms
    int gain_c = 14500;                 //constant used in gain calculation
    
    string stefan = "Stefan Simion's Empty Histogram (tm). Formerly known as channel ";
    
    int x,f,i,j;
    double sig[65], sig_dev[65], base[65], base_dev[65], hist[65][65], hist2[65][65], hits_pc[65], bins[65], photo[65], gain_pmt[65], mean_norm[64], rate_norm[65];
    string title[65];
    for(i=1;i<=64;i++)
    {
        sig[i] = 0;
        sig_dev[i] = 0;
        base[i] = 0;
        base_dev[i] = 0;
        hits_pc[i] = 0;
        photo[i] = 0;
        gain_pmt[i] = 0;
        hist[0][i] = 0;
        mean_norm[i] = 0;
        rate_norm[i] = 0;
    }
    
    //get "typical module" data
    cout << "Getting data for typical module...\n";
    
    if(homedir != "")
    {
        string homedirstr = homedir + "/Scripts_mb/normalized.txt";
        ifstream infile(homedirstr.c_str());
        string line;
        size_t comma = 0;
        size_t comma2 = 0;
        size_t comma3 = 0;
        int a = 0;
        double b,c;
        
        if(!infile.good()){
            printf("Can not open file\n");
            infile.close();
            goto nohomedir;
        }
        else
        {
            getline(infile, line);                 //get rid of the first line
            while(!infile.eof())
            {
                getline(infile, line);
                
                comma = line.find(',',0);
                a = atoi(line.substr(0,comma).c_str());
                comma2 = line.find(',',comma + 1);
                b = atof(line.substr(comma+1,comma2-comma-1).c_str());
                comma3 = line.find(',',comma2 +1);
                c = atof(line.substr(comma2+1,comma3-comma2-1).c_str());
                mean_norm[a] = b;
                rate_norm[a] = c;
            }
        }
        infile.close();
    }
    
nohomedir:
    
    //get baselines
    cout << "Getting baselines...\n";
    
    if(baselines != "")
    {
        ifstream infile(baselines.c_str());
        string line;
        size_t comma = 0;
        size_t comma2 = 0;
        size_t comma3 = 0;
        int a = 0;
        double b,c;
        
        if(!infile.good())
        {
            cout << "Can not open file\n";
            infile.close();
        }
        else
        {
            getline(infile, line);                 //get rid of the first line
            while(!infile.eof())
            {
                getline(infile, line);
                
                comma = line.find(',',0);
                a = atoi(line.substr(0,comma).c_str());
                comma2 = line.find(',',comma + 1);
                b = atof(line.substr(comma+1,comma2-comma-1).c_str());
                comma3 = line.find(',',comma2 +1);
                c = atof(line.substr(comma2+1,comma3-comma2-1).c_str());
                base[a] = b;
                base_dev[a] = c;
            }
        }
        infile.close();
    }
    
    
    //parse data file
    
    cout << "Getting signal file..." << file << "\n";
    //parse data file
    string cmd = "/e/h.0/localdev/readout/cppscript/unpacksignal/unpacksignal " + file + " " + to_string(pmt_board);
    FILE *infile1 = popen(cmd.c_str(), "r");
    int first, last, count, countovf;
    count = 0;
    last = 0;
    first = 0;
    countovf = 0;
    char buf1[1024];
    int c,d,t;
    size_t comma = 0;
    size_t comma2 = 0;
    size_t comma3 = 0;
    
    if(!infile1)
    {
        cout << "Can not open file\n";
        pclose(infile1);
    }
    else
    {
        while(fgets(buf1,1024,infile1))
        {   
            string line(buf1);
            comma = line.find(',',0);
            c = atoi(line.substr(0,comma).c_str());
            comma2 = line.find(',',comma + 1);
            d = atof(line.substr(comma+1,comma2-comma-1).c_str());
            comma3 = line.find(',',comma2 +1);
            t = atof(line.substr(comma2+1,comma3-comma2-1).c_str());
            
            if(count == 0)
            {
                first = t;
                count++;
            }
            if(t<last)
            {
                countovf++;
            }
            last = t;
            if(c==0)
            {
                hist[c][d]++;                 //hist[0] = hits per packet
            }
            else{
                d -= base[c];
                hist[c][d]++;
                hits_pc[c]++;
            }
        }
        pclose(infile1);
    }
    double rate;
    countovf = countovf *pow(2,32);
    rate = (last + countovf - first) * 16 * pow(10,-9);
    if(rate)
        rate = 1/rate;
    
    
    
    //calculate
    cout << "Calculating..\n";
    int min = 10000;
    int max = -10000;
    int max_c = -10000;
    for(int i = 1; i<=64; i++)
    {
        int avg = 0;
        int dev = 0;
        int n = hits_pc[i];
        if(n == 0)
            continue;
        int j = 0;
        while(hist[j][i])
        {
            x = j;
            f = hist[j][i];
            avg += f * x;
            if(x<min)
                min = x;
            if(x>max)
                max = x;
            j++;
        }
        avg = avg/n;
        j = 0;
        while(hist[j][i])
        {
            x = j;
            f = hist[j][i];
            dev += f * pow((x-avg),2);
            j++;
        }
        dev = sqrt(dev/n);
        sig[i] = avg;
        sig_dev[i] = dev;
        if(sig_dev[i])
            photo[i] = sig[i]/(pow(sig_dev[i],2));
        if(photo[i])
            gain_pmt[i] = gain_c * sig[i] / photo[i];
    }
    
    // histogram hits
    cout << "Histogramming...\n";
    int wbins = (max - min)/nbins;
    if(wbins == 0)
        wbins = 1;
    min -= 0.0001 * wbins;
    max += 0.0001 * wbins;
    wbins = (max - min)/nbins;
    for(j = 0; j < nbins; j++)
    {
        hist2[0][j] = min + (j+0.5) * wbins;
    }
    for( i = 1; i <= 64; i++)
    {
        for( j = 0; j<nbins; j++)
            bins[j] = 0;
        int t = 0;
        while(hist[t][i])
        {
            x = t;
            f = hist[t][i];
            bins[int((x-min)/wbins)] += f;
            t++;
        }
        if(hits_pc[i] == 0)
        {
            bins[0] += 0.00001;
            title[i] = stefan + to_string(i);
        }
        else
        {
            title[i] = "Channel " + to_string(i) + "  Mean = " + to_string(sig[i]) + "  Std Dev = " + to_string(sig_dev[i]) + "  Total = " + to_string(hits_pc[i]);
        }
        for(j=0; j<nbins; j++)
        {
            hist2[i][j] = bins[j];
            if(bins[j] > max_c)
                max_c = bins[j];
        }
    }
    
    // raw data file
    FILE * rawdata;
    string cmd1 = dir + "/hist.csv";
    rawdata = fopen(cmd1.c_str(),"w");
    if(rawdata == NULL){
        printf("Can not open file\n");
        fclose(rawdata);
    }
    else
    {
        for(i = 1; i<=64; i++)
        {
            fprintf(rawdata,"# Channel %d\n", i);
            if(hits_pc[i] == 0)
                fprintf(rawdata,"%d,0\n%d,0\n",min,max);
            else
            {
                j = 0;
                while(hist[j][i])
                {
                    x = j;
                    f = hist[j][i];
                    fprintf(rawdata,"%d,%d\n",x,f);
                }
            }
            fprintf(rawdata,"\n\n");
        }
        fclose(rawdata);
    }
    
    // histogram file
    FILE * histfile;
    string cmd2 = dir + "/hist.csv";
    histfile = fopen(cmd2.c_str(),"w");
    if(histfile == NULL){
        printf("Can not open file\n");
        fclose(histfile);
    }
    else
    {
        fprintf(histfile,"#x,");
        for(i=1;i<=64;i++)
            fprintf(histfile,"%d",i);
        fprintf(histfile,"\n");
        //fprintf(histfile,"#x,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,52,53,54,55,56,57,58,59,60,61,62,63,64\n");
        for(j=0;j<nbins;j++)
        {
            fprintf(histfile,"%f",hist2[0][j]);
            for(i=1;i<=64;i++)
                fprintf(histfile,"%f",hist2[i][j]);
            fprintf(histfile,"\n");
        }
        fclose(histfile);
    }
    
    // summary file
    FILE * summaryfile;
    string cmd3 = dir + "/summary.csv";
    summaryfile = fopen(cmd3.c_str(),"w");
    if(summaryfile == NULL){
        printf("Can not open file\n");
        fclose(summaryfile);
    }
    else
    {
        fprintf(summaryfile,"#channel,mean,dev,hits_per_channel,hits_per_packet,photo_e,gain_pmt,rate_per_channel,gain_constant,normalized_rate\n");
        for(i=1; i<=64; i++)
        {
            double tmp1 = hits_pc[i] * rate;
            double tmp2 = 0;
            if(rate_norm[i])
                tmp2 = tmp1/rate_norm[i];
            fprintf(summaryfile,"%d,%f,%f,%f,%f,%f,%f,%f,%d,%f\n",i,sig[i],sig_dev[i],hits_pc[i],hist[0][i],photo[i],gain_pmt[i],tmp1,gain[i],tmp2);
        }
        fclose(summaryfile);
    }
    
    // 2d summary
    FILE * summaryfile_2d;
    string cmd4 = dir + "/summary_2d.csv";
    summaryfile_2d = fopen(cmd4.c_str(),"w");
    if(summaryfile_2d == NULL){
        printf("Can not open file\n");
        fclose(summaryfile_2d);
    }
    else
    {
        fprintf(summaryfile_2d,"#x,y,mean,dev,hits_per_channel,hits_per_packet,photo_e,gain_pmt,norm_mean\n");
        for(i=0;i<8;i++)
        {
            for(j=0;j<8;j++)
            {
                double xx = i + 0.5;
                double yy = j + 0.5;
                int c = 8 * i + j + 1;
                if(c<=64)
                {
                    double tmp3 = 0;
                    if(mean_norm[c])
                        tmp3 = sig[c]/mean_norm[c];
                    fprintf(summaryfile_2d,"%f,%f,%f,%f,%f,%f,%f,%f,%f\n",xx,yy,sig[c],sig_dev[c],hits_pc[c],hist[0][c],photo[c],gain_pmt[c],tmp3);
                }
                else
                {
                    fprintf(summaryfile_2d,"%f,%f,0,0,0,0,0,0,0\n",xx,yy);
                }
            }
            fprintf(summaryfile_2d,"\n");
        }
        fclose(summaryfile_2d);
    }
    
    // 2d histograms
    FILE * histfile_2d;
    string cmd5 = dir + "/hist_2d.csv";
    histfile_2d = fopen(cmd5.c_str(),"w");
    if(histfile_2d == NULL){
        printf("Can not open file\n");
        fclose(histfile_2d);
    }
    else
    {
        fprintf(histfile_2d,"#x,y,z\n");
        for(i=0;i<64;i++)
        {
            for(j=0;j<nbins;j++)
            {
                double xx = i + 0.5;
                double yy = min + (j + 0.5) * wbins;
                if((i<64) && (j<nbins))
                {
                    fprintf(histfile_2d,"%f,%f,%f\n",xx,yy,hist2[i+1][j]);
                }
                else
                {
                    fprintf(histfile_2d,"%f,%f,0\n",xx,yy);
                }
            }
            fprintf(histfile_2d,"\n");
        }
        fclose(histfile_2d);
    }
    
    // change min and max so we can use them as ranges
    min += 0.5 * wbins;
    max -= 0.5 * wbins;
    
    // generate plots
    cout << "Plotting...\n" ;
    FILE * PLOT;
    string cmd6 = dir + "/plot.gp";
    PLOT = fopen(cmd6.c_str(),"w");
    if(PLOT == NULL){
        printf("Can not open file\n");
        fclose(PLOT);
    }
    else
    {
        fprintf(PLOT,"\n");
        fprintf(PLOT,"set grid\n");
        fprintf(PLOT,"set datafile separator \",\"\n");
        fprintf(PLOT,"set term png size 1024,768\n");
        fprintf(PLOT,"\n");
        
        // summary
        fprintf(PLOT,"set title \"ADC pulse heights\"\n");
        fprintf(PLOT,"set xlabel \"Channel #\"\n");
        fprintf(PLOT,"set ylabel \"ADC counts\"\n");
        fprintf(PLOT,"set autoscale\n");
        fprintf(PLOT,"set xrange [0.5:64.5]\n");
        fprintf(PLOT,"set yrange [%d:*]\n",min);
        fprintf(PLOT,"set output \"%s/mean.png\"\n",dir.c_str());
        fprintf(PLOT,"plot \"%s/summary.csv\" using 1:2 title \"Mean\" with histeps\n",dir.c_str());
        fprintf(PLOT,"\n");
        
        fprintf(PLOT,"set title \"ADC sigma\"\n");
        fprintf(PLOT,"set xlabel \"Channel #\"\n");
        fprintf(PLOT,"set ylabel \"ADC counts\"\n");
        fprintf(PLOT,"set autoscale\n");
        fprintf(PLOT,"set xrange [0.5:64.5]\n");
        fprintf(PLOT,"set yrange [0:*]\n");
        fprintf(PLOT,"set output \"%s/sigma.png\"\n",dir.c_str());
        fprintf(PLOT,"plot \"%s/summary.csv\" using 1:3 title \"Sigma\" with histeps\n",dir.c_str());
        fprintf(PLOT,"\n");
        
        fprintf(PLOT,"set title \"Hits by channel\"\n");
        fprintf(PLOT,"set xlabel \"Channel #\"\n");
        fprintf(PLOT,"set ylabel \"Count\"\n");
        fprintf(PLOT,"set autoscale\n");
        fprintf(PLOT,"set xrange [0.5:64.5]\n");
        fprintf(PLOT,"set yrange [0:*]\n");
        fprintf(PLOT,"set output \"%s/per_channel.png\"\n",dir.c_str());
        fprintf(PLOT,"plot \"%s/summary.csv\" using 1:4 title \"Hits\" with histeps\n",dir.c_str());
        fprintf(PLOT,"\n");
        
        fprintf(PLOT,"set title \"Hits in ADC packet\"\n");
        fprintf(PLOT,"set xlabel \"Number #\"\n");
        fprintf(PLOT,"set ylabel \"Count\"\n");
        fprintf(PLOT,"set autoscale\n");
        fprintf(PLOT,"set xrange [0.5:64.5]\n");
        fprintf(PLOT,"set yrange [0:*]\n");
        fprintf(PLOT,"set output \"%s/per_packet.png\"\n",dir.c_str());
        fprintf(PLOT,"plot \"%s/summary.csv\" using 1:5 title \"Hits\" with histeps\n",dir.c_str());
        fprintf(PLOT,"\n");
        
        fprintf(PLOT,"set title \"Photoelectrons\"\n");
        fprintf(PLOT,"set xlabel \"Channel #\"\n");
        fprintf(PLOT,"set ylabel \"Number\"\n");
        fprintf(PLOT,"set autoscale\n");
        fprintf(PLOT,"set xrange [0.5:64.5]\n");
        fprintf(PLOT,"set yrange [0:*]\n");
        fprintf(PLOT,"set output \"%s/photo_e.png\"\n",dir.c_str());
        fprintf(PLOT,"plot \"%s/summary.csv\" using 1:6 title \"Photoelectrons\" with histeps\n",dir.c_str());
        fprintf(PLOT,"\n");
        
        fprintf(PLOT,"set title \"Gain\"\n");
        fprintf(PLOT,"set xlabel \"Channel #\"\n");
        fprintf(PLOT,"set ylabel \"Number\"\n");
        fprintf(PLOT,"set autoscale\n");
        fprintf(PLOT,"set xrange [0.5:64.5]\n");
        fprintf(PLOT,"set yrange [0:*]\n");
        fprintf(PLOT,"set output \"%s/gain.png\"\n",dir.c_str());
        fprintf(PLOT,"plot \"%s/summary.csv\" using 1:7 title \"Gain\" with histeps\n",dir.c_str());
        fprintf(PLOT,"\n");
        
        fprintf(PLOT,"set title \"Data acquisition rate per channel\"\n");
        fprintf(PLOT,"set xlabel \"Channel #\"\n");
        fprintf(PLOT,"set ylabel \"Rate [HZ]\"\n");
        fprintf(PLOT,"set autoscale\n");
        fprintf(PLOT,"set xrange [0.5:64.5]\n");
        fprintf(PLOT,"set yrange [0:*]\n");
        fprintf(PLOT,"set output \"%s/rate_per_channel.png\"\n",dir.c_str());
        fprintf(PLOT,"plot \"%s/summary.csv\" using 1:8 title \"Rate (HZ)\" with histeps\n",dir.c_str());
        fprintf(PLOT,"\n");
        
        fprintf(PLOT,"set title \"Maroc Gain Constants\"\n");
        fprintf(PLOT,"set xlabel \"Channel #\"\n");
        fprintf(PLOT,"set ylabel \"Number\"\n");
        fprintf(PLOT,"set autoscale\n");
        fprintf(PLOT,"set xrange [0.5:64.5]\n");
        fprintf(PLOT,"set yrange [0:42]\n");
        fprintf(PLOT,"set output \"%s/gainconst.png\"\n",dir.c_str());
        fprintf(PLOT,"plot \"%s/summary.csv\" using 1:9 title \"Maroc Gain Constants\" with histeps\n",dir.c_str());
        fprintf(PLOT,"\n");
        
        fprintf(PLOT,"set title \"Data acquisition rate per channel (normalized)\"\n");
        fprintf(PLOT,"set xlabel \"Channel #\"\n");
        fprintf(PLOT,"set ylabel \"Normalized Rate\"\n");
        fprintf(PLOT,"set autoscale\n");
        fprintf(PLOT,"set xrange [0.5:64.5]\n");
        fprintf(PLOT,"set yrange [0:*]\n");
        fprintf(PLOT,"set output \"%s/rate_per_channel_normalized.png\"\n",dir.c_str());
        fprintf(PLOT,"plot \"%s/summary.csv\" using 1:10 title \"Normalized Rate\" with histeps\n",dir.c_str());
        fprintf(PLOT,"\n");
        
        // histograms - individual channels
        for(i=1;i<=64;i++)
        {
            int minus1 = i - 1;
            int plus1 = i + 1;
            string tmp = (i < 10) ? ("0" + to_string(i)) : to_string(i);
            fprintf(PLOT,"set title \"%s\"\n",title[i].c_str());
            fprintf(PLOT,"set autoscale\n");
            fprintf(PLOT,"set yrange [0:%d]\n",max_c);
            fprintf(PLOT,"set xlabel \"ADC counts\"\n");
            fprintf(PLOT,"set ylabel \"Count\"\n");
            fprintf(PLOT,"set output \"%s/adc_%s.png\"\n",dir.c_str(),tmp.c_str());
            fprintf(PLOT,"plot \"%s/hist.csv\" using 1:%d title \"Histogram\" with histeps, \\ \"%s/raw.csv\" index %d title \"Raw data\" with impulses\n",dir.c_str(),plus1,dir.c_str(),minus1);
            fprintf(PLOT,"\n");
        }
        
        // histograms - all in one
        fprintf(PLOT,"set title \"All channels\"\n");
        fprintf(PLOT,"set xlabel \"ADC counts\"\n");
        fprintf(PLOT,"set ylabel \"Count\"\n");
        fprintf(PLOT,"set output \"%s/all_in_one.png\"\n",dir.c_str());
        fprintf(PLOT,"set autoscale\n");
        fprintf(PLOT,"set yrange [0:%d]\n",max_c);
        fprintf(PLOT,"set key spacing 0.5\n");
        fprintf(PLOT,"\\\n");
        for(i=1;i<=64;i++)
        {
            int plus1 = i + 1;
            fprintf(PLOT,"\"%s/hist.csv\" using 1:%d title \"Channel %d\" with histeps\n",dir.c_str(),plus1,i);
            if(i<64)
                fprintf(PLOT,", \\\n");
        }
        fprintf(PLOT,"\n");
        
        // 2d pulse heights
        fprintf(PLOT,"set title \"ADC pulse heights\"\n");
        fprintf(PLOT,"unset grid\n");
        fprintf(PLOT,"set xlabel \"X\"\n");
        fprintf(PLOT,"set ylabel \"Y\"\n");
        fprintf(PLOT,"set autoscale\n");
        fprintf(PLOT,"set xrange [0.5:8.5]\n");
        fprintf(PLOT,"set yrange [0.5:8.5]\n");
        fprintf(PLOT,"set pm3d at b corners2color c1\n");
        fprintf(PLOT,"set view map\n");
        fprintf(PLOT,"set sytle data pm3d\n");
        fprintf(PLOT,"set output \"%s/mean_2d.png\"\n",dir.c_str());
        fprintf(PLOT,"splot \"%s/summary_2d.csv\" using 1:2:3 title \"\"\n",dir.c_str());
        fprintf(PLOT,"unset pm3d\n");
        fprintf(PLOT,"set grid\n");
        fprintf(PLOT,"\n");
        
        // 2d pulse heights normalized
        fprintf(PLOT,"set title \"Normalized ADC pulse heights\"\n");
        fprintf(PLOT,"unset grid\n");
        fprintf(PLOT,"set xlabel \"X\"\n");
        fprintf(PLOT,"set ylabel \"Y\"\n");
        fprintf(PLOT,"set autoscale\n");
        fprintf(PLOT,"set xrange [0.5:8.5]\n");
        fprintf(PLOT,"set yrange [0.5:8.5]\n");
        fprintf(PLOT,"set pm3d at b corners2color c1\n");
        fprintf(PLOT,"set view map\n");
        fprintf(PLOT,"set sytle data pm3d\n");
        fprintf(PLOT,"set output \"%s/mean_normalized_2d.png\"\n",dir.c_str());
        fprintf(PLOT,"splot \"%s/summary_2d.csv\" using 1:2:9 title \"\"\n",dir.c_str());
        fprintf(PLOT,"unset pm3d\n");
        fprintf(PLOT,"set grid\n");
        fprintf(PLOT,"\n");
        
        // 2d all in one
        fprintf(PLOT,"set title \"Histograms\"\n");
        fprintf(PLOT,"unset grid\n");
        fprintf(PLOT,"set xlabel \"Channel\"\n");
        fprintf(PLOT,"set ylabel \"X\"\n");
        fprintf(PLOT,"set autoscale\n");
        fprintf(PLOT,"set xrange [0.5:64.5]\n");
        fprintf(PLOT,"set yrange [%d:%d]\n",min,max);
        fprintf(PLOT,"set pm3d at b corners2color c1\n");
        fprintf(PLOT,"set view map\n");
        fprintf(PLOT,"set sytle data pm3d\n");
        fprintf(PLOT,"set output \"%s/hist_2d.png\"\n",dir.c_str());
        fprintf(PLOT,"splot \"%s/hist_2d.csv\" using 1:2:3 title \"\"\n",dir.c_str());
        fprintf(PLOT,"unset pm3d\n");
        fprintf(PLOT,"set grid\n");
        fprintf(PLOT,"\n");
        
        // histograms - 8x8 individual channels
        fprintf(PLOT,"set output \"%s/all_in_8x8.png\"\n",dir.c_str());
        fprintf(PLOT,"unset grid\n");
        fprintf(PLOT,"unset title\n");
        fprintf(PLOT,"unset xlabel\n");
        fprintf(PLOT,"unset ylabel\n");
        fprintf(PLOT,"unset key\n");
        fprintf(PLOT,"set autoscale\n");
        fprintf(PLOT,"set yrange [0:%d]\n",max_c);
        fprintf(PLOT,"set noxtics\n");
        fprintf(PLOT,"set noytics\n");
        fprintf(PLOT,"set format x \"\"\n");
        fprintf(PLOT,"set format y \"\"\n");
        fprintf(PLOT,"set multiplot layout 8,8\n");
        fprintf(PLOT,"\n");
        
        for(i=0;i<64;i++)
        {
            int plus2 = i + 2;
            fprintf(PLOT,"plot \"%s/hist.csv\" using 1:%d with histeps, \\ \"%s/raw.csv\" index %d with impulses\n",dir.c_str(),plus2,dir.c_str(),i);
            fprintf(PLOT,"\n");
        }
        fprintf(PLOT,"unset multiplot\n");
        
        fclose(PLOT);
        string plotstr = "/usr/bin/gnuplot " + dir + "/plot.gp";
        int status = system(plotstr.c_str());
        if(status != 0)
        {
            cout << "Error running gnuplot: " << status << endl;
        }
    }
    
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

//////////////////////////////////////////////////////////////////////////////////


int main()
{
    int usb = 33;
    run_length = 5;
    int debug = 0;           // 1=debug mode, 0=mysql
    pmt = 3;
    mode = "debug";
    
    int disk = 1;           //Disk number (1 or 2)
    string DataPath = "/OVDAQ/DATA";
    string DataFolder = "/e/h.0/localdev/readout/data" + to_string(disk) + DataPath;
    int totalpmt = 1;
    int totusb = 1;

    string online_path = "/e/h.0/localdev";
    string DataPath2 = "/e/h.0/localdev/readout/DCOV";
    
    string cmd = online_path + "\\/readout\\/script\\/start_readout.sh \"readout\" \"1\" \"1\" \"none\"";
    system(cmd.c_str());   

    loadpmtdata(usb,pmt,debug,"off");
    
  //  test_system_time(usb,pmt);
   
  //  sleep(5); 

  //  check_system_time;
    
    initializeboard("auto",500,0,0);
    
    starttakedata(0,0,0,0);
    
    sleep(run_length);
    
    stoptakedata(0,0,0,0);
    
  //  check_rate()
  
    string baselines_Path = "/e/h.0/localdev/readout/cppscript/baselines/";
    string cmd_baseline = baselines_Path + "baselines " + DataFolder + "/Run_" + run_number + "/binary/ " + "baseline_" + to_string(usb) + " " + to_string(pmt);
    system(cmd_baseline.c_str());
  
    generatecsv(usb,pmt);

    plotdatamb(usb,pmt); 
    
}










