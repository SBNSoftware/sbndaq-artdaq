/// sends literal command without parameters to feb
#include <zmq.h>
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#define MAXPACKLEN 1500


void usage()
{
 printf("Usage: ");
 printf("confsetgain input_SCRfile.txt gain output_SCRfile.txt\n");
 printf("SCRfile is a bitstream .txt file for SC register.\n");

}

uint8_t bufSCR[MAXPACKLEN];

int ConfigGetBit(uint8_t *buffer, uint16_t bitlen, uint16_t bit_index)
{
  uint8_t byte;
  uint8_t mask;
  byte=buffer[(bitlen-1-bit_index)/8];
  mask= 1 << (7-bit_index%8);
  byte=byte & mask;
  if(byte!=0) return 1; else return 0; 
}


uint8_t ConfigGetGain(int chan)
{
 uint8_t val=0;
 for(int b=0;b<6;b++)
 {
   val=val << 1;
   if(ConfigGetBit(bufSCR,1144,619+chan*15+b)>0) val=val+1;
 } 
 return val;
}

uint8_t ConfigGetBias(int chan)
{
 uint8_t val=0;
 for(int b=0;b<8;b++)
 {
   val=val << 1;
   if(ConfigGetBit(bufSCR,1144,331+chan*9+b)>0) val=val+1;
 } 
 return val;
}

void ConfigSetBit(uint8_t *buffer, uint16_t bitlen, uint16_t bit_index, int value)
{
  uint8_t byte;
  uint8_t mask;
  byte=buffer[(bitlen-1-bit_index)/8];
  mask= 1 << (7-bit_index%8);
  byte=byte & (~mask);
  if(value>0) byte=byte | mask;
  buffer[(bitlen-1-bit_index)/8]=byte;
}


void ConfigSetGain(int chan, uint8_t val)
{
  uint8_t mask=1<<5;
 for(int b=0;b<6;b++)
 {
   if((val & mask)>0) ConfigSetBit(bufSCR,1144,619+chan*15+b,1); else ConfigSetBit(bufSCR,1144,619+chan*15+b,0);
   mask=mask>>1;
 } 
  
}

void ConfigSetBias(int chan, uint8_t val)
{
  uint8_t mask=1<<7;
 for(int b=0;b<8;b++)
 {
   if((val & mask)>0) ConfigSetBit(bufSCR,1144,331+chan*9+b,1); else ConfigSetBit(bufSCR,1144,331+chan*9+b,0);
   mask=mask>>1;
 } 

}


int WriteBitStreamAnnotated(const char * fname, uint8_t *buf, int bitlen) // write CITIROC SC bitstream to file, buf[MAXPACKLEN]
{ 
  FILE *file = fopen(fname, "w");
  uint8_t byte;
  int ib=0;
  char ascii[MAXPACKLEN];
  for(int i=bitlen/8-1;i>=0;i--)
  {
   byte=buf[i];
   for(int j=0;j<8;j++) {if(byte & 0x80) ascii[ib]='1'; else ascii[ib]='0'; byte = byte<<1; ib++;}
  }
  //for(int i=0;i<bitlen;i++) fprintf(file,"%c",ascii[i]);
  ib=0;
  for(int Ch=0; Ch<32;Ch++) {
	fprintf(file,"%c%c%c%c \' Ch%d 4-bit DAC_t ([0..3])\'\n",ascii[ib],ascii[ib+1],ascii[ib+2],ascii[ib+3],Ch);
        ib+=4;
  }
  for(int Ch=0; Ch<32;Ch++) {
	fprintf(file,"%c%c%c%c \' Ch%d 4-bit DAC ([0..3])\'\n",ascii[ib],ascii[ib+1],ascii[ib+2],ascii[ib+3],Ch);
        ib+=4;
  }
  fprintf(file,"%c \'Enable discriminator\'\n",ascii[ib++]);
  fprintf(file,"%c \' Disable trigger discriminator power pulsing mode (force ON)\'\n",ascii[ib++]);
  fprintf(file,"%c \' Select latched (RS : 1) or direct output (trigger : 0)\'\n",ascii[ib++]);
  fprintf(file,"%c \' Enable Discriminator Two\'\n",ascii[ib++]);
  fprintf(file,"%c \' Disable trigger discriminator power pulsing mode (force ON)\'\n",ascii[ib++]);
  fprintf(file,"%c \' EN_4b_dac\'\n",ascii[ib++]);
  fprintf(file,"%c \'PP: 4b_dac\'\n",ascii[ib++]);
  fprintf(file,"%c \' EN_4b_dac_t\'\n",ascii[ib++]);
  fprintf(file,"%c \'PP: 4b_dac_t\'\n",ascii[ib++]);
  for(int Ch=0; Ch<32;Ch++) fprintf(file,"%c",ascii[ib++]);
     fprintf(file," \' Allows to Mask Discriminator (channel 0 to 31) [active low]\'\n");
fprintf(file,"%c \' Disable High Gain Track & Hold power pulsing mode (force ON)\'\n",ascii[ib++]);
fprintf(file,"%c \' Enable High Gain Track & Hold\'\n",ascii[ib++]);
fprintf(file,"%c \' Disable Low Gain Track & Hold power pulsing mode (force ON)\'\n",ascii[ib++]);
fprintf(file,"%c \' Enable Low Gain Track & Hold\'\n",ascii[ib++]);
fprintf(file,"%c \' SCA bias ( 1 = weak bias, 0 = high bias 5MHz ReadOut Speed)\'\n",ascii[ib++]);
fprintf(file,"%c \'PP: HG Pdet\'\n",ascii[ib++]);
fprintf(file,"%c \' EN_HG_Pdet\'\n",ascii[ib++]);
fprintf(file,"%c \'PP: LG Pdet\'\n",ascii[ib++]);
fprintf(file,"%c \' EN_LG_Pdet\'\n",ascii[ib++]);
fprintf(file,"%c \' Sel SCA or PeakD HG\'\n",ascii[ib++]);
fprintf(file,"%c \' Sel SCA or PeakD LG\'\n",ascii[ib++]);
fprintf(file,"%c \' Bypass Peak Sensing Cell\'\n",ascii[ib++]);
fprintf(file,"%c \' Sel Trig Ext PSC\'\n",ascii[ib++]);
fprintf(file,"%c \' Disable fast shaper follower power pulsing mode (force ON)\'\n",ascii[ib++]);
fprintf(file,"%c \' Enable fast shaper\'\n",ascii[ib++]);
fprintf(file,"%c \' Disable fast shaper power pulsing mode (force ON)\'\n",ascii[ib++]);
fprintf(file,"%c \' Disable low gain slow shaper power pulsing mode (force ON)\'\n",ascii[ib++]);
fprintf(file,"%c \' Enable Low Gain Slow Shaper\'\n",ascii[ib++]);

fprintf(file,"%c%c%c \' Low gain shaper time constant commands (0…2)  [active low] 100\'\n",ascii[ib],ascii[ib+1],ascii[ib+2]);
        ib+=3;

fprintf(file,"%c \' Disable high gain slow shaper power pulsing mode (force ON)\'\n",ascii[ib++]);
fprintf(file,"%c \' Enable high gain Slow Shaper\'\n",ascii[ib++]);

fprintf(file,"%c%c%c \' High gain shaper time constant commands (0…2)  [active low] 100\'\n",ascii[ib],ascii[ib+1],ascii[ib+2]);
        ib+=3;

fprintf(file,"%c \' Low Gain PreAmp bias ( 1 = weak bias, 0 = normal bias)\'\n",ascii[ib++]);
fprintf(file,"%c \' Disable High Gain preamp power pulsing mode (force ON)\'\n",ascii[ib++]);
fprintf(file,"%c \' Enable High Gain preamp\'\n",ascii[ib++]);
fprintf(file,"%c \' Disable Low Gain preamp power pulsing mode (force ON)\'\n",ascii[ib++]);
fprintf(file,"%c \' Enable Low Gain preamp\'\n",ascii[ib++]);
fprintf(file,"%c \' Select LG PA to send to Fast Shaper\'\n",ascii[ib++]);
fprintf(file,"%c \' Enable 32 input 8-bit DACs\'\n",ascii[ib++]);
fprintf(file,"%c \' 8-bit input DAC Voltage Reference (1 = external 4,5V , 0 = internal 2,5V)\'\n",ascii[ib++]);

  for(int Ch=0; Ch<32;Ch++) {
	fprintf(file,"%c%c%c%c",ascii[ib],ascii[ib+1],ascii[ib+2],ascii[ib+3]); ib+=4;
	fprintf(file,"%c%c%c%c",ascii[ib],ascii[ib+1],ascii[ib+2],ascii[ib+3]); ib+=4;
	fprintf(file," %c \' Input 8-bit DAC Data channel %d – (DAC7…DAC0 + DAC ON), higher-higher bias\'\n",ascii[ib++],Ch);
  }

  for(int Ch=0; Ch<32;Ch++) {
	fprintf(file,"%c%c%c",ascii[ib],ascii[ib+1],ascii[ib+2]); ib+=3;
	fprintf(file,"%c%c%c ",ascii[ib],ascii[ib+1],ascii[ib+2]); ib+=3;
	fprintf(file,"%c%c%c",ascii[ib],ascii[ib+1],ascii[ib+2]); ib+=3;
	fprintf(file,"%c%c%c ",ascii[ib],ascii[ib+1],ascii[ib+2]); ib+=3;
	fprintf(file,"%c%c%c ",ascii[ib],ascii[ib+1],ascii[ib+2]); ib+=3;
	fprintf(file,"\' Ch%d   PreAmp config (HG gain[5..0], LG gain [5..0], CtestHG, CtestLG, PA disabled)\'\n",Ch);
  }


fprintf(file,"%c \' Disable Temperature Sensor power pulsing mode (force ON)\'\n",ascii[ib++]);
fprintf(file,"%c \' Enable Temperature Sensor\'\n",ascii[ib++]);
fprintf(file,"%c \' Disable BandGap power pulsing mode (force ON)\'\n",ascii[ib++]);
fprintf(file,"%c \' Enable BandGap\'\n",ascii[ib++]);
fprintf(file,"%c \' Enable DAC1\'\n",ascii[ib++]);
fprintf(file,"%c \' Disable DAC1 power pulsing mode (force ON)\'\n",ascii[ib++]);
fprintf(file,"%c \' Enable DAC2\'\n",ascii[ib++]);
fprintf(file,"%c \' Disable DAC2 power pulsing mode (force ON)\'\n",ascii[ib++]);

	fprintf(file,"%c%c ",ascii[ib],ascii[ib+1]); ib+=2;
	fprintf(file,"%c%c%c%c ",ascii[ib],ascii[ib+1],ascii[ib+2],ascii[ib+3]); ib+=4;
	fprintf(file,"%c%c%c%c \' 10-bit DAC1 (MSB-LSB): 00 1100 0000 for 0.5 p.e.\'\n",ascii[ib],ascii[ib+1],ascii[ib+2],ascii[ib+3]); ib+=4;

	fprintf(file,"%c%c ",ascii[ib],ascii[ib+1]); ib+=2;
	fprintf(file,"%c%c%c%c ",ascii[ib],ascii[ib+1],ascii[ib+2],ascii[ib+3]); ib+=4;
	fprintf(file,"%c%c%c%c \' 10-bit DAC2 (MSB-LSB): 00 1100 0000 for 0.5 p.e.\'\n",ascii[ib],ascii[ib+1],ascii[ib+2],ascii[ib+3]); ib+=4;


fprintf(file,"%c \' Enable High Gain OTA'  -- start byte 2\n",ascii[ib++]);
fprintf(file,"%c \' Disable High Gain OTA power pulsing mode (force ON)\'\n",ascii[ib++]);
fprintf(file,"%c \' Enable Low Gain OTA\'\n",ascii[ib++]);
fprintf(file,"%c \' Disable Low Gain OTA power pulsing mode (force ON)\'\n",ascii[ib++]);
fprintf(file,"%c \' Enable Probe OTA\'\n",ascii[ib++]);
fprintf(file,"%c \' Disable Probe OTA power pulsing mode (force ON)\'\n",ascii[ib++]);
fprintf(file,"%c \' Otaq test bit\'\n",ascii[ib++]);
fprintf(file,"%c \' Enable Val_Evt receiver\'\n",ascii[ib++]);
fprintf(file,"%c \' Disable Val_Evt receiver power pulsing mode (force ON)\'\n",ascii[ib++]);
fprintf(file,"%c \' Enable Raz_Chn receiver\'\n",ascii[ib++]);
fprintf(file,"%c \' Disable Raz Chn receiver power pulsing mode (force ON)\'\n",ascii[ib++]);
fprintf(file,"%c \' Enable digital multiplexed output (hit mux out)\'\n",ascii[ib++]);
fprintf(file,"%c \' Enable digital OR32 output\'\n",ascii[ib++]);
fprintf(file,"%c \' Enable digital OR32 Open Collector output\'\n",ascii[ib++]);
fprintf(file,"%c \' Trigger Polarity\'\n",ascii[ib++]);
fprintf(file,"%c \' Enable digital OR32_T Open Collector output\'\n",ascii[ib++]);
fprintf(file,"%c \' Enable 32 channels triggers outputs\'\n",ascii[ib++]);
 // for(int i=ib;i<bitlen;i++) fprintf(file,"%c",ascii[i]);
//  fprintf(file,"\n"); 
  fclose(file);
  return bitlen;
}


int readbitstream(char * fname, uint8_t *buf)
{
  FILE *file = fopen(fname, "r");
       if(file<=0) return 0;
       char line[128];
       char bits[128];
       char comment[128];
       char bit; 
       int ptr, byteptr;
       int bitlen=0;
       char ascii[MAXPACKLEN];
	while (fgets(line, sizeof(line), file)) {
	  bit=1; ptr=0; byteptr=0;
//	  	printf("%d: %s",bitlen,line);

	  while(bit!=0x27 && bit!=0 && ptr<sizeof(line) && bitlen<MAXPACKLEN) // ASCII(0x27)= '
	  {
	    bit=line[ptr];
	    ptr++;
	    if(bit==0x20 || bit==0x27) continue; //ignore spaces and apostrophe
//	     printf("%c",bit);
	    ascii[bitlen]=bit;
	    bitlen++;
	  }
//	printf("\n");
	}
  fclose(file);
  memset(buf,0,MAXPACKLEN); //reset buffer
// Now encode ASCII bitstream into binary
  for(ptr=bitlen-1;ptr>=0;ptr--)
  {
    byteptr=(bitlen-ptr-1)/8;
    if(ascii[ptr]=='1')  buf[byteptr] |= (1 << (7-ptr%8)); 
 //   if((ptr%8)==0) printf("bitpos=%d buf[%d]=%02x\n",ptr,byteptr,buf[byteptr]);
  }

  for (ptr=0;ptr<bitlen/8;++ptr){
    printf("%02x", buf[ptr]);
  }
  puts("\n");

  return bitlen;
}

int main (int argc, char **argv)
{
if(argc!=4) { usage(); return 0;}
int bitlen=0;
char cmd[32];
uint8_t buf[MAXPACKLEN];
int gain=atoi(argv[2]);
bitlen=readbitstream(argv[1], bufSCR);
if(bitlen==1144) printf("ReadBitStream: %d bits read from SCR config file %s.\n",bitlen,argv[1]);
else { printf("ReadBitStream: %d bits read from unrecognized type file %s. Aborting\n",bitlen,argv[1]);
return 0; }
printf("Setting gain for all 32 channels to %d.\n",gain);
for(int i=0;i<32;i++) { printf("ch %d was %d, set to %d\n", i,ConfigGetGain(i) , gain); ConfigSetGain(i, gain);}
WriteBitStreamAnnotated(argv[3], bufSCR, bitlen);

return 1;
}
