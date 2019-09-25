#ifndef sbndaq_artdaq_Generators_ICARUSTriggerUDP_hh
#define sbndaq_artdaq_Generators_ICARUSTriggerUDP_hh

// The UDP Receiver class recieves UDP data from an OtSDAQ applicance and
// puts that data into UDPFragments for further ARTDAQ analysis.
//
// It currently assumes two things to be true:
// 1. The first word of the UDP packet is an 8-bit flag with information
// about the status of the sender
// 2. The second word is an 8-bit sequence ID, used for detecting
// dropped UDP datagrams
// Some C++ conventions used:
// -Append a "_" to every private member function and variable

#include "fhiclcpp/fwd.h"
#include "artdaq-core/Data/Fragment.hh"
#include "artdaq/Generators/CommandableFragmentGenerator.hh"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <array>
#include <list>
#include <queue>
#include <atomic>
namespace sbndaq
{
  /**
   * \brief Enumeration describing valid command types
   */
  enum class CommandType : uint8_t
  {
    Read = 0,
      Write = 1,
      Start_Burst = 2,
      Stop_Burst = 3,
      };
  /**
   * \brief Enumeration describing status codes that indicate current sender position in the stream
   */
  enum class ReturnCode : uint8_t
  {
    Read = 0,
      First = 1,
      Middle = 2,
      Last = 3,
      };
  /**
   * \brief Enumeration describing potential data types
   */
  enum class DataType : uint8_t
  {
    Raw = 0,
      JSON = 1,
      String = 2,
      };
  /**
   * \brief Struct defining UDP packet used for communicating with data receiver
   */
        struct CommandPacket
        {
	  CommandType type; ///< The type of this CommandPacket
	  uint8_t dataSize; ///< How many words of data are in the packet
	  uint64_t address; ///< The destination of the CommandPacket
	  uint64_t data[182]; ///< The data for the CommandPacket
        };
  typedef std::array<uint8_t, 1500> packetBuffer_t; ///< An array of 1500 bytes (MTU length)
  typedef std::list<packetBuffer_t> packetBuffer_list_t; ///< A std::list of packetbuffer_t objects
  /**
   * \brief An artdaq::CommandableFragmentGenerator which receives data in the form of UDP datagrams
   */
  class ICARUSTriggerUDP : public artdaq::CommandableFragmentGenerator
  {
  public:
    /**
     * \brief ICARUSTriggerUDP Constructor
     * \param ps ParameterSet used to configure ICARUSTriggerUDP
     * 
     * \verbatim
     * UDPRecevier accepts the following Parameters:
     * "port" (Default: 6343): The port on which to receive UDP data
     * "ip" (Default: 127.0.0.1): The Address to bind to ("0.0.0.0" listens on all addresses)
     * "send_CAPTAN_commands" (Default: false): Whether to send CommandPackets to start and stop the data flow
     * "raw_output_enabled" (Default: false): Whether to write UDP data to disk as well as to EventBuilders
     * "raw_output_path" (Default: "/tmp"): Directory to save raw output file (ICARUSTriggerUDP-[ip]:[port].bin)
     * \endverbatim
     */
    explicit ICARUSTriggerUDP(fhicl::ParameterSet const& ps);
  private:
    // The "getNext_" function is used to implement user-specific
    // functionality; it's a mandatory override of the pure virtual
    // getNext_ function declared in CommandableFragmentGenerator
    bool getNext_(artdaq::FragmentPtrs& output) override;
    void start() override;
    void stop() override;
    void stopNoMutex() override {} // nothing special needs to be done in this method
    void pause() override;
    void resume() override;
    DataType getDataType(uint8_t byte) { return static_cast<DataType>((byte & 0xF0) >> 4); }
    ReturnCode getReturnCode(uint8_t byte) { return static_cast<ReturnCode>(byte & 0xF); }
    void send(CommandType flag);
    // FHiCL-configurable variables. Note that the C++ variable names
    // are the FHiCL variable names with a "_" appended
    int dataport_;
    std::string ip_;
    //The packet number of the next packet. Used to discover dropped packets
    uint8_t expectedPacketNumber_;
    //Socket parameters
    struct sockaddr_in si_data_;
    int datasocket_;
    bool sendCommands_;
    packetBuffer_list_t packetBuffers_;
    bool rawOutput_;
    std::string rawPath_;

    unsigned int n_sends_;
  };
}
#endif /* sbndaq_artdaq_Generators_ICARUSTriggerUDP_hh */
