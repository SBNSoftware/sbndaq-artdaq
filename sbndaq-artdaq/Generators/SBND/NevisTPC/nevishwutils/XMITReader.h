#ifndef _NEVISTPC_XMITREADER_H
#define _NEVISTPC_XMITREADER_H 1

#include "NevisReadoutPCIeCard.h"
#include "fhiclcpp/ParameterSet.h"

#include <iosfwd>
#include <string>

namespace nevistpc{
  
  struct dma_settings {
    explicit dma_settings ( uint32_t dma_buffer_size_, uint32_t dma_max_polling_time_microseconds_)
      : dma_buffer_size {dma_buffer_size_}, dma_max_polling_time_microseconds {dma_max_polling_time_microseconds_} {};
    
    std::string toString();
    
    const uint32_t dma_buffer_size;
    const uint32_t dma_max_polling_time_microseconds;
  };
  
  typedef std::shared_ptr<dma_settings> dma_settingsSPtr;
  
  struct dma_buffer {
    PVOID  pUserModeBuffer;
    WD_DMA* pDMABuffer;
    DWORD  r_cs_reg;
    size_t bufferSize;
    size_t readSize;
  };
  
  class XMITReader{
  public:
    enum class dma_completion_status: uint8_t {
      timeout = 0,
      complete
    };

    explicit XMITReader ( std::string const& streamName, fhicl::ParameterSet const & ps );

    
    void configureReader();

    void initializePCIeCard();

    void setupTXModeRegister();

    virtual std::streamsize readsome ( char* buffer, std::streamsize requestedSize );
    virtual void dmaStop();
    virtual ~XMITReader(){};
  private:
    bool dmaLockBuffer ( dma_buffer& dma );
    void dmaSetTXModeRegister();
    void dmaAbort();
    void dmaClearRegister();
    void dmaInitializeOnFirstLoop();
    void initializeReceivers();
    void startReceiver ( dma_buffer& dma );
    void startReceivers();
    void setupTransceivers ( dma_buffer& dma );
    void dmaStart ( dma_buffer& dma );
    dma_completion_status  dmaWaitWithTimeout ( unsigned int microseconds );
    void dmaReportTransferStatus();
    void writeBufferToBinaryFile ( const char* buffer, const std::streamsize size );

  private:
    const std::string _stream_name; // XMIT stream name
    const fhicl::ParameterSet _params; // FHiCL parameter set
    std::unique_ptr<DeviceInfo> nevisDeviceInfo; // Pointer to the parameters identifying the PCIe card
    NevisReadoutPCIeCardUPtr nevisPCIeCard; // Pointer to the PCIe card reading the XMIT stream

    bool _isFirstEverDMA; // First DMA flag
    bool _do_timing; // Timing profile flag

    dma_settingsSPtr _dma_settings;
    dma_buffer _dma1;
    dma_buffer _dma2;

  };

  typedef std::shared_ptr<XMITReader> XMITReaderSPtr;

}  // end of namespace nevistpc
#endif //_NEVISTPC_XMITREADER_H
