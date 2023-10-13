#ifndef SBNDAQ_CIRCULAR_BUFFER_HH
#define SBNDAQ_CIRCULAR_BUFFER_HH

//#define TRACE_NAME "CircularBuffer"
#include "artdaq/DAQdata/Globals.hh"

#include <boost/circular_buffer.hpp>
#include <memory>
#include <atomic>
#include <mutex>

namespace sbndaq{

  template <class T> class CircularBuffer{

  public:
    CircularBuffer(uint32_t capacity): buffer( boost::circular_buffer<T>(capacity) ),
				       mutexptr(new std::mutex),linearizeCount{0}
    { Init(); }
    CircularBuffer()
    { CircularBuffer(0); }

    void Init(){
      buffer.clear();
      mutexptr->unlock();

      metricMan->sendMetric(".CircularBuffer.LinearizeCount",linearizeCount,"count",11,artdaq::MetricMode::LastPoint);
    }

    size_t Insert(size_t,std::unique_ptr<T[]>  const& );
    size_t Insert(size_t, T const* );
    size_t Erase(size_t);
    size_t Linearize();
    T const* LinearizeAndGetData();
    size_t Size();

    boost::circular_buffer<T> const& Buffer() const
    { return buffer; }

  private:
    boost::circular_buffer<T> buffer;
    std::unique_ptr<std::mutex> mutexptr;
    uint64_t linearizeCount;

    enum {
      TERROR    = TLVL_ERROR,
      TWARNING  = TLVL_WARNING,
      TINFO     = TLVL_INFO,
      TDEBUG    = TLVL_DEBUG
    };

  };

  template <class T>
  size_t sbndaq::CircularBuffer<T>::Insert(size_t n_obj, std::unique_ptr<T[]> const& dataptr){
    
    TRACE(TDEBUG+1,"Inserting %lu objects. Currently %lu/%lu in buffer.",
	  n_obj,buffer.size(),buffer.capacity());
    
    //don't fill while we wait for available capacity...
    while( (buffer.capacity()-buffer.size()) < n_obj){ usleep(100); }
    
    //obtain the lock
    std::unique_lock<std::mutex> lock(*(mutexptr));
    TRACE(TDEBUG+2,"Obtained circular buffer lock for insert.");
    
    buffer.insert(buffer.end(),&(dataptr[0]),&(dataptr[n_obj]));
    
    TRACE(TDEBUG+3,"Inserted %lu objects. Currently have %lu/%lu in buffer.",
	  n_obj,buffer.size(),buffer.capacity());  
    
    return buffer.size();
  }
  
  template <class T>
  size_t sbndaq::CircularBuffer<T>::Insert(size_t n_obj, T const* dataptr){

    TRACE(TDEBUG+1,"Inserting %lu objects (pointer version). Currently %lu/%lu in buffer.",
          n_obj,buffer.size(),buffer.capacity());

    //don't fill while we wait for available capacity...
    while( (buffer.capacity()-buffer.size()) < n_obj){ usleep(100); }

    //obtain the lock
    std::unique_lock<std::mutex> lock(*(mutexptr));
    TRACE(TDEBUG+2,"Obtained circular buffer lock for insert.");

    buffer.insert(buffer.end(),dataptr,dataptr+n_obj);

    TRACE(TDEBUG+3,"Inserted %lu objects. Currently have %lu/%lu in buffer.",
          n_obj,buffer.size(),buffer.capacity());

    return buffer.size();
  }

  template <class T>
  size_t sbndaq::CircularBuffer<T>::Erase(size_t n_obj){
    
    TRACE(TDEBUG+4,"Erasing %lu objects. Currently %lu/%lu in buffer.",
	  n_obj,buffer.size(),buffer.capacity());
    
    std::unique_lock<std::mutex> lock(*(mutexptr));
    TRACE(TDEBUG+5,"Obtained circular buffer lock for erase.");
    
    buffer.erase_begin(n_obj);
    TRACE(TDEBUG+6,"Erased %lu objects. Currently have %lu/%lu in buffer.",
	  n_obj,buffer.size(),buffer.capacity());  
    
    return buffer.size();	
  }
  
  template <class T>
  size_t sbndaq::CircularBuffer<T>::Linearize(){
    
	//TRACE(TDEBUG,"Linearize circular buffer called. Size is %lu. Is linear? %s",
	//  buffer.size(),buffer.is_linearized()?"yes":"no");
	TLOG(TDEBUG)<<"Linearize circular buffer called. Size is "<<buffer.size()
				<<". Is linear? "<< std::string(buffer.is_linearized()?"yes":"no");
    
    if(buffer.is_linearized())
      return buffer.size();
    
    std::unique_lock<std::mutex> lock(*(mutexptr));
    TRACE(TDEBUG,"Obtained circular buffer lock for linearize.");
    
    buffer.linearize();
    
    //TRACE(TDEBUG,"Circular buffer linearize complete. Size is %lu. Is linear? %s",
    //  buffer.size(),buffer.is_linearized()?"yes":"no");
    TLOG(TDEBUG+2)<< "Circular buffer linearize complete. Size is "<<buffer.size()
				<<". Is linear? "<< std::string(buffer.is_linearized()?"yes":"no");
   
    if(!buffer.is_linearized() )	
		 throw std::runtime_error("Circular buffer is not linear.");

    metricMan->sendMetric(".CircularBuffer.LinearizeCount",++linearizeCount,"count",11,artdaq::MetricMode::LastPoint);

    return buffer.size();
  }

  template <class T>
  T const* sbndaq::CircularBuffer<T>::LinearizeAndGetData(){

    TRACE(TDEBUG+2,"Linearize circular buffer called. Size is %lu. Is linear? %s",
          buffer.size(),std::string(buffer.is_linearized()?"yes":"no").c_str());

    if(buffer.is_linearized())
      return &buffer.front();

    std::unique_lock<std::mutex> lock(*(mutexptr));
    TRACE(TDEBUG+2,"Obtained circular buffer lock for linearize.");

    T const* data_ptr = buffer.linearize();

    TLOG(TDEBUG+2)<< "Circular buffer linearize complete. Size is "<<buffer.size()
      <<". Is linear? "<< std::string(buffer.is_linearized()?"yes":"no");

    metricMan->sendMetric(".CircularBuffer.LinearizeCount",++linearizeCount,"count",11,artdaq::MetricMode::LastPoint);

    return data_ptr;
  }

  template <class T>
  size_t sbndaq::CircularBuffer<T>::Size() {

    std::unique_lock<std::mutex> lock(*(mutexptr));
    size_t size = buffer.size();
    TRACE(TDEBUG+2, "Obtained the size of the circular buffer: %lu.", size );
    return size;

  }
}

#endif
