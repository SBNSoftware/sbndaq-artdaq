#ifndef sbndaq_artdaq_Generators_ICARUSTriggerV3_hh
#define sbndaq_artdaq_Generators_ICARUSTriggerV3_hh

// The V3 Receiver class recieves V3 data from an OtSDAQ applicance and
// puts that data into V3Fragments for further ARTDAQ analysis.
//
// It currently assumes two things to be true:
// 1. The first word of the V3 packet is an 8-bit flag with information
// about the status of the sender
// 2. The second word is an 8-bit sequence ID, used for detecting
// dropped V3 datagrams
// Some C++ conventions used:
// -Append a "_" to every private member function and variable

#include "fhiclcpp/fwd.h"
#include "artdaq-core/Data/Fragment.hh"
#include "artdaq/Generators/CommandableFragmentGenerator.hh"
#include "sbndaq-artdaq-core/Overlays/ICARUS/ICARUSTriggerV3Fragment.hh"
#include "sbndaq-artdaq-core/Overlays/ICARUS/ICARUSPMTGateFragment.hh"
#include "sbndaq-artdaq-core/Overlays/FragmentType.hh"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <array>
#include <list>
#include <queue>
#include <atomic>
#include <vector>

namespace sbndaq
{

  //command definitions
  typedef char Command_t[16];
  const Command_t TTLK_INIT="TTLK_CMD_INIT";
  const Command_t TRIG_VETO="TRIG_CMD_VETO";
  const Command_t TRIG_ALLW="TRIG_CMD_ALLW";

  class ICARUSTriggerV3 : public artdaq::CommandableFragmentGenerator
  {
  public:
    
    explicit ICARUSTriggerV3(fhicl::ParameterSet const& ps);
    
  private:
    
    bool getNext_(artdaq::FragmentPtrs& output) override;
    void start() override;
    void stop() override;
    void stopNoMutex() override {} // nothing special needs to be done in this method
    void pause() override;
    void resume() override;
    
    void sendUDP(const Command_t);
    int poll_with_timeout(int,std::string,struct sockaddr_in&, int);
    //int read(int,uint16_t*);
    int read(int, std::string, struct sockaddr_in&,int,char*);
    int readTCP(int, std::string, struct sockaddr_in&,int,char*);
    int send_init_params(std::vector<std::string>, fhicl::ParameterSet);
    void configure_socket(int, struct sockaddr_in&);
    
    //int send_TTLK_INIT(int, int);
    int send_stop(int);
    int initialization(int, int);
    void send_TRIG_VETO();
    void send_TRIG_ALLW();
    
    // FHiCL-configurable variables. Note that the C++ variable names
    // are the FHiCL variable names with a "_" appended

    uint32_t fragment_id_;
    uint32_t fragment_id_pmt_;
    size_t max_fragment_size_bytes_;
    size_t max_fragment_size_bytes_pmt_;

    int configport_;
    std::string ip_config_;

    int dataport_;
    std::string ip_data_;

    //Socket parameters
    struct sockaddr_in si_config_;
    int configsocket_;

    struct sockaddr_in si_data_;
    int datasocket_;

    int datafd_;

    int dataconnfd_;

    int pmtdataport_;
    std::string ip_data_pmt_;

    struct sockaddr_in si_pmtdata_;
    int pmtsocket_;

    //retry for init msg
    int n_init_retries_;
    int n_init_timeout_ms_;
    uint64_t fEventCounter;
    uint64_t fLastEvent;
    uint64_t fNTP_time;
    uint64_t fLastTimestamp;
    uint64_t fLastTimestampBNBMaj;
    uint64_t fLastTimestampBNBMinbias;
    uint64_t fLastTimestampNuMIMaj;
    uint64_t fLastTimestampNuMIMinbias;
    uint64_t fLastTimestampOther;
    uint64_t fLastTimestampBNBOffMaj;
    uint64_t fLastTimestampBNBOffMinbias;
    uint64_t fLastTimestampNuMIOffMaj;
    uint64_t fLastTimestampNuMIOffMinbias;
    uint64_t fLastTimestampCalibMaj;
    uint64_t fLastTimestampCalibMinbias;
    long fLastGatesNum;
    long fLastGatesNumBNBMaj;
    long fLastGatesNumBNBMinbias;
    long fLastGatesNumNuMIMaj;
    long fLastGatesNumNuMIMinbias;
    long fLastGatesNumOther;
    long fLastGatesNumBNBOffMaj;
    long fLastGatesNumBNBOffMinbias;
    long fLastGatesNumNuMIOffMaj;
    long fLastGatesNumNuMIOffMinbias;
    long fLastGatesNumCalibMaj;
    long fLastGatesNumCalibMinbias;
    long fDeltaGates;
    long fDeltaGatesBNBMaj;
    long fDeltaGatesBNBMinbias;
    long fDeltaGatesNuMIMaj;
    long fDeltaGatesNuMIMinbias;
    long fDeltaGatesOther;
    long fDeltaGatesBNBOffMaj;
    long fDeltaGatesBNBOffMinbias;
    long fDeltaGatesNuMIOffMaj;
    long fDeltaGatesNuMIOffMinbias;
    long fDeltaGatesCalibMaj;
    long fDeltaGatesCalibMinbias;
    long fLastTriggerBNBMaj;
    long fLastTriggerBNBMinbias;
    long fLastTriggerNuMIMaj;
    long fLastTriggerNuMIMinbias;
    long fLastTriggerBNBOffMaj;
    long fLastTriggerBNBOffMinbias;
    long fLastTriggerNuMIOffMaj;
    long fLastTriggerNuMIOffMinbias;
    long fLastTriggerCalibMaj;
    long fLastTriggerCalibMinbias;
    long fTotalTriggerBNB;
    long fTotalTriggerBNBMaj;
    long fTotalTriggerBNBMinbias;
    long fTotalTriggerNuMI;
    long fTotalTriggerNuMIMaj;
    long fTotalTriggerNuMIMinbias;
    long fTotalTriggerBNBOff;
    long fTotalTriggerBNBOffMaj;
    long fTotalTriggerBNBOffMinbias;
    long fTotalTriggerNuMIOff;
    long fTotalTriggerNuMIOffMaj;
    long fTotalTriggerNuMIOffMinbias;
    long fTotalTriggerCalib;
    long fTotalTriggerCalibMaj;
    long fTotalTriggerCalibMinbias;
    long fTotalGatesBNB;
    long fTotalGatesBNBMaj;
    long fTotalGatesBNBMinbias;
    long fTotalGatesNuMI;
    long fTotalGatesNuMIMaj;
    long fTotalGatesNuMIMinbias;
    long fTotalGatesBNBOff;
    long fTotalGatesBNBOffMaj;
    long fTotalGatesBNBOffMinbias;
    long fTotalGatesNuMIOff;
    long fTotalGatesNuMIOffMaj;
    long fTotalGatesNuMIOffMinbias;
    long fTotalGatesCalibMaj;
    long fTotalGatesCalibMinbias;
    int fLastTriggerType;
    int fLastSourceType;
    uint64_t fStartOfRun;
    int fInitialStep;
    bool use_wr_time_;
    long wr_time_offset_ns_;
    //expected fragments
    int generated_fragments_per_event_;
    char buffer[1000] = {'\0'};
    uint8_t peekBuffer[2] = {0,0};

    fhicl::ParameterSet initialization_data_fpga_;
    fhicl::ParameterSet initialization_data_spexi_;
  };
}
#endif /* sbndaq_artdaq_Generators_ICARUSTriggerV3_hh */
