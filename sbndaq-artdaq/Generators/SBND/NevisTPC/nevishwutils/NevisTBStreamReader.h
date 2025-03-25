#ifndef _NEVISTPC_NEVISTBSTREAMREADER_H
#define _NEVISTPC_NEVISTBSTREAMREADER_H 1

#include "NevisControllerPCIeCard.h"
//#include "NevisPCIeCard.h"

#include "fhiclcpp/ParameterSet.h"

#include <iosfwd>
#include <string>
//#include "NevisTBStreamReaderSynchronization.h"

namespace nevistpc{

  void doNothing ( void );

  class NevisTBStreamReader{
  public:

    explicit NevisTBStreamReader ( std::string const& streamName, fhicl::ParameterSet const & ps);
    //: _nevistb_reader("nevistb_reader")  {}
    void configureReader();

    virtual std::streamsize readsome ( char* buffer, std::streamsize requestedSize );
    //    void setReaderSynchronizationObject ( NevisTBStreamReaderSynchronizationObjectSPtr& mutex );

    void initializePCIeCard();

    void setupTXModeRegister();
    virtual ~NevisTBStreamReader(){};
  private:
    const std::string _stream_name;
    const fhicl::ParameterSet _params; // FHiCL parameter set
    std::unique_ptr<DeviceInfo> nevisDeviceInfo;
    NevisControllerPCIeCardUPtr nevisPCIeCard;

    std::function<void ( void ) > _triggerCallFunction;
    //NevisTBStreamReaderSynchronizationObjectSPtr _reader_synch_object;
    //    NevisTBStreamReaderSynchronizationObjectSPtr reader_synch_object1;
    bool _do_timing;
    UINT64 _trig_data_ctr;
    uint32_t _trig_ctr;
    uint32_t _trig_frame;
    uint32_t _trig_sample;
    uint16_t _trig_sample_remain_16MHz;
    uint16_t _trig_sample_remain_64MHz;
  };

  typedef std::shared_ptr<NevisTBStreamReader> NevisTBStreamReaderSPtr;
  //NevisTBStreamReaderSPtr _nevistb_reader;



}  // end of namespace nevistpc                                                                                                      
#endif //_NEVISTPC_NEVISTBSTREAMREADER_H                                                                                                      
