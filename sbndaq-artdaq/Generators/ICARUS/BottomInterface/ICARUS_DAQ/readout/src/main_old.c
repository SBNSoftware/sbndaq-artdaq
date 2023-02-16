#define DAEMONIZE 1 // 0
#define BUF_NO (64) //256               -- 32 ok
#define BUF_SIZE (64 * 1024)//32 * 1024 -- 64 *1024 ok	
#define DATA_IN_TIMEOUT 2000 // 500     -- 1000 ok

#include <aio.h>
#include <errno.h>
#include <fcntl.h>
#include <mcheck.h>
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h> 
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <netdb.h> 
#include <syslog.h> 
#include <signal.h> 
#include <libusb-1.0/libusb.h>

// usb
//#include "libusb-1.0/libusb.h"

#define IN_DEV		( LIBUSB_ENDPOINT_IN \
                    | LIBUSB_REQUEST_TYPE_STANDARD \
					| LIBUSB_RECIPIENT_DEVICE)
#define GET_DESC 	LIBUSB_REQUEST_GET_DESCRIPTOR
#define GET_STR		(LIBUSB_DT_STRING << 8)

// endpoints
#define EP_DATA_IN	(LIBUSB_ENDPOINT_IN | 2)
#define EP_DIAG_IN	(LIBUSB_ENDPOINT_IN | 6)
#define EP_COM_OUT	8

// messages
#define MSG_COM_OUT 1
#define MSG_LATENCY 2
#define MSG_FILE 3
#define MSG_INHIBIT 4
#define MSG_RUN_NUMBER 5
#define MSG_DATA_DISK 6
#define MSG_DATA_PATH 7

#define MSG_BUF_SIZE 2048 //1024

// com in
#define COM_IN_TIMEOUT 500 //100

// com out
#define COM_OUT_TIMEOUT 500 //100
#define MAX_ATTEMPTS 10
#define COM_OUT_SIZE (MSG_BUF_SIZE)		// these should be equal


// gaibu definition
#define BUFSIZE 	1024 
static int gaibu_sockfd; // make an array
struct sockaddr_in gaibu_serveraddr;
char gaibu_msg_buf[BUFSIZE];
char GAIBU_SERVER_IP[BUFSIZE];
long int GAIBU_PORTNUMBER;


struct msgbuf_t {
	long			mtype;
	unsigned char	mdata[MSG_BUF_SIZE];
};

#define MNAME -1
#define MINSTRUCTION 1
#define MNOTICE 2
#define MWARNING 3
#define MERROR 4
#define MEXCEPTION 5

struct msgbuf_t msg_buf;
static int msqid, msq_common;

// exit strategy
static int kill_me = 0;
static void sighandler(int signum) { kill_me = 1; }

// usb
static int ser_no = 0;
struct libusb_device **devs;
struct libusb_device *dev;
static struct libusb_device_handle *devh = NULL;
int usbsystemdevice = 0;

// diag in
static struct libusb_transfer *diag_in_transfer = NULL;
static unsigned char diag_in_buf[512];
static unsigned int diag_expecting, diag_word, diag_result;
static int diag_in_ready;

static struct libusb_context *ctx;

static struct libusb_transfer *com_out_transfer = NULL;
static unsigned char com_out_buf[COM_OUT_SIZE];
static int com_out_ready;
static int com_out_attempt;
static int out_timeout = 0;

// periodic out
#define PER_OUT_SIZE 28
#define PER_OUT_TIMEOUT 100

static struct libusb_transfer *per_out_transfer = NULL;
static unsigned char per_out_buf[PER_OUT_SIZE];
static int per_out_ready;

// data in
#define BUF_NOTHING 	0
#define BUF_READING 	1
#define BUF_CANCELLING 	2
#define BUF_DONE	3
#define BUF_WRITING 	4

struct buf_t {
	int buf_state;							// state of this buffer
	struct libusb_transfer *usb_transfer;	// data in from usb
	unsigned char buf[BUF_SIZE];			// buffers
	int len;								// length of valid data
	int f;									// file (0 or 1)
	struct aiocb *disk_transfer;			// aio write to file
};

struct buf_t d[BUF_NO];

#define F_NOTHING 0
#define F_WRITING 1
#define F_CLOSING 2

struct file_t {
	int state;				// state
	char name_tmp[80];		// name (while writing)
	char name[80];			// name
	int offset;				// write offset
	int desc;				// file descriptor
};

struct file_t f[2];
	
static int read_bytes;
static int written_bytes;
static int nr;				// nothing -> reading
static int rw; 				// reading_done -> writing
static int wn; 				// writing_done -> nothing

// second timers
#define TIMER_NO 4
#define T_TIMESTAMP	0
#define T_FILE_CHANGE 1
#define T_RESPAWN 2
#define T_DIAG 3

static long int otimers[TIMER_NO];
static long int time_lastopen[100];

static int key_base = 10000;
static long int latency = 20;
static char run_number[100]="";
static int disk_number = 1; //default disk number
static char directory[300];
static char target[300];
static char target_dir[300]= "/home/nfs/icarus/DAQ_DevAreas/DAQ_12Dec2022_rhowell/srcs/sbndaq_artdaq/sbndaq-artdaq/Generators/ICARUS/BottomInterface/ICARUS_DAQ/readout/data1/OVDAQ/DATA"; //default data folder
static char target_path[300]= "OVDAQ/DATA/"; // default relative path
static char baseline[300];
static char decoded[200];
static char val[BUFSIZE];
static char val1[200];
static int inhibit_writetofile = 1;
static int inhibit_writetogaibu = 1;  // set this to 1 to turn off gaibu or 1 to turn on gaibu
// timers ----------------------------------------------------------------------

static void init_timers() {
	int i;
	for(i = 0; i < TIMER_NO; i++) otimers[i] = time(0);
	for(i = 0; i < 100; i++) time_lastopen[i] = -1;
}

static int timer_zero(int t_index) {
	return (time(0) >= otimers[t_index]);
}

static void timer_reset(int t_index, long int t_timeout) {
	otimers[t_index] = time(0) + t_timeout;
}

static int timer_zero_reset(int t_index, long int t_timeout) {
	int t = time(0);
	
	if (t >= otimers[t_index]) {
		otimers[t_index] = t + t_timeout;
		return 1;
	}
	return 0;		
}

// debugging -------------------------------------------------------------------

static void send_message() {
	msg_buf.mtype = 1;		// any number > 0
	msgsnd(msq_common, &msg_buf, strlen(msg_buf.mdata), IPC_NOWAIT);
}

static int ot = 0;

static void debug() {
	int i;
	char gaibu_debug_msg[BUFSIZE];
	int t = time(0);
	if (t != ot) { // So you don't get debug info until next unix timestamp second
		if(ot==0) { ot = t; return; }
		ot = t;
		/*
		printf("s=%d\t", ser_no);
		
		for (i = 0; i < BUF_NO; i++) printf("%d", d[i].buf_state);
		printf("\tnr=%d\t", nr);
		printf("rw=%d\t", rw);
		printf("wn=%d\t", wn);
		printf("r=%d\t", read_bytes); read_bytes = 0;
		printf("w=%d\t", written_bytes); written_bytes = 0;
		*/

		if (diag_result & 1) {
			printf("resending: %d\n",ser_no); 
			sprintf(gaibu_debug_msg,"resending: %d",ser_no);
			gaibu_msg(MWARNING, gaibu_debug_msg);
		}

		//
		if (diag_result & 2) {
			printf("usb_full: %d\n",ser_no);
			sprintf(gaibu_debug_msg,"usb_full: %d",ser_no);
			gaibu_msg(MEXCEPTION, gaibu_debug_msg);
		}
		if (diag_result & 4) {
			printf("pmt_full: %d\n",ser_no);
			sprintf(gaibu_debug_msg,"pmt_full: %d",ser_no);
			gaibu_msg(MEXCEPTION, gaibu_debug_msg);
		}
		if (!(diag_result & 8)) {
			printf("no_lock: %d\n",ser_no);
			sprintf(gaibu_debug_msg,"no_lock: %d",ser_no);
			gaibu_msg(MEXCEPTION, gaibu_debug_msg);
		}
		if (!(diag_result & 16)) {
			printf("no_clk: %d\n",ser_no);
			sprintf(gaibu_debug_msg,"no_clk: %d",ser_no);
			gaibu_msg(MEXCEPTION, gaibu_debug_msg);
		}
		//

		//if (diag_result & 256) printf("auto_token\t");
		//if (diag_result & 512) printf("near_debug\t");
		//if (diag_result & 1024) printf("far_debug\t");
		//if (diag_result & 2048) printf("auto_resend\t");
	}
}

// diag in ---------------------------------------------------------------------

// diag in callback
static void cb_diag_in(struct libusb_transfer *transfer) {
	int i;
	unsigned char data, type;
	int len;
	
	switch(transfer->status) {
		case LIBUSB_TRANSFER_COMPLETED:
		case LIBUSB_TRANSFER_TIMED_OUT:
			len = transfer->actual_length;
			break;
		default:
			printf("%d diag in error %d\n", ser_no, transfer->status); kill_me = 1;
			len = 0;
			break;
	}

	// parse in buffer
	for (i = 0; i < len; i++) {
		type = transfer->buffer[i] >> 6;
		data = transfer->buffer[i] & 63;
		
		if (type == 0) {
			diag_expecting = 1;
			diag_word = data;
		} else {
			if (type == diag_expecting) {
				diag_word = (diag_word << 6) | data;
				if (++diag_expecting == 4) {
					diag_expecting = 0;
					diag_result = diag_word;
				}
			} else {
				diag_expecting = 0;
			}
		}
	}
	// resubmit
	diag_in_ready = 1;
}

static void check_diag_in() {
	int len;

	// diag in submit
	if (!diag_in_ready) return;
	
	// fill transfer
	libusb_fill_bulk_transfer(
	 	diag_in_transfer, 		// transfer
	 	devh, 					// device handle
	 	EP_DIAG_IN, 			// endpoint
	 	diag_in_buf,			// buffer
		sizeof(diag_in_buf), 	// buffer size
	 	cb_diag_in, 			// callback
	 	NULL, 					// user data
	 	COM_IN_TIMEOUT			// timeout
	);

	// submit transfers
	int r = libusb_submit_transfer(diag_in_transfer);
	if (r < 0) {
		printf("com in: submit error\n"); kill_me = 1;
	} else {
		diag_in_ready = 0;
	}
}

// data in	--------------------------------------------------------------------

static void init_buffers() {
	int i;
	
	for (i = 0; i < BUF_NO; i++) {
		d[i].buf_state = BUF_NOTHING;
		d[i].usb_transfer = NULL;
		d[i].disk_transfer = NULL;
		d[i].f = -1;
	}
}

static void init_files() {
	int i;
	
	for (i = 0; i < 2; i++) {
		f[i].state = F_NOTHING;
		f[i].desc = -1;
		f[i].offset = 0;
	}
}

static void read_data_in();

// data in callback
static void cb_data_in(struct libusb_transfer *transfer)
{
	struct buf_t *p = transfer->user_data;
	
	//printf("read cb i=%d\n", p->index);
	switch(transfer->status) {
		case LIBUSB_TRANSFER_COMPLETED:
		case LIBUSB_TRANSFER_TIMED_OUT:
			p->len = transfer->actual_length;
			break;
		default:
			printf("%d data in error %d\n", ser_no, transfer->status); 
			kill_me = 1;
			p->len = 0;
			break;
	}
	p->buf_state = BUF_DONE;
}

// try to open new file (0 or 1) returns success
int try_to_open(int file) {


  long int t = time(0);
  
  //check the time elapsed between switching between two consecutive files for a particular USB
  if(file<2) {
    if (f[file].state != F_NOTHING) return 0;
    if(time_lastopen[ser_no] != -1){ // check that we are not just starting or we are not restarting
      if( (t - time_lastopen[ser_no]) % latency != 0){
	//Gaibu Alert
	sprintf(gaibu_msg_buf,"Error naming file for USB %d, time now=%ld, time_pred=%ld, get auto name",ser_no,time(0),time_lastopen[ser_no]+latency);
	gaibu_msg(MNOTICE, gaibu_msg_buf);
//	t = time_lastopen[ser_no] + latency;
        time_lastopen[ser_no] = t;	

      }
    }
     sprintf(f[file].name, "%s/%ld_%d", target, t, ser_no);
     sprintf(f[file].name_tmp, "%s/%ld_%d.wr", target, t, ser_no);
     time_lastopen[ser_no] = t;
  }

  /*  if(file<2){
    if (f[file].state != F_NOTHING) return 0;
    sprintf(f[file].name, "%s/%ld_%d", target, t, ser_no);
    sprintf(f[file].name_tmp, "%s/%ld_%d.wr", target, t, ser_no);
    time_lastopen[ser_no] = t;
    }*/
  else
    {
      //for baseline only
      //sprintf(f[0].name, "%s/%d_%d", baseline, t, ser_no);
      sprintf(f[0].name, "%s/baseline_%d", target, ser_no);
      sprintf(f[0].name_tmp, "%s/baseline_%d.wr", target, ser_no);
      file = 0;
    }

  //
  
  int fd = creat(f[file].name_tmp, 0666);

  if (fd < 0) {
    printf("error creating file %s\n", f[file].name_tmp);
    kill_me = 1;
    //Gaibu Alert
    sprintf(gaibu_msg_buf,"Error creating file for USB %d",ser_no);
    gaibu_msg(MNOTICE, gaibu_msg_buf);
    return 0;
  }
  
  f[file].desc = fd;
  f[file].offset = 0;
  f[file].state = F_WRITING;
  return 1;
}

// try to close a file (0 or 1)
void try_to_close(int file) {
  int res, j;
  
  if (f[file].state != F_CLOSING) return;
  for (j = 0; j < BUF_NO; j++)
    if ((d[j].buf_state == BUF_WRITING) && (d[j].f == file)) return;
  
  res = close(f[file].desc);
  if (res) {
    printf("closing file error %d\n", errno); 
    kill_me = 1; 
    //Gaibu Alert
    char mymsg[BUFSIZE];
    sprintf(mymsg,"Error closing file for USB %d",ser_no);
    gaibu_msg(MNOTICE, mymsg);
    return;
  }
  
  res = rename(f[file].name_tmp, f[file].name);
  if (res) { 
        char mymsg[BUFSIZE];
        sprintf(mymsg,"Error cound not rename file for USB %d",ser_no);
        gaibu_msg(MNOTICE, mymsg);
	perror("couldn't rename file\n"); 
	}
  f[file].state = F_NOTHING;
}

// data in polling
static void check_data_in() {


  int i, len, tmp, active = -1;
  char mymsg[BUFSIZE];
  
  // open first file
  for (i = 0; i < 2; i++) if (f[i].state == F_WRITING) active = i; 
  
  
  // check latency message
  len = msgrcv(msqid, &msg_buf, MSG_BUF_SIZE-1, MSG_LATENCY,
	       IPC_NOWAIT | MSG_NOERROR);
  if (len >= 0) {
    tmp = *((int *) msg_buf.mdata);
    if (tmp >= 1) { 
      latency = tmp;                                           //change latency from perl script
      //printf("got latency %d\n", tmp);
    }
  }
  
  // Get run number
  len = msgrcv(msqid, &msg_buf, MSG_BUF_SIZE-1, MSG_RUN_NUMBER,
	       IPC_NOWAIT | MSG_NOERROR);
  
  if (len >= 0) {
	  sprintf(run_number,"%s",msg_buf.mdata);
	  printf("Run Number: %s\n",run_number);
  }

  // Get path folder
  len = msgrcv(msqid, &msg_buf, MSG_BUF_SIZE-1, MSG_DATA_PATH,
	       IPC_NOWAIT | MSG_NOERROR);
  
  if (len >= 0) {
	  sprintf(target_path,"%s",msg_buf.mdata);
	  printf("Target Path: %s\n",target_path);
  }


  //get data disk
  len = msgrcv(msqid, &msg_buf, MSG_BUF_SIZE-1, MSG_DATA_DISK,
	       IPC_NOWAIT | MSG_NOERROR);
  
  if (len >= 0) {
    disk_number = *((int *) msg_buf.mdata);
    //sprintf(target_dir,"/data%d/OVDAQ/DATA",disk_number);
    sprintf(target_dir,"../data%d%s",disk_number,target_path);
    printf("Targer Directory: %s\n",target_dir);
  }
  
  // Get inhibit
  len = msgrcv(msqid, &msg_buf, MSG_BUF_SIZE-1, MSG_INHIBIT,
	       IPC_NOWAIT | MSG_NOERROR);
  if (len >= 0) {
    time_lastopen[ser_no] = -1; //when getting inhibit message allow to reset the time in the filename
    tmp = *((int *) msg_buf.mdata);
    printf("Inhibit: %d\n",tmp);
    //normal starting will be data_flag = 1 suppress write data to disk
    
    if(tmp == 0) { 
      inhibit_writetofile = 0; 
      printf("USB %d: releasing inhibit\n",ser_no);
      /* Alert Gaibu Server */
      sprintf(mymsg,"USB %d: releasing inhibit",ser_no);
      gaibu_msg(MNOTICE, mymsg);
      
    }    //normal data taking -- write to disk enabled
    
    else if(tmp == -1) {
      inhibit_writetofile = 1;  //suppress write data to disk  
      printf("USB %d: inhibiting writing to file\n",ser_no);
      /* Alert Gaibu Server */
      sprintf(mymsg,"USB %d: inhibiting writing to file",ser_no);
      gaibu_msg(MNOTICE, mymsg);
      
      //f[active].state = F_CLOSING;   //move state to close and close files
    }
    else if(tmp == -2) {  // to take baseline
      inhibit_writetofile = 0;
      printf("USB %d: releasing inhibit for baseline\n",ser_no);
      
      /* Alert Gaibu Server */
      sprintf(mymsg,"USB %d: releasing inhibit for baseline",ser_no);
      gaibu_msg(MNOTICE, mymsg);
      
      try_to_open(2);
      timer_reset(T_FILE_CHANGE, 100);
      return;
    }
    else if(tmp == -3) {  // to take baseline
      time_t rawtime;
      struct tm * timeinfo;
      
      time ( &rawtime );
      timeinfo = localtime ( &rawtime );
      
      printf("Run number to be assigned: %s\n",run_number);
      printf("Data folder: %s\n",target_dir);
      //read the run number from input line
      if( strcmp(run_number,"") == 0 ) { //automatic run number is assigned
		  sprintf(directory,"%s/Run_%0.4d%0.2d%0.2d%0.2d%0.2d",target_dir,
				  1900+timeinfo->tm_year,(1+timeinfo->tm_mon),timeinfo->tm_mday,
				  timeinfo->tm_hour,timeinfo->tm_min);
      }
      else {  //get run number from input line
		  sprintf(directory,"%s/Run_%s",target_dir,run_number);
      }
	    
      sprintf(target,"%s/binary",directory);
      //sprintf(baseline,"%s/baseline",directory);
      sprintf(decoded,"%s/decoded",directory);

      umask(0);
      mkdir(directory, 0777);
      mkdir(target, 0777);
      //mkdir(baseline, 0777);
      mkdir(decoded, 0777);
      return;
    }
  }


  if( inhibit_writetofile == 1){
    return;
  }
  
  
  //
  
  if (active == -1) { 
    try_to_open(0);
    timer_reset(T_FILE_CHANGE, latency);
    return;
  }
  
  
  // do we have to switch and write to new file?
  int other = 1 - active;  //jump between the two file state 0->1 and viceversa
  if (timer_zero(T_FILE_CHANGE)) {
    if(try_to_open(other)) {
      f[active].state = F_CLOSING;
      timer_reset(T_FILE_CHANGE, latency);
    }
  }
	  
}


// data in state machine
static void read_data_in() {
  int i, r, len, active, start_up;
  
#if 1
  // sometimes a transfer doesn't complete
  if (d[rw].buf_state == BUF_READING) {
    int total = 0;
    for (i = 1; i < BUF_NO; i++) {
      int j = (rw + i) % BUF_NO;
      if (d[j].buf_state == BUF_DONE) total++;
    }
    if (total >= 1) {
      r = libusb_cancel_transfer(d[rw].usb_transfer);
      if (r < 0){ 
	printf("error cancelling data in\n");
	//Gaibu Alert
	sprintf(gaibu_msg_buf,"Error deleting data IN for USB %d",ser_no);
	gaibu_msg(MNOTICE, gaibu_msg_buf);
      }
      else d[rw].buf_state = BUF_CANCELLING;
    }
  }
#endif
  
  // reading -> writing
  while (d[rw].buf_state == BUF_DONE) {
    len = d[rw].len;
    read_bytes += len;
    
    // write to disk
    if (len) {
      active = -1;
      for (i = 0; i < 2; i++) if (f[i].state == F_WRITING) active = i;
      if (active == -1) {
	//CM&MD 09142009				
	if( inhibit_writetofile == 0) {
	  printf("no file to write to\n"); 
	  //Gaibu Alert
	  sprintf(gaibu_msg_buf,"Error:no file to write to for USB %d",ser_no);
	  gaibu_msg(MNOTICE, gaibu_msg_buf);
	  kill_me = 1;   // not to write to a file depending on flag
	  //d[rw].buf_state = BUF_NOTHING;
	}
	//printf("no file to write to\n"); 
	//kill_me = 1;
	d[rw].buf_state = BUF_NOTHING;
      } 
      else {
			
	// set up aio request
	struct aiocb *p = d[rw].disk_transfer;
	bzero((char *) p, sizeof(struct aiocb));
	p->aio_fildes = f[active].desc;
	d[rw].f = active;
	p->aio_buf = d[rw].buf;
	p->aio_nbytes = d[rw].len;
	p->aio_offset = f[active].offset;
	
	/*
	// callback
	p->aio_sigevent.sigev_notify = SIGEV_THREAD;
	p->aio_sigevent.sigev_notify_function = aio_cb;
	p->aio_sigevent.sigev_notify_attributes = NULL;
	p->aio_sigevent.sigev_value.sival_ptr = p;
	*/
			
	int ret = aio_write(p);
	if (ret) {
	  printf("aio write submit error: %d\n", errno); 
	  //Gaibu Alert
	  sprintf(gaibu_msg_buf,"aio write submit error for USB %d",ser_no);
	  gaibu_msg(MNOTICE, gaibu_msg_buf);
	  kill_me = 1;
	  d[rw].buf_state = BUF_NOTHING;
	} else {
	  f[active].offset += d[rw].len;
	  d[rw].buf_state = BUF_WRITING;
	}
      }
    } else {
      d[rw].buf_state = BUF_NOTHING;
      //Gaibu Alert
      //sprintf(gaibu_msg_buf,"No data read for USB %d",ser_no);
      //gaibu_msg(MNOTICE, gaibu_msg_buf);
    }
    rw = (rw + 1) % BUF_NO;
  }
  
  // writing -> nothing
  while (1) {
    if (wn == rw) break;
    if (d[wn].buf_state != BUF_WRITING) {
      wn = (wn + 1) % BUF_NO;
      continue;
    }
    
    switch (aio_error(d[wn].disk_transfer)) {
    case ECANCELED:
    case 0:
      // success
      r = aio_return(d[wn].disk_transfer);
      if (r > 0) {
	written_bytes += r;
	//printf("aio cb %d\n", i);
      } else {
	// oops
	printf("aio error1 %d\n", errno);
	//Gaibu Alert
	sprintf(gaibu_msg_buf,"aio error1 %d",errno);
	gaibu_msg(MNOTICE, gaibu_msg_buf);
	kill_me = 1; //// for now
      }
      
      d[wn].buf_state = BUF_NOTHING;
      d[wn].f = -1;
      wn = (wn + 1) % BUF_NO;
      break;
      
    case EINPROGRESS:
      break;
      
    case -1:
    default:
      // oops
      printf("aio error2 %d\n", errno);
      //Gaibu Alert
      sprintf(gaibu_msg_buf,"aio error2 %d",errno);
      gaibu_msg(MNOTICE, gaibu_msg_buf);
      kill_me = 1; //// for now
      break;
    }
  }
  
    //
  active = -1;
  for (i = 0; i < 2; i++) {
    if (f[i].state == F_WRITING) { active = i; }
  }
  if( active >=0 && inhibit_writetofile == 1 ) {f[active].state = F_CLOSING;}

  try_to_close(0);  //try to close if the state is closing
  try_to_close(1);  //same as above
  
  // nothing -> reading
  while (d[nr].buf_state == BUF_NOTHING) { 
		libusb_fill_bulk_transfer(
					  d[nr].usb_transfer,			// transfer
					  devh, 				// device handle
					  EP_DATA_IN, 				// endpoint
					  d[nr].buf,				// buffer
					  BUF_SIZE,				// buffer size
					  cb_data_in, 				// callback
					  &(d[nr]),				// user data
					  DATA_IN_TIMEOUT          	        // timeout
					  );
		int r = libusb_submit_transfer(d[nr].usb_transfer);
		if (r < 0) {
		  printf("data in: submit error\n"); 
		  //Gaibu Alert
		  sprintf(gaibu_msg_buf,"data in submit error for USB %d",ser_no);
		  gaibu_msg(MNOTICE, gaibu_msg_buf);
		  kill_me = 1;
		  break;		// keep trying?
		} else {
		  d[nr].buf_state = BUF_READING;
		  nr = (nr + 1) % BUF_NO;
		}
  }
}

// com out ---------------------------------------------------------------------

static void expand(unsigned char *dest, unsigned int *src, int len) {
  unsigned int t;
  
  while(len > 0) {
    t = *src++;  
    *dest++ =  (t >> 18) & 63;
    *dest++ = ((t >> 12) & 63) |  64;
    *dest++ = ((t >>  6) & 63) | 128;
    *dest++ =  (t        & 63) | 192;
    len -= 4;
  }
}

// com out callback
static void cb_com_out(struct libusb_transfer *transfer) {
  
  //printf("com out callback\n");
  if (transfer->status == LIBUSB_TRANSFER_COMPLETED) {
    out_timeout = 0;
    com_out_attempt = 0;
  } else {
    out_timeout = 1;
    printf("com out transfer status %d?\n", transfer->status); 
    //Gaibu Alert
    sprintf(gaibu_msg_buf,"com out transfer status %d", transfer->status);
    gaibu_msg(MNOTICE, gaibu_msg_buf);
    kill_me = 1;
  }
  com_out_ready = 1;
}

// com out polling
static void check_com_out() {
  int i, r, len;
	
  if (!com_out_ready) return;
  
  // resubmit old transfer?
  if (com_out_attempt) {
    com_out_attempt--;
    goto cco_resubmit;
  }
  
  // read com out message
  len = msgrcv(msqid, &msg_buf, MSG_BUF_SIZE, MSG_COM_OUT, 
	       IPC_NOWAIT | MSG_NOERROR);
  if (len > 0) {
    // debug
#if 0
    printf("com out: type = %ld, data = ", msg_buf.mtype);
    for (i = 0; i < len; i++) printf("%d ", msg_buf.mdata[i]);
    printf("\n");
#endif
    // sanitize len
    if (len > COM_OUT_SIZE) len = COM_OUT_SIZE;
    len -= (len & 3);
    
    // fill transfer
    libusb_fill_bulk_transfer(
			      com_out_transfer, 			// transfer
			      devh, 					// device handle
			      EP_COM_OUT, 				// endpoint
			      com_out_buf,				// buffer
			      len,				 	// buffer size
			      cb_com_out, 				// callback
			      NULL, 					// user data
			      COM_OUT_TIMEOUT				// timeout
			      );

    // copy/expand data
    expand(com_out_transfer->buffer, (unsigned int *) msg_buf.mdata, len);
    
    // try this many times
    com_out_attempt = MAX_ATTEMPTS;
  }
  
  // submit transfer
 cco_resubmit:
  if (com_out_attempt) {
    r = libusb_submit_transfer(com_out_transfer);
    if (r < 0) {
      printf("com out: submit error\n"); 
      //Gaibu Alert
      sprintf(gaibu_msg_buf,"com out: submit error for USB %d", ser_no);
      gaibu_msg(MNOTICE, gaibu_msg_buf);
      kill_me = 1;
    } else {
      com_out_ready = 0;
    }
  }
}

// periodic out callback
static void cb_per_out(struct libusb_transfer *transfer) { 
	per_out_ready = 1; 
	if (transfer->status == LIBUSB_TRANSFER_COMPLETED) 
	  out_timeout = 0;
	else
	  out_timeout = 1;
}

// periodic out
static void check_per_out() {
  int i, r, len;
  unsigned int *p;
  unsigned int t;
  
  if (!per_out_ready) return;
  if (!timer_zero_reset(T_TIMESTAMP, 1)) return;
  
  // fill transfer
  libusb_fill_bulk_transfer(
			    per_out_transfer, 			// transfer
			    devh, 				// device handle
			    EP_COM_OUT, 			// endpoint
			    per_out_buf,			// buffer
			    PER_OUT_SIZE,		 	// buffer size
			    cb_per_out, 			// callback
			    NULL, 				// user data
			    PER_OUT_TIMEOUT			// timeout
			    );
  
  // setup data
  t = time(0);
  p = (unsigned int *) per_out_buf;
  p[0] = 0xc80000 | ((t >> 16) & 0xffff);
  p[1] = 0xc90000 |  (t        & 0xffff);
  p[2] = 0xc50000;
  p[3] = 0xc60000;
  p[4] = 0xc60000;
  p[5] = 0xc60000;
  p[6] = 0xc60000;
  expand(per_out_buf, p, PER_OUT_SIZE);
  
  r = libusb_submit_transfer(per_out_transfer);
  if (r < 0) {
    printf("per out: submit error\n"); 
    //Gaibu Alert
    sprintf(gaibu_msg_buf,"per out: submit error for USB");
    gaibu_msg(MNOTICE, gaibu_msg_buf);
    kill_me = 1;
  } else {
    per_out_ready = 0;
  }
}

// log files -------------------------------------------------------------------
static char out_log[200];
static char err_log[200];

static void open_log_files() {
  FILE *fd_out;
  FILE *fd_err;
  
  fd_out = freopen(out_log, "w", stdout);
  fd_err = freopen(err_log, "a", stderr);
  setvbuf(fd_out, (char *)NULL, _IONBF, 1024);
  setvbuf(fd_err, (char *)NULL, _IOLBF, 1024);
}

//get the configuration from the DCSpaceIP.config file --

// read IP configuration from file

char * config_string(const char* path, const char* key)
{ 
  char ch[300];
  char name[200];
  
  char IP[200];
  char port[200];
  
  FILE * file;
  file = fopen(path,"r");
  if (file != NULL) {
    while( fgets( ch, 300, file ) != NULL ) {
      if (sscanf(ch, "%s :%s", &name, &IP) == 2) {// getting the IP/hostname 
		  if (  strcmp(name, key) == 0 ) { 
			  sprintf(val,IP);
			  fclose(file);
			  return val; 
		  } 
      }
      else if (sscanf(ch, "%s %s", &name, &port) == 2){// getting the port number
		  if ( strcmp(name, key) == 0 ) { 
			  sprintf(val,port);
			  fclose(file);
			  return val; 
		  }
      }
      
    }
    fclose(file);
  }
  else { 
	  printf("Unable to open %s\n",path);
	  exit(1);
  }
}


int config_int(const char* path, const char* key)
{ 
	return atoi(config_string(path,key));
}


void get_gaibu_config() {

	char spaceIP_path[BUFSIZE];
	sprintf(spaceIP_path,"%s/config/DCSpaceIP.config",getenv("DCONLINE_PATH"));

	sprintf(GAIBU_SERVER_IP,"%s",config_string(spaceIP_path,"DCGAIBU_IP"));
	printf("Gaibu ip: %s\n",GAIBU_SERVER_IP);
	
	char spacePort_path[BUFSIZE];
	sprintf(spacePort_path,"%s/config/DCSpacePort.config",getenv("DCONLINE_PATH"));
	GAIBU_PORTNUMBER = config_int(spacePort_path,"DCGAIBU_PORT");
	printf("Gaibu port: %d\n",GAIBU_PORTNUMBER);

}

int initialize_gaibu() {

	int m;
    char msg_buf_local[BUFSIZE];

	gaibu_sockfd = init_socket(&gaibu_serveraddr,GAIBU_SERVER_IP,GAIBU_PORTNUMBER);
	if(gaibu_sockfd>0) {
		if (connect(gaibu_sockfd,(struct sockaddr *) &gaibu_serveraddr, sizeof(gaibu_serveraddr)) < 0) {
			gaibu_sockfd = -1;
			sprintf(msg_buf_local,"Error connecting to Gaibu server");
			syslog(LOG_ERR, msg_buf_local);
			printf("Error connecting to Gaibu server\n");
		} else {
			syslog(LOG_NOTICE,"DC OV DAQ connected to gaibu server\n");
			sprintf(msg_buf_local,"DCFOVDAQ\n"); 
			
			m = send(gaibu_sockfd, msg_buf_local, strlen(msg_buf_local),MSG_NOSIGNAL);
			if (m < 0) {
				syslog(LOG_ERR,"ERROR writing to socket\n");
				printf("ERROR writing to socket\n");
				gaibu_sockfd = -1;
				//return -1;
			}
		}
	}

}

/////////////////////////////////////////////////////////////////////////////////


int gaibu_msg(int priority, char *gaibu_buf) 
{
	int m;
    char msg_buf_local[BUFSIZE];
	if(inhibit_writetogaibu != 1) {
		if(gaibu_sockfd < 0) {
			syslog(LOG_ERR,"ERROR writing to socket\n");
			printf("ERROR writing to socket\n");
			//return -1;
		}
		else {
			sprintf(msg_buf_local,"%d  %s\n",priority,gaibu_buf); //composing gaibu's message
			
			m = send(gaibu_sockfd, msg_buf_local, strlen(msg_buf_local),MSG_NOSIGNAL);
			
			if (m < 0) {
				syslog(LOG_ERR,"ERROR writing to socket\n");
				printf("ERROR writing to socket\n");
				gaibu_sockfd = -1;
			}
		}
	} // closing bracket for if(!inhibit_writetogaibu)

	if(priority > 1) {
		if(priority == 2) { syslog(LOG_NOTICE, gaibu_buf); }
		else if(priority == 3) { syslog(LOG_WARNING, gaibu_buf); }
		else if(priority == 4) { syslog(LOG_ERR, gaibu_buf); }
		else { syslog(LOG_CRIT, gaibu_buf); }
	}

	return 0;
}

int init_socket(struct sockaddr_in *serveraddr, char *hostname, int portno)
{
	int sockfd;
	struct hostent *server;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
		printf("ERROR opening socket");
        //error("ERROR opening socket");
	}
	
	/* gethostbyname: get the server's DNS entry */
	server = gethostbyname(hostname);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host as %s\n", hostname);
        exit(0);
    }
	
    /* build the server's Internet address */
	bzero((char *) &(*serveraddr), sizeof((*serveraddr)));
	(*serveraddr).sin_family = AF_INET;
    //memcpy(&(*serveraddr)->sin_addr,server->h_addr,server->h_length);
    bcopy((char *)server->h_addr, 
	  (char *)&(*serveraddr).sin_addr.s_addr, server->h_length);

    (*serveraddr).sin_port = htons(portno);
	return sockfd;
}
//
/////////////////////////////////////////////////////////////////////////////////
// main ------------------------------------------------------------------------
////////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv)
{
	int n, r, i, j, k, index, len, usb_counter;
	key_t key;
	int show_help = 0;
	char c;
	char log_dir[200];


	sprintf(log_dir,"%s/DCOV/log",getenv("DCONLINE_PATH"));
	openlog("OV DAQ Event", LOG_NDELAY, LOG_USER);
	
	// parse command line parameters
	while ((c = getopt (argc, argv, "d:r:g:")) != -1) {
    	switch (c) {
		case 'g': inhibit_writetogaibu = atoi(optarg);	break;
		case 'd': sprintf(target_dir,"../data%d/%s",disk_number,optarg); strcpy(target_path,optarg);	break;
		case 'r': strcpy(run_number,optarg);  break;
		default:  show_help = 1;			break;
		}
	}
	if (optind < argc) show_help = 1;

    //printf("lat=%d, key=%d, tar dir=%s run=%d\n", latency, key_base, target_dir, run_number);

	for (index = optind; index < argc; index++) { 
		printf ("Non-option argument %s\n", argv[index]);
		printf("lat=%d, key=%d, tar dire=%s run=%s\n", latency, key_base, target, run_number);
        }


	if (latency < 1) latency = 1;
	
	if (show_help) {
		printf("Usage: %s [-l <seconds>] [-m <key_base>] [-d <directory>]\n", argv[0]);
		printf("-g : inhibit writing to gaibu server (0/1) [default: 0]\n");
		printf("-r : set run number for data        [default: YYYYMMDD_hh_mm]\n");
		printf("-d : where to write output files in real-time mode [default: ./binary]\n");
		exit(0);
	}

	gaibu_sockfd = -1;
	if(inhibit_writetogaibu != 1) { 
		get_gaibu_config(); 
		initialize_gaibu();
	}

	//mkdir(target, 0777);
	//mkdir(baseline, 0777);
	mkdir(log_dir, 0777);
	
	// set up common message queue
	key = key_base - 1;
	r = msgget(key, IPC_CREAT | 0666);
	if (r < 0) {
		printf("could not open common message queue %d\n", key);
		//Gaibu Alert
		sprintf(gaibu_msg_buf,"could not open common message queue %d",key);
		gaibu_msg(MNOTICE, gaibu_msg_buf);
		goto m_return;
	}
	msq_common = r;

	// memory leak detection (set MALLOC_TRACE env var)
	//mtrace();

#if DAEMONIZE
	// daemonize
	pid_t pid, sid;
	pid = fork();
	if (pid < 0) { perror("fork"); exit(errno); }
	if (pid > 0) exit(0);
	sid = setsid();
	if (sid < 0) { printf("sid"); exit(errno); }
	umask(0);
	freopen("/dev/null", "a", stdin);
	sprintf(out_log, "%s/usb_main_out.log",log_dir);
	sprintf(err_log, "%s/usb_main_err.log",log_dir);
	open_log_files();    //open just 1 log file fo the main process
	sprintf(gaibu_msg_buf,"OV DAQ Process Started");
	gaibu_msg(MNOTICE,gaibu_msg_buf);
#endif
	// install our own signal handler
	struct sigaction sigact;
	
	sigact.sa_handler = sighandler;
	sigemptyset(&sigact.sa_mask);
	sigact.sa_flags = 0;
	sigaction(SIGINT, &sigact, NULL);
	sigaction(SIGTERM, &sigact, NULL);
	sigaction(SIGQUIT, &sigact, NULL);
	//

start_over:
	if(inhibit_writetogaibu != 1) { 
		if(gaibu_sockfd<0) {
			initialize_gaibu();
		}
	}

	// initialise libusb
	r = libusb_init(&ctx);
	if (r < 0) {
		printf("failed to initialise libusb\n");
		//Gaibu Alert
		sprintf(gaibu_msg_buf,"failed to initialize libusb");
		gaibu_msg(MNOTICE, gaibu_msg_buf);
		goto m_return;
	}
	libusb_set_debug(ctx,3); // 0-nothing 1-error 2-warning 3-info

	// get device list
	r = libusb_get_device_list(ctx,&devs);
	if (r < 0) {
		printf("could not get list of devices\n");
		//Gaibu Alert
		sprintf(gaibu_msg_buf,"could not get device's list");
		gaibu_msg(MNOTICE, gaibu_msg_buf);
		goto m_deinit;
	}

	// find readout devices
	i = 0;
	usb_counter = 0;
	while ((dev = devs[i++]) != NULL) {
		struct libusb_device_descriptor desc;
		libusb_get_device_descriptor(dev, &desc);
		//printf("%04x:%04x\n", desc.idVendor, desc.idProduct); // CM
		if (desc.idVendor != 0x04b4 || desc.idProduct != 0xdead) continue;
		// open device
		r = libusb_open(dev, &devh);
		if (r < 0) {
			printf("could not open device\n");
			//Gaibu Alert
			sprintf(gaibu_msg_buf,"Could not open device");
			gaibu_msg(MNOTICE,gaibu_msg_buf);
                        usbsystemdevice--;
			continue;
		}
		
		// claim interface
		r = libusb_claim_interface(devh, 0);
		//printf("r=%d\n"); //CM
		if (r < 0) {
			usb_counter++;
			//printf("usb_claim_interface error %d %s\n", r, strerror(-r));
			goto m_find_close;
		}

		// device and its serial number
		j = desc.iSerialNumber;
		if(j == 0) {
			printf("Found USB readout board with no serial number\n");
			//Gaibu Alert
			sprintf(gaibu_msg_buf,"Found USB readout board without any serial number");
			gaibu_msg(MNOTICE, gaibu_msg_buf);
			goto m_find_release;
		}

		// read serial number
		unsigned char data[64];
		r = libusb_get_string_descriptor_ascii(
				devh, 		// handle
				j,		// descriptor index
				data,		// buffer
				sizeof(data)	// length
			);
		if (r < 0) {
			printf("when getting serial number: error %d\n", r);
			//Gaibu Alert
			sprintf(gaibu_msg_buf,"Getting serial number problem number: %d",r);
			gaibu_msg(MNOTICE, gaibu_msg_buf);
			goto m_find_release;
		}

		len = (unsigned int) r;
		ser_no = 0;
		for (k = 0; k < len; k++) {
		  ser_no = 10 * ser_no + (data[k] - '0');
		}
		printf("Found USB readout board serial #%d\n", ser_no);
		usbsystemdevice++;
		// set up message queue for given thread
		key = key_base + ser_no;
		r = msgget(key, IPC_CREAT | 0666);
		if (r < 0) {
		  printf("Com out: could not open message queue %d\n", key);
		  goto m_find_release;
		}
		msqid = r;  //one for each serial number
		
		// fork off a process to handle this device
		pid_t pid = fork();
		if (pid < 0) {			// failed to fork
		  printf("Failed to fork usb %d\n", ser_no);
		  goto m_find_release;
		  } else if (pid == 0) {	// child
		  goto mh_init;
		}
		// parent
		goto start_over;
		
	m_find_release:
		libusb_release_interface(devh, 0);
		usbsystemdevice--;
	m_find_close:
		libusb_release_interface(devh, 0);
		libusb_close(devh);
	}
	
	// wait a little then try again
	while (!kill_me) {
	  //     	      sprintf(gaibu_msg_buf,"Number of USBs connected = %d(%d)",usb_counter,usbsystemdevice);
	  //    gaibu_msg(MERROR,gaibu_msg_buf);
	  if (timer_zero_reset(T_RESPAWN, 5)) {
	    if(usbsystemdevice%usb_counter!=0) {  
	      printf("Error! Number of USBs connected = %d(%d)\n",usb_counter, usbsystemdevice);
	      sprintf(gaibu_msg_buf,"Number of USBs connected = %d(%d)",usb_counter,usbsystemdevice);
	      gaibu_msg(MERROR,gaibu_msg_buf);
	      goto start_over;
	    }
	    //goto start_over;
	  }
		sleep(1);
		sprintf(msg_buf.mdata, "%d,USB parent", getpid());
		//printf("%s\n", msg_buf.mdata);
		send_message();
	}

        usbsystemdevice--; 

	libusb_free_device_list(devs, 1);
m_deinit:
	libusb_exit(ctx);
m_return:
        usbsystemdevice--;
	return 0;

// -----------------------------------------------------------------------------
// a separate instance for every usb readout device
// -----------------------------------------------------------------------------

mh_init:
	// log files
#if DAEMONIZE
	sprintf(out_log, "%s/usb_%d_out.log", log_dir, ser_no);
	sprintf(err_log, "%s/usb_%d_err.log", log_dir, ser_no);
	open_log_files();
        usbsystemdevice--;
	sprintf(gaibu_msg_buf,"OV DAQ USB Stream %d Started", ser_no);
	gaibu_msg(MNOTICE,gaibu_msg_buf);
#endif
		
	// initialize my variables
	rw = 0;	
	nr = 0;
	wn = 0;
	read_bytes = 0;
	written_bytes = 0;
	diag_in_ready = 1;
	diag_expecting = 0;
	com_out_ready = 1;
	com_out_attempt = 0;
	per_out_ready = 1;
	
	init_timers();
	init_buffers();
	init_files();

	// allocate transfers
	com_out_transfer = libusb_alloc_transfer(0);
	if (!com_out_transfer) { 
		printf("com out: not enough memory\n"); goto mh_free0; 
	}
	
	per_out_transfer = libusb_alloc_transfer(0);
	if (!per_out_transfer) { 
		printf("per out: not enough memory\n"); goto mh_free1;
	}

	diag_in_transfer = libusb_alloc_transfer(0);
	if (!diag_in_transfer) {	
		printf("diag in: not enough memory\n"); goto mh_free2; 
	}
	
	for (i = 0; i < BUF_NO; i++) {
		d[i].usb_transfer = libusb_alloc_transfer(0);
		d[i].disk_transfer = malloc(sizeof(struct aiocb));
		if (!d[i].usb_transfer || !d[i].disk_transfer) {
			printf("alloc: not enough memory\n"); goto mh_free3;
		}
	}

	// main loop
	while(!kill_me) {

		check_data_in(); //will open the file eventually depending on the flag
		check_diag_in();
		check_com_out();
		check_per_out();
		read_data_in();  //will close the file eventually depending on the flag


#if 1 //DAEMONIZE == 0
		debug();
#endif

		// send alive message
		if (timer_zero_reset(T_DIAG, 1)) {
			sprintf(msg_buf.mdata, "%d,USB,%d,%d,%d,%d",
					getpid(), ser_no, read_bytes, written_bytes, 
					diag_result | (out_timeout << 7));
			read_bytes = 0; written_bytes = 0;
			//printf("%s\n", msg_buf.mdata);
			send_message();
		}
		
		// let library do its thing
		r = libusb_handle_events(ctx);
		if (r) {
			printf("handle events error\n");
			kill_me = 1;
			break;
		}
	}

	// clean up
	if (!com_out_ready) {
		r = libusb_cancel_transfer(com_out_transfer);
		if (r < 0) printf("error cancelling com out\n");
	}

	if (!per_out_ready) {
		r = libusb_cancel_transfer(per_out_transfer);
		if (r < 0) printf("error cancelling per out\n");
	}

	if (!diag_in_ready) {
		r = libusb_cancel_transfer(diag_in_transfer);
		if (r < 0) printf("error cancelling com in\n");
	}

	for (i = 0; i < BUF_NO; i++) {
		if (d[i].buf_state == BUF_READING) {
			r = libusb_cancel_transfer(d[i].usb_transfer);
			if (r < 0) printf("error cancelling data in\n");
			else d[i].buf_state = BUF_CANCELLING;
		}
		if (d[i].buf_state == BUF_WRITING) {
			r = aio_cancel(d[i].f, d[i].disk_transfer);	
			printf("aio cancel %d: %d\n", i, r);
			d[i].buf_state = BUF_NOTHING;
		}
	}
		
	// wait for cancellation
	int done;
	do {
		if (libusb_handle_events(ctx) < 0) break;
		done = 1;
		if (!per_out_ready || !com_out_ready || !diag_in_ready) done = 0;
		for (i = 0; i < BUF_NO; i++)
			if (d[i].buf_state == BUF_READING) done = 0;
	} while (!done);

	for (i = 0; i < 2; i++) {
		if (f[i].state == F_WRITING) f[i].state = F_CLOSING;
		try_to_close(i);
	}

mh_free3:
	for (i = 0; i < BUF_NO; i++) {
		if (d[i].usb_transfer) libusb_free_transfer(d[i].usb_transfer);
		if (d[i].disk_transfer) free(d[i].disk_transfer);
	}
	libusb_free_transfer(diag_in_transfer);
mh_free2:
	libusb_free_transfer(per_out_transfer);
mh_free1:
	libusb_free_transfer(com_out_transfer);
mh_free0:
	
	// clear stalls on endpoints & reset device
	r = libusb_clear_halt(devh, EP_DATA_IN);
	if (r < 0) { printf("couldn't clear stall ep_data_in %d\n", r); }

	r = libusb_clear_halt(devh, EP_DIAG_IN);
	if (r < 0) { printf("couldn't clear stall ep_com_in %d\n", r); }

	r = libusb_clear_halt(devh, EP_COM_OUT);
	if (r < 0) { printf("couldn't clear stall ep_com_out %d\n", r); }

	r = libusb_reset_device(devh);
	if (r < 0) { printf("couldn't reset device %d\n", r); }
	 
	libusb_release_interface(devh, 0);
	libusb_close(devh);
	libusb_free_device_list(devs, 1);
	libusb_exit(ctx);

        usbsystemdevice--;

	close(gaibu_sockfd);
	closelog();
	return 0;
}
