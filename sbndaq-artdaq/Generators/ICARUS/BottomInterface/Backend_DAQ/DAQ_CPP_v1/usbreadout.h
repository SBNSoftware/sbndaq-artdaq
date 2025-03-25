#ifndef usbreadout_h
#define usbreadout_h

#include <stdio.h>

void handle(int sig);
void find_mqs();
void mq_send(int msg, char data[]);
void mq_send_str(int msg, char data[]);
void mq_send_usb(int msg, int usb_num, unsigned char data[]);
void send_out(int usbdata);
void send_out_usb(int usb, unsigned int usbdata);
void set_data_disk(int disk_num);
void set_run_number(char run_number[]);
void set_data_path(char data_path[]);
void com_usb(int usb, int pmt, int b2, int b3);
void set_inhibit_usb(int usb, int usbdata);
void usb_usb(int usb,int adr,int data);
void usb(int adr, int data);
void dac_usb(int usb, int pmt, int data);




#endif /* usbreadout_h */
