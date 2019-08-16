#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

typedef struct {
	uint16_t mac5;
	uint16_t flags;
	uint16_t lostcpu;
	uint16_t lostfpga;
	uint32_t ts0;
	uint32_t ts1;
	uint16_t adc[32];
	uint32_t coinc;
} EVENT_t_pac;

typedef struct {
	uint16_t mac5; //==0xFFFF
	uint16_t flags;//==0xFFFF
	//uint16_t lostcpu;
	//uint16_t lostfpga; 
	uint32_t ts0;  // ==MAGICWORD32
	uint32_t ts1;  // ==MAGICWORD32
	uint32_t coinc;
	uint16_t nevsinpoll; 
	uint32_t start_s;
	uint32_t d1;
	uint16_t start_ms;
	uint16_t dd2;
	uint32_t d2;
	uint32_t end_s;
	uint32_t d3;
	uint16_t end_ms;
} EOP_EVENT_t;


EVENT_t_pac evbufr[4];

#define events_per_pack 1024  //number of events send per package

int ConvertBin2Root(char *fname)
{
	int i,j;
	FILE *data;
	uint16_t mac5;
	uint16_t flags;
	uint16_t lostcpu;
	uint16_t lostfpga;
	uint32_t ts0;
	uint32_t ts1;
	uint16_t adc[32];
	uint32_t coinc; 

	int msec;
	int sec;
	int emsec;
	int esec;
	int nevsinpoll;

	data=fopen(fname,"r");
	if (!data)
		return 1;
	fseek(data, 0, SEEK_END); // seek to end of file

	int size = ftell(data); // get current file pointer
	cout << "size " << size << " " << sizeof(EVENT_t_pac) << " " << sizeof(EOP_EVENT_t)<< endl;

	cout << "size " << size << " " << sizeof(EVENT_t) << " " << sizeof(EOP2_EVENT_t)<< endl;

	fseek(data, 0, SEEK_SET); // seek back to beginning of file

	int size_ev=size/sizeof(EVENT_t_pac); //number of total events
	printf("Total Number of events: %d\n",size_ev);

	EVENT_t_pac evbuf[events_per_pack+1];
	int ev_counter=0;
	char ofname[64];
	sprintf(ofname,"mppc_%s.root",fname);
	
	TFile *myfile = new TFile(ofname,"RECREATE");
	TTree *tr=new TTree("mppc","mppc");
	tr->Branch("mac5",&mac5,"mac5/b");
	tr->Branch("flags",&flags,"flags/b");
	tr->Branch("chg",adc,"chg[32]/s");
	tr->Branch("ts0",&ts0,"ts0/i");
	tr->Branch("ts1",&ts1,"ts1/i");
	tr->Branch("coincidence",&coinc,"coinc/i"); 

	while(ev_counter<size_ev)
	{
		//break;
		//Fill the buffer
		for (int counter=0; counter < events_per_pack; counter++)
		{
			if(counter>50) break;	
			fread(&evbuf[counter],sizeof(EVENT_t_pac),1,data);
			if(evbuf[counter].mac5<0xFF)
			{
				flags = evbuf[counter].flags;
				mac5  = evbuf[counter].mac5;
				ts0   = evbuf[counter].ts0;
				ts1   = evbuf[counter].ts1;
				for(int i = 0; i<32; i++) adc[i] = evbuf[counter].adc[i];
				coinc = evbuf[counter].coinc;
				cout << mac5 
					 << " " << flags 
					 << " " << ts0 
					 << " " << ts1
					 << " " << coinc;
				for(int i = 0; i<32; i++) cout << " " << adc[i];
				cout << endl;
				tr->Fill();
			}
			if (evbuf[counter].mac5>=0xFF)
			{
				sec=((EOP_EVENT_t*)(&evbuf[counter].mac5))->start_s;
				msec=((EOP_EVENT_t*)(&evbuf[counter].mac5))->start_ms;
				esec=((EOP_EVENT_t*)(&evbuf[counter].mac5))->end_s;
				emsec=((EOP_EVENT_t*)(&evbuf[counter].mac5))->end_ms;
				nevsinpoll=((EOP_EVENT_t*)(&evbuf[counter].mac5))->nevsinpoll;
				printf("EOP: start %d s %d ms; end %d s %d ms; %d events in poll. \n",sec,msec,esec,emsec,nevsinpoll);
				cout << "EOP "<< ((EOP_EVENT_t*)(&evbuf[counter].mac5))->mac5
					 << " " << ((EOP_EVENT_t*)(&evbuf[counter].mac5))->flags 
					 << " " << ((EOP_EVENT_t*)(&evbuf[counter].mac5))->ts0 
					 << " " << ((EOP_EVENT_t*)(&evbuf[counter].mac5))->ts1 
					 << endl; 
			}
		}
		ev_counter+=events_per_pack;
		printf("sent events: %d\n", ev_counter);
	}
	tr->Write(0,TObject::kOverwrite);
	myfile->Close();
	fclose(data);
	printf("\n");
	return 0;
}


