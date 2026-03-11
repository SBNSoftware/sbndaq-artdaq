#include "A2795Board.h"
#include "structures.h"
#include "veto.h"

#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "CAENComm.h"
#include "sbndaq-artdaq/Generators/Common/CAENDecoder.hh"
#include "artdaq/DAQdata/Globals.hh"

#define WRITE 1
#define READ 0

#define HOST 1

A2795Board::A2795Board(int nbr, int bus) : boardNbr(nbr), boardId(nbr), busNbr(bus)
{

#ifndef _simulate_

  int ret;
  bdhandle = 0;

  TRACEN("A2795Board.cc", TLVL_DEBUG + 1, "calling CAENComm_OpenDevice for boardNbr %d boardId %d", boardNbr, boardId);

  ret = CAENComm_OpenDevice2(CAENComm_OpticalLink, &bus, nbr, 0, &bdhandle);

  TRACEN("A2795Board.cc", TLVL_DEBUG + 2, "CAENComm_OpenDevice returned status %d", ret);

  if (ret != CAENComm_Success)
  {
    char msgBuffer[256];
    CAENComm_DecodeError(ret, msgBuffer);
    TRACEN("A2795Board.cc", TLVL_ERROR, "A2795Board::A2795Board(): CAENComm_OpenDevice2 failed on link %d board %d: %s",
           busNbr, boardNbr, msgBuffer);
    boardId = -1;
  }
  else
  {
    int status;
    // CAENComm_Write32(bdhandle, A_Signals, SIGNALS_SWRESET);

    ret = CAENComm_Read32(bdhandle, A_StatusReg, (uint32_t *)&status);
    if (ret != CAENComm_Success)
    {
      char msgBuffer[256];
      CAENComm_DecodeError(ret, msgBuffer);
      TRACEN("A2795Board.cc", TLVL_ERROR, "A2795Board::A2795Board(): CAENComm_Read32(A_StatusReg) failed on link %d board %d: %s",
             busNbr, boardNbr, msgBuffer);
    }
    TRACEN("A2795Board.cc", TLVL_DEBUG + 2, "board %d status %d", boardId, status);

    boardId = status & STATUS_SLOT_ID;

    // CAENComm_Write32(bdhandle, A_ControlReg_Clear, CTRL_TTLINK_MODE);
    // CAENComm_Write32(bdhandle, A_ControlReg_Set, CTRL_ACQRUN);
  }
#endif

  serialNbr = boardNbr;
  if (boardId != -1)
  {
    TRACEN("A2795Board.cc", TLVL_DEBUG + 3, "BoardId %d serial %d bus %d slot %d", boardId, serialNbr, busNbr, slotNbr);
  }
}

A2795Board::~A2795Board()
{
  if (bdhandle != -1)
  {
    int ret = CAENComm_Write32(bdhandle, A_Signals, SIGNALS_TTLINK_EOR);
    if (ret != CAENComm_Success)
    {
      char msgBuffer[256];
      CAENComm_DecodeError(ret, msgBuffer);
      TRACEN("A2795Board.cc", TLVL_WARNING, "A2795Board::~A2795Board(): CAENComm_Write32(SIGNALS_TTLINK_EOR) failed on link %d board %d: %s",
             busNbr, boardNbr, msgBuffer);
    }
    ret = CAENComm_CloseDevice(bdhandle);
    if (ret != CAENComm_Success)
    {
      char msgBuffer[256];
      CAENComm_DecodeError(ret, msgBuffer);
      TRACEN("A2795Board.cc", TLVL_WARNING, "A2795Board::~A2795Board(): CAENComm_CloseDevice failed on link %d board %d: %s",
             busNbr, boardNbr, msgBuffer);
    }
  }
}

#ifndef _simulate_

// Configures the board.
// Input (param): BoardConf conf - structure with the configuration parameters.
void A2795Board::configure(BoardConf conf)
{
  // int status;
  TRACEN("A2795Board.cc", TLVL_DEBUG + 4, "Configuring Board");

  CAENComm_Write32(bdhandle, A_RELE, RELE_TP_DIS);

  // Configure timebase
  TRACEN("A2795Board.cc", TLVL_DEBUG + 5, "Conf Board %d offs %f thre %f ", boardNbr, conf.coff[boardNbr * 2], conf.cthre[boardNbr * 2]);
  TRACEN("A2795Board.cc", TLVL_DEBUG + 6, "offs %f thre %f ", conf.coff[(boardNbr * 2) + 1], conf.cthre[(boardNbr * 2) + 1]);
  TRACEN("A2795Board.cc", TLVL_DEBUG + 7, "off %f thre %f ", conf.coff[(boardNbr) * 2], conf.cthre[(boardNbr) * 2]);
  TRACEN("A2795Board.cc", TLVL_DEBUG + 8, "off %f thre %f", conf.coff[(boardNbr) * 2 + 1], conf.cthre[(boardNbr) * 2 + 1]);

  // Set DC offset
  // tobeset     CAENComm_Write32(bdhandle, A_DAC, 0x00070000 | (conf.coff[boardNbr*2] & 0xFFFF) );

  TRACEN("A2795Board.cc", TLVL_DEBUG + 9, "A2795Board::configure(): board %d configured.", boardNbr);
}

// Configures the trigger parameters.
// Input (param): TrigConf conf - structure with the trigger parameters.
void A2795Board::configureTrig(TrigConf conf)
{
  nSamples = conf.nsamples;
  preSamples = conf.presamples;
}

// Starts the DPU on the board.
void A2795Board::startDPU()
{
  TRACEN("A2795Board.cc", TLVL_DEBUG + 10, "A2795Board::startDPU(): core of board (%d,%d) started.", boardNbr, (int)boardId);
  CAENComm_Write32(bdhandle, A_ControlReg_Set, CTRL_ACQRUN);
  // CAENComm_Write32(bdhandle, A_Signals, SIGNALS_TTLINK_SOR);
}

// Tells if data are ready
// Output (return): true if there are ready data, false otherwise.
int A2795Board::isDataRdy()
{

  int timeoutCounter = 250000;    // Timeout waiting for a trigger
  int errTimeoutCounter = 500000; // Timeout for acquisition completion

  TRACEN("A2795Board.cc", TLVL_DEBUG + 11, "A2795Board::isDataRdy(): BoardId %d BoardNbr %d", boardId, boardNbr);

  int status;
  bool done = false;

  while (!done && errTimeoutCounter--)
  {
    int ret = CAENComm_Read32(bdhandle, A_StatusReg, (uint32_t *)&status);

    // Printing out error on comunication
    if (ret != CAENComm_Success)
    {
      char msgBuffer[256];
      CAENComm_DecodeError(ret, msgBuffer);
      TRACEN("A2795Board.cc", TLVL_ERROR, "A2795Board::isDataRdy(): CAENComm_Read32(A_StatusReg) failed on link %d board %d: %s",
             busNbr, boardNbr, msgBuffer);
    }

    TRACEN("A2795Board.cc", TLVL_DEBUG + 12, "A2795Board::isDataRdy(): Status: %x", status);
    done = (status & STATUS_DRDY); // has data bit 4 Status Reg
    if (!timeoutCounter--)         // Trigger timeout occured
    {
      TRACEN("A2795Board.cc", TLVL_DEBUG + 13, "A2795Board::isDataRdy(): Slow trigger...");
      vetoOff();
    }
  }

  if (!done)
  {
    TRACEN("A2795Board.cc", TLVL_WARNING,
           "A2795Board::isDataRdy(): timeout waiting for data-ready on link %d board %d",
           busNbr, boardNbr);
  }
  return done;
}

int A2795Board::Status()
{
  int status;
  int ret = CAENComm_Read32(bdhandle, A_StatusReg, (uint32_t *)&status);
  if (ret != CAENComm_Success)
  {
    char msgBuffer[256];
    CAENComm_DecodeError(ret, msgBuffer);
    TRACEN("A2795Board.cc", TLVL_ERROR,
           "A2795Board::Status(): CAENComm_Read32(A_StatusReg) failed on link %d board %d: %s",
           busNbr, boardNbr, msgBuffer);
  }
  return status;
}

uint32_t A2795Board::Temperatures()
{
  uint32_t status;
  int ret = CAENComm_Read32(bdhandle, A_Temperature, &status);
  if (ret != CAENComm_Success)
  {
    char msgBuffer[256];
    CAENComm_DecodeError(ret, msgBuffer);
    TRACEN("A2795Board.cc", TLVL_ERROR,
           "A2795Board::Temperatures(): CAENComm_Read32(A_Temperature) failed on link %d board %d: %s",
           busNbr, boardNbr, msgBuffer);
  }
  return status;
}

int A2795Board::ArmTrigger()
{

  // #ifdef _dbg_
  TRACEN("A2795Board.cc", TLVL_DEBUG + 14, "A2795Board::ArmTrigger(): wait a second...");
  // #endif

  return 0;
}
int A2795Board::fillHeader(DataTile *buf)
{
  TRACEN("A2795Board.cc", TLVL_DEBUG + 15, "A2795Board::fillHeader(): wait a second...");

  int ret;
  ret = CAENComm_Read32(bdhandle, A_ControlReg, (uint32_t *)&buf->Header.info1);
  if (ret != CAENComm_Success)
  {
    char msgBuffer[256];
    CAENComm_DecodeError(ret, msgBuffer);
    TRACEN("A2795Board.cc", TLVL_ERROR,
           "A2795Board::fillHeader(): CAENComm_Read32(A_ControlReg) failed on link %d board %d: %s",
           busNbr, boardNbr, msgBuffer);
  }
  ret = CAENComm_Read32(bdhandle, A_StatusReg, (uint32_t *)&buf->Header.info2);
  if (ret != CAENComm_Success)
  {
    char msgBuffer[256];
    CAENComm_DecodeError(ret, msgBuffer);
    TRACEN("A2795Board.cc", TLVL_ERROR,
           "A2795Board::fillHeader(): CAENComm_Read32(A_StatusReg) failed on link %d board %d: %s",
           busNbr, boardNbr, msgBuffer);
  }
  ret = CAENComm_Read32(bdhandle, A_NevStored, (uint32_t *)&buf->Header.info3);
  if (ret != CAENComm_Success)
  {
    char msgBuffer[256];
    CAENComm_DecodeError(ret, msgBuffer);
    TRACEN("A2795Board.cc", TLVL_ERROR,
           "A2795Board::fillHeader(): CAENComm_Read32(A_NevStored) failed on link %d board %d: %s",
           busNbr, boardNbr, msgBuffer);
  }

  TRACEN("A2795Board.cc", TLVL_DEBUG + 16, "Filled header...");

  return 0;
}

#define BUFFER_SIZE ((4 * 1024) * 32) + 3

// Writes the data gathered by the board starting from the passed address.
// Output (param): char* buf - pointer to the memory area to write to.
int A2795Board::getData(int channel, char *buf)
{

  (void)channel;

  TRACEN("A2795Board.cc", TLVL_DEBUG + 17, "A2795Board::getData(): wait a second...");

  // ### Readout the data ###
  int ret;
  int nw;

  TRACEN("A2795Board.cc", TLVL_DEBUG + 18, "Before BLT Read call: %d, %d, %d", bdhandle, A_OutputBuffer, BUFFER_SIZE);

  ret = CAENComm_BLTRead(bdhandle, A_OutputBuffer, (uint32_t *)buf, BUFFER_SIZE, &nw);

  if ((ret != CAENComm_Success) && (ret != CAENComm_Terminate))
  {
    char msgBuffer[256];
    CAENComm_DecodeError(ret, msgBuffer);
    TRACEN("A2795Board.cc", TLVL_ERROR,
           "A2795Board::getData(): CAENComm_BLTRead(A_OutputBuffer) failed on link %d board %d, nw=%d: %s",
           busNbr, boardNbr, nw, msgBuffer);
  }

  // TRACEN("A2795Board.cc",20,"After BLT Read call.");

  TRACEN("A2795Board.cc", TLVL_DEBUG + 21, "start decoding nw %d Boardid %d", nw, boardId);

  return nw * 4;
}

// Writes to a register on the FPGA.
// Input (param): int reg - register number
// Input (param): int value - value to write.
// Output (return): int - outcome of the operation.
int A2795Board::write(int reg, int value)
{
  int ret = CAENComm_Write32(bdhandle, reg, value);
  if (ret != CAENComm_Success)
  {
    char msgBuffer[256];
    CAENComm_DecodeError(ret, msgBuffer);
    TRACEN("A2795Board.cc", TLVL_ERROR,
           "A2795Board::write(): CAENComm_Write32(reg=0x%x, val=0x%x) failed on link %d board %d: %s",
           reg, value, busNbr, boardNbr, msgBuffer);
  }
  return ret;
}

// Reads from a register on the FPGA.
// Input (param): int reg - register number
// Output (param): int* buffer - pointer to the memory area to write to.
// Output (return): int - outcome of the operation.
int A2795Board::read(int reg, int *value)
{
  int ret = CAENComm_Read32(bdhandle, reg, (uint32_t *)value);
  if (ret != CAENComm_Success)
  {
    char msgBuffer[256];
    CAENComm_DecodeError(ret, msgBuffer);
    TRACEN("A2795Board.cc", TLVL_ERROR,
           "A2795Board::read(): CAENComm_Read32(reg=0x%x) failed on link %d board %d: %s",
           reg, busNbr, boardNbr, msgBuffer);
  }
  return ret;
}
#endif

// For testing purposes. If _simulate_ is defined, it should work without the hardware.
#ifdef _simulate_
void A2795Board::configure(BoardConf conf)
{

  TRACEN("A2795Board.cc", TLVL_DEBUG + 21, "Conf Board %d offs %f thre %f \n", boardNbr, conf.coff[boardNbr * 2],
         conf.cthre[boardNbr * 2]);

  TRACEN("A2795Board.cc", TLVL_DEBUG + 22, "offs %f thre %f \n", conf.coff[(boardNbr * 2) + 1], conf.cthre[(boardNbr * 2) + 1]);
}

void A2795Board::configureTrig(TrigConf conf)
{

  nSamples = conf.nsamples;
  preSamples = conf.presamples;

  TRACEN("A2795Board.cc", TLVL_DEBUG + 23, "Set nSamples to %d ", conf.nsamples);
  TRACEN("A2795Board.cc", TLVL_DEBUG + 24, "Set preSamples to %d ", conf.presamples);
}

void A2795Board::startDPU() {}

int A2795Board::isDataRdy()
{
  // #ifdef _dbg_
  TRACEN("A2795Board.cc", TLVL_DEBUG + 25, "A2795Board::isDataRdy(): wait a second...");
  sleep(1);
  // #endif
  return true;
}

int A2795Board::ArmTrigger()
{
  return 0;
}
int A2795Board::fillHeader(DataTile *buf)
{
  // buf->Header.timeinfo= htonl((int) rValue[0]);
  buf->Header.chID = htonl(serialNbr);
}

int A2795Board::getData(int boa, char *buf)
{
  // #ifdef _dbg_
  TRACEN("A2795Board.cc", TLVL_DEBUG + 26, "A2795Board::getData(): filling with %d (%d) for %d bytes", id, id, nSamples);
  // #endif
  for (int i = 0; i < nSamples; i++)
    buf[i] = (char)(boardNbr + '0');
  return nSamples;
}

#endif
