#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <ctype.h>
#include <vector>
#include <cmath>
#include <sstream>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <string>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include "usbreadout.h"


#define buf_size 1024
#define MSG_BUF_SIZE 32
#define MSG_BUF_SIZE_2 5

//using namespace std;
using std::cout;
using std::endl;
using std::string;

int Msg_Base = 10000;
long MQs[128][128];
double DBH;
int Error=0;      // 0=no error; 1=error
int MQ_size=0;
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
    cout<<sig<<endl;
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
    //    printf("Size of MQ=%d\n",MQ_size);
}

//mq_send $msg_id,$data
void mq_send(int msg, char data[]){
    int mq0;
    int mq1;
    struct mymsg msgp;
    memset(msgp.mtext,0,MSG_BUF_SIZE);
    //MQ_size = 0;
    //    if(MQs[0][0]==0)
    if(MQ_size == 0)  find_mqs();
    while(1){
        try{
            signal(SIGALRM,handle);
            alarm(1);
            try{
                msgp.mtype = msg;
                //strcpy((char *)msgp.mtext,data);
		for(int t =0;t<MSG_BUF_SIZE;t++)
                    msgp.mtext[t] = data[t];
                // sprintf (msgp.mtext,"%d",data[0]);
                // msgp.mtext[0] = data[0];
                // int a = *((int *)msgp.mtext);
                // printf("disk = %d \n",a);
                // cout << "msgp = " << *((int *)msgp.mtext) << "\n";
                for(int i = 0; i<=MQ_size;i++){
                    msgsnd(MQs[0][i],(void*)&msgp,MSG_BUF_SIZE,IPC_NOWAIT | MSG_NOERROR);
                    //cout << "mq1 = " << MQs[1][i] << ",mq0 = " << MQs[0][i] << ",msg = " << msgp.mtype << ",data = " << *((int *)msgp.mtext) << ", " << msgp.mtext << "\n";
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
    ////memset(msgp.mtext,0,MSG_BUF_SIZE);
    memset(msgp.mtext,0,MSG_BUF_SIZE);
    //MQ_size = 0;
    //if(MQs[0][0]==0)

    if(MQ_size==0) find_mqs();

    while(1){
        try{
            signal(SIGALRM,handle);
            alarm(1);
            try{
                msgp.mtype = msg;
		for(int t =0;t<MSG_BUF_SIZE;t++)
                    msgp.mtext[t] = data[t];
		//                strcpy((char *)msgp.mtext, data);
                for(int i =0;i<=MQ_size;i++){
                    msgsnd(MQs[0][i],(void*)&msgp,MSG_BUF_SIZE,IPC_NOWAIT|MSG_NOERROR);
                    //cout << "mq1 = " << MQs[1][i] << ",mq0 = " << MQs[0][i] << ",msg = " << msgp.mtype << ",data = " << msgp.mtext << "\n";
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
        find_mqs();                             //remove it from MQs
    }
}


//mq_send_usb $msg_id, $usb_num,$data;
void mq_send_usb (int msg, int usb_num, unsigned char data[]){
    //std::cout<<"inside mq_send_usb"<<std::endl;
    int mq0;
    int mq1;
    struct mymsg1 msgp;
    // memset(msgp.mtext,0,4);
   // std::cout<<"memset"<<std::endl;
    memset(msgp.mtext,'\0',MSG_BUF_SIZE_2);
    // struct msgbu* msgp = (struct msgbuf*)malloc(sizeof(struct msgbuf) + sizeof(data));
    //    MQ_size = 0;
    //if(MQs[0][0]==0)
    if(MQ_size ==0 )    find_mqs();
    usb_num += Msg_Base;
    while(1){
        try{
   // std::cout<<"try block"<<std::endl;
            signal(SIGALRM,handle);
            alarm(1);
            try{
   //std::cout<<"second try block"<<std::endl;
                msgp.mtype = msg;
                // strcpy((char *)msgp.mtext,(char *)data);
                //int msgsz = sizeof(msgp) - sizeof(long);
                for(int t =0;t<4;t++)
    {//std::cout<<"for loop "<<t<<std::endl;
                    msgp.mtext[t] = data[t];}
                // int tmp1 = *((int *)msgp.mtext);
                // printf ("tmp = %d\n", tmp1);
                for(int i=0; i<=MQ_size; i++){
    //std::cout<<"second for loop "<<i<<std::endl;
		  //cout << "mq1 = " << MQs[1][i] << ",mq0 = " << MQs[0][i] << ",msg = " << msgp.mtype << ",data = " << msgp.mtext << ",mdata= " << (unsigned int *) msgp.mtext <<"\n";
                    //    printf("mdata = %d\n",(unsigned int *)msgp.mtext);
                    // for(int j = 0;j<4;j++)    printf("%d",msgp.mtext[j]);
                    // printf("\n");
                    mq1 = MQs[1][i];
                    if(mq1==usb_num){
                        //int rc = msgsnd(MQs[0][i],(void*)&msgp,msgsz, 0);
   //std::cout<<"if statement"<<std::endl;
                        int rc = msgsnd(MQs[0][i],(void*)&msgp,4, IPC_NOWAIT | MSG_NOERROR);
                        if(rc == -1){
                            std::cerr<<"errno = "<<errno<<std::endl;
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
    //std::cout<<"inside send_out_usb"<<std::endl;
    unsigned char data1[4];
    //std::cout<<"memset"<<std::endl;
    memset(data1,'\0',4);
    memset(data1,0,4);
    //strcpy(data1,data.c_str());
    // data1[0] = usbdata;
    // memcpy(data1,&usbdata,sizeof(int));
    ////std::cout<<"starting data block"<<std::endl;
    data1[0] = usbdata & 0xff;
    data1[1] = (usbdata>>8) & 0xff;
    data1[2] = (usbdata>>16) & 0xff;
    data1[3] = (usbdata>>24) & 0xff;
    //cout << "data1[0] = " << data1[0] << (usbdata) << " & " << 0xff << "\n";
   // cout << "data1[1] = " << data1[1] << (usbdata>>8) << " & " << 0xff << "\n";
    //cout << "data1[2] = " << data1[2] << (usbdata>>16) << " & " << 0xff << "\n";
    //cout << "data1[3] = " << data1[3] << (usbdata>>24) << " & " << 0xff << "\n";
   // std::cout<<"mq_send_usb"<<std::endl;
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
    //std::cout<<"inside com_usb"<<std::endl;
    unsigned int b1 = 4194304+((pmt & 63) * 256 + b2) *256 + b3;
    //cout << "b2 = " << b2 << ", b3 = " << b3 << ", b1 = " << b1 << "\n";
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


