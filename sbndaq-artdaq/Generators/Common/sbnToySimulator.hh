#ifndef sbndaq_artdaq_Generators_sbnToySimulator_hh
#define sbndaq_artdaq_Generators_sbnToySimulator_hh

// Some C++ conventions used:

// -Append a "_" to every private member function and variable

#include "sbndaq-artdaq-core/Overlays/Common/sbnToyFragment.hh"
#include "artdaq-core/Data/Fragment.hh"
#include "artdaq/Generators/CommandableFragmentGenerator.hh"
#include "fhiclcpp/fwd.h"


namespace sbndaq {
/**
 * \brief ToySimulator is a simple type of fragment generator intended to be
 * studied by new users of artdaq as an example of how to create such
 * a generator in the "best practices" manner. Derived from artdaq's
 * CommandableFragmentGenerator class, it can be used in a full DAQ
 * simulation, obtaining data from the ToyHardwareInterface class
 *
 */
class sbnToySimulator : public artdaq::CommandableFragmentGenerator
  {
    public:
      /**
       * \brief ToySimulator Constructor
       * \param ps ParameterSet used to configure ToySimulator
       *
       */
      explicit sbnToySimulator(fhicl::ParameterSet const& ps);

      /**
       * \brief Shutdown the sbnToySimulator
       */
      virtual ~sbnToySimulator();

    private:
      /**
       * \brief The "getNext_" function is used to implement user-specific
       * functionality; it's a mandatory override of the pure virtual
       * getNext_ function declared in CommandableFragmentGenerator
       * \param output New FragmentPtrs will be added to this container
       * \return True if data-taking should continue
       */
      bool getNext_(artdaq::FragmentPtrs& output) override;

      // The start, stop and stopNoMutex methods are declared pure
      // virtual in CommandableFragmentGenerator and therefore MUST be
      // overridden; note that stopNoMutex() doesn't do anything here

      /**
       * \brief Perform start actions
       *
       * Override of pure virtual function in CommandableFragmentGenerator.
       */
      void start() override;

      /**
       * \brief Perform stop actions
       *
       * Override of pure virtual function in CommandableFragmentGenerator.
       */
      void stop() override;

      /**
       * \brief Override of pure virtual function in CommandableFragmentGenerator.
       * stopNoMutex does not do anything in ToySimulator
       */
      void stopNoMutex() override {}

      sbnToyFragment::Metadata metadata_;
//      sbnToyFragment::Header header_;

      int fragment_id_;

      std::chrono::time_point<std::chrono::system_clock> timestamp_;
      std::chrono::duration<int, std::ratio<1, 1000>> time_step_;

  };
}  // namespace sbndaq

#endif /* sbndaq_artdaq_Generators_sbnToySimulator_hh */
