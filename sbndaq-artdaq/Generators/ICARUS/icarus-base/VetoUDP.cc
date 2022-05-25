#include "VetoUDP.h"

#include "artdaq/DAQdata/Globals.hh"

VetoUDP::VetoUDP(const char* input_host, int p):
  port(p),
  myudp( Cudp(port) ),
  totp(0)
{
  std::strcpy(mhost,input_host);
}

int VetoUDP::VetoOn()
{
  strcpy(buf,"ON");
  int ret = myudp.SendTo(mhost,port,buf,2);
  totp++;
  return ret;
}

int VetoUDP::VetoOff()
{
  strcpy(buf,"OF");
  int ret = myudp.SendTo(mhost,port,buf,2);
  totp++;
  return ret;
}

int VetoUDP::TestRun(unsigned int sleep_us)
{
  VetoOn();
  TRACEN("VetoUDP.cc",TLVL_DEBUG+1, "Sent VETO ON packet");
  usleep(sleep_us);

  VetoOff();
  TRACEN("VetoUDP.cc",TLVL_DEBUG+1, "Sent VETO OFF packet");

  usleep(sleep_us);

  return totp;
}
